/* profileservlet -- the public, read-only account profile behind
 * dreamland.rocks/player/<id>. Trello dIW1M9t8.
 *
 *   /profile {args:{id}} or {args:{char}} -> account title + its visible characters
 *
 * No token: this is the page anyone can open, same footing as the public /who.
 * Linking characters into one account is information `who`/`whois` never give
 * out, so the profile only exists when all three hold:
 *   - at least one character on the account has been reborn (remorts > 0),
 *     hidden characters included, so hiding the reborn one cannot leave
 *     fresh alts on show;
 *   - the owner has not switched it off (account config "profile": false);
 *   - at least one character is not hidden (pfile json attribute
 *     "profile": {"hidden": true}, kept on the char so a rename keeps it hidden).
 * Anything else answers 404, indistinguishable from a missing account.
 *
 * Each character carries what an anonymous `whois` shows: race, class, remorts,
 * Legendary/Immortal rank, a non-hidden clan, titles and online state. Level
 * stays out (who conceals it, see can_see_level), and so does anything from
 * the account record beyond its title.
 */
#include <algorithm>
#include <jsoncpp/json/json.h>

#include "servlet.h"
#include "servlet_utils.h"
#include "accountmanager.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "remortdata.h"
#include "grammar_entities_impl.h"
#include "merc.h"
#include "pcrace.h"
#include "profession.h"
#include "player_utils.h"
#include "clanreference.h"
#include "commonattributes.h"
#include "logstream.h"
#include "def.h"

using namespace std;

CLAN(none);

// Defined in who.cpp: online players as the public who list shows them
// (wizinvis gods and nowho characters left out).
list<PCharacter *> who_find_online(PCharacter *looker);

bool account_profile_char_hidden(PCMemoryInterface *pci)
{
    Json::Value p;
    if (!get_json_attribute(pci, "profile", p) || !p.isObject())
        return false;

    const Json::Value &h = p["hidden"];
    return h.isBool() && h.asBool();
}

void account_profile_set_char_hidden(PCMemoryInterface *pci, bool hidden)
{
    if (hidden) {
        Json::Value p;
        p["hidden"] = true;
        set_json_attribute(pci, "profile", p);
    } else {
        pci->getAttributes().eraseAttribute("profile");
    }

    PCharacterManager::saveMemory(pci);
}

// The owner's switch. On unless explicitly set to false.
bool account_profile_enabled(const DLString &id)
{
    Json::Value cfg = AccountManager::getConfig(id);
    if (!cfg.isObject() || !cfg.isMember("profile"))
        return true;

    const Json::Value &v = cfg["profile"];
    return !v.isBool() || v.asBool();
}

// At least one character on the account has been reborn.
bool account_profile_eligible(const DLString &id)
{
    for (const DLString &name : AccountManager::charsOf(id)) {
        PCMemoryInterface *pci = PCharacterManager::find(name);
        if (pci != 0 && pci->getRemorts().size() > 0)
            return true;
    }
    return false;
}

static Json::Value profile_char_entry(PCMemoryInterface *pci, bool online)
{
    Json::Value c;

    // The per-language display names the game itself shows: the English form or
    // the login, the Russian form, the Ukrainian form or the Russian fallback.
    c["name"]["login"] = pci->getName().c_str();
    c["name"]["en"] = pci->getNameP('1', LANG_EN).c_str();
    c["name"]["ru"] = pci->getNameP('1', LANG_RU).c_str();
    c["name"]["ua"] = pci->getNameP('1', LANG_UA).c_str();
    c["sex"] = pci->getSex() == SEX_FEMALE ? "f" : "m";

    const Race &race = *pci->getRace();
    bool female = pci->getSex() == SEX_FEMALE;
    DLString raceRu = female ? race.getFemaleName() : race.getMaleName();
    DLString raceUa = female ? race.getFemaleNameUa() : race.getMaleNameUa();
    if (raceUa.empty())
        raceUa = raceRu;
    c["race"]["en"] = race.getName().c_str();
    c["race"]["ru"] = raceRu.ruscase('1').c_str();
    c["race"]["ua"] = raceUa.ruscase('1').c_str();

    // Same resolution as the roster entry in accountservlet.cpp: the RU/UA names
    // are Flexer pads, ruscase('1') takes the nominative.
    auto prof = pci->getProfession();
    DLString profRu = prof->getRusName().ruscase('1');
    DLString profUa = prof->getUaName().empty() ? profRu : prof->getUaName().ruscase('1');
    c["class"]["key"] = prof->getName().c_str();
    c["class"]["en"] = prof->getName().c_str();
    c["class"]["ru"] = profRu.c_str();
    c["class"]["ua"] = profUa.c_str();

    c["remorts"] = (int)pci->getRemorts().size();

    // Mirrors whois printName: level 101 with mortal trust is a Legend,
    // anything above is a god.
    if (pci->getLevel() > 100 || pci->getTrust() > 100) {
        if (pci->getLevel() == 101 && pci->getTrust() <= 101)
            c["rank"] = "legendary";
        else
            c["rank"] = "immortal";
    }

    if (!pci->getPretitle().empty())
        c["pretitle"]["en"] = pci->getPretitle().colourStrip().c_str();
    if (!pci->getRussianPretitle().empty())
        c["pretitle"]["ru"] = pci->getRussianPretitle().colourStrip().c_str();

    c["title"]["en"] = Player::title(pci, LANG_EN).colourStrip().c_str();
    c["title"]["ru"] = Player::title(pci, LANG_RU).colourStrip().c_str();
    c["title"]["ua"] = Player::title(pci, LANG_UA).colourStrip().c_str();

    const Clan &clan = *pci->getClan();
    if (pci->getClan() != clan_none && pci->getClan()->isValid() && !clan.isHidden()) {
        c["clan"]["en"] = clan.getNameFor(LANG_EN).colourStrip().c_str();
        c["clan"]["ru"] = clan.getRussianName().ruscase('1').colourStrip().c_str();
        c["clan"]["ua"] = clan.getNameFor(LANG_UA).ruscase('1').colourStrip().c_str();
        c["clan"]["title"]["en"] = clan.getTitle(pci, LANG_EN).colourStrip().c_str();
        c["clan"]["title"]["ru"] = clan.getTitle(pci, LANG_RU).colourStrip().c_str();
        c["clan"]["title"]["ua"] = clan.getTitle(pci, LANG_UA).colourStrip().c_str();
        DLString clr = clan.getColor();
        if (!clr.empty())
            c["clan"]["color"] = clr.c_str();
    }

    c["online"] = online;
    return c;
}

static bool profile_valid_id(const DLString &id)
{
    if (id.empty() || id.size() > 16)
        return false;

    for (char ch : id)
        if (!isalnum((unsigned char)ch))
            return false;
    return true;
}

static void profile_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    DLString id, charName;
    servlet_get_arg(params, "id", id);
    servlet_get_arg(params, "char", charName);

    // By character name: the character must itself be visible, so a hidden
    // alt's name never leads to the account.
    if (id.empty() && !charName.empty()) {
        PCMemoryInterface *pci = PCharacterManager::find(charName.capitalize());
        if (pci == 0 || account_profile_char_hidden(pci)) {
            servlet_response_404(response, "Not found");
            return;
        }
        id = AccountManager::accountOf(pci->getName());
    }

    id.toUpper();
    if (!profile_valid_id(id) || !AccountManager::exists(id)
        || !account_profile_enabled(id) || !account_profile_eligible(id)) {
        servlet_response_404(response, "Not found");
        return;
    }

    list<PCharacter *> online = who_find_online(0);

    list<PCMemoryInterface *> shown;
    for (const DLString &name : AccountManager::charsOf(id)) {
        PCMemoryInterface *pci = PCharacterManager::find(name);
        if (pci != 0 && !account_profile_char_hidden(pci))
            shown.push_back(pci);
    }

    if (shown.empty()) {
        servlet_response_404(response, "Not found");
        return;
    }

    // Most reborn first, then by name.
    shown.sort([](PCMemoryInterface *a, PCMemoryInterface *b) {
        int ra = a->getRemorts().size(), rb = b->getRemorts().size();
        if (ra != rb)
            return ra > rb;
        return a->getName() < b->getName();
    });

    Json::Value body;
    body["id"] = id.c_str();
    body["title"] = AccountManager::titleOf(id).c_str();
    body["chars"] = Json::Value(Json::arrayValue);

    for (PCMemoryInterface *pci : shown) {
        bool isOnline = false;
        for (PCharacter *pch : online)
            if (pch->getName() == pci->getName()) {
                isOnline = true;
                break;
            }
        body["chars"].append(profile_char_entry(pci, isOnline));
    }

    servlet_response_200_json(response, body);
}

SERVLET_HANDLE(api_profile, "/profile")
{
    profile_servlet(request, response);
}
