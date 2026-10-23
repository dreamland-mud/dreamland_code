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
 * stays out (who conceals it, see can_see_level). The account block sums the
 * SHOWN characters only (playing since, hours across every life, lives lived,
 * the class with the most hours), so a hidden alt never moves a number.
 *
 * Achievements come from Fenia (.tmp.achievements.profile in the private
 * world scripts): definitions, the test gate and the secrets never pass
 * through here. No script, a throw or a closed gate leave the field out.
 */
#include <algorithm>
#include <map>
#include <vector>
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
#include "fenia/register-impl.h"
#include "fenia/object.h"
#include "feniamanager.h"
#include "wrapperbase.h"
#include "reglist.h"
#include "json_utils_ext.h"
#include "dreamland.h"
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

// Same resolution as the roster entry in accountservlet.cpp: the RU/UA names
// are Flexer pads, ruscase('1') takes the nominative.
static Json::Value profile_class_json(Profession &prof)
{
    Json::Value c;
    DLString profRu = prof.getRusName().ruscase('1');
    DLString profUa = prof.getUaName().empty() ? profRu : prof.getUaName().ruscase('1');
    c["key"] = prof.getName().c_str();
    c["en"] = prof.getName().c_str();
    c["ru"] = profRu.c_str();
    c["ua"] = profUa.c_str();
    return c;
}

// The most hours one life can honestly hold: the character's age, but never
// less than PROFILE_LIFE_MAX (old characters carry an estimated creation date). A longer life is a corrupt record
// (one pfile carries a 379637-hour life) and counts as 0. Fenia's
// .tmp.achievements.charHours applies the same rule.
static const long PROFILE_LIFE_MAX = 50000;

static long profile_life_cap(PCMemoryInterface *pci)
{
    long created = pci->getCreated();
    long cap = PROFILE_LIFE_MAX;
    if (created > 0 && (dreamland->getCurrentTime() - created) / 3600 > cap)
        cap = (dreamland->getCurrentTime() - created) / 3600;
    return cap;
}

static long profile_life_hours(long hours, long cap)
{
    return (hours > 0 && hours <= cap) ? hours : 0;
}

// True hours of every life of a character: the current one plus the remorts.
static int profile_char_hours(PCMemoryInterface *pci)
{
    long cap = profile_life_cap(pci);
    long hours = profile_life_hours(pci->getPlayedHours(), cap);
    for (auto &life : pci->getRemorts())
        hours += profile_life_hours(life.time.getValue(), cap);
    return (int)hours;
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

    c["class"] = profile_class_json(*pci->getProfession());

    c["remorts"] = (int)pci->getRemorts().size();

    // Mirrors whois printName: level 101 with mortal trust is a Legend,
    // anything above is a god.
    // get_trust is the effective one: a coder is a god at any level.
    if (pci->getLevel() > 100 || pci->get_trust() > 100) {
        if (pci->getLevel() == 101 && pci->get_trust() <= 101)
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
        // The internal name keys the clan badge (the who widget's letter).
        c["clan"]["key"] = clan.getName().c_str();
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

    // The character's own description, as look shows it. Empty is left out.
    const DLString &dEn = pci->getDescription(LANG_EN);
    const DLString &dRu = pci->getDescription(LANG_RU);
    const DLString &dUa = pci->getDescription(LANG_UA);
    if (!dEn.empty())
        c["desc"]["en"] = dEn.colourStrip().c_str();
    if (!dRu.empty())
        c["desc"]["ru"] = dRu.colourStrip().c_str();
    if (!dUa.empty())
        c["desc"]["ua"] = dUa.colourStrip().c_str();

    c["online"] = online;
    return c;
}

// Account-wide numbers over the shown characters: the earliest creation date
// (created survives remort; 0 = a character older than the field, and then the
// true start is unknown, so no date at all beats a too-recent one), true hours
// of the current life plus every past one, and the class played the longest.
static Json::Value profile_account_stats(const list<PCMemoryInterface *> &shown)
{
    Json::Value a;
    long since = 0;
    bool sinceUnknown = false;
    long hours = 0;
    int lives = 0;
    map<DLString, long> byClass;

    for (PCMemoryInterface *pci : shown) {
        long created = pci->getCreated();
        if (created <= 0)
            sinceUnknown = true;
        else if (since == 0 || created < since)
            since = created;

        long cap = profile_life_cap(pci);
        long now = profile_life_hours(pci->getPlayedHours(), cap);
        hours += now;
        byClass[pci->getProfession()->getName()] += now;
        lives++;

        Remorts &r = pci->getRemorts();
        for (unsigned int i = 0; i < r.size(); i++) {
            long t = profile_life_hours(r[i].time.getValue(), cap);
            hours += t;
            byClass[r[i].classCh.getValue()] += t;
            lives++;
        }
    }

    if (since > 0 && !sinceUnknown)
        a["since"] = (Json::Int64)since;
    a["hours"] = (Json::Int64)hours;
    a["lives"] = lives;

    // Most hours first; a class name from an old life may no longer resolve,
    // then the next one takes its place.
    vector<pair<long, DLString> > ranked;
    for (auto &kv : byClass)
        if (kv.second > 0)
            ranked.push_back(make_pair(kv.second, kv.first));
    sort(ranked.begin(), ranked.end(), [](const pair<long, DLString> &x, const pair<long, DLString> &y) {
        return x.first > y.first;
    });

    for (auto &r : ranked) {
        Profession *prof = professionManager->findExisting(r.second);
        if (prof != 0) {
            a["favClass"] = profile_class_json(*prof);
            break;
        }
    }

    return a;
}

// .tmp.achievements.profile(names) over the shown characters. True and the
// list in out when the script answered with one.
static bool profile_achievements(const list<PCMemoryInterface *> &shown, Json::Value &out)
{
    using namespace Scripting;

    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP("tmp"), ID_ACHIEVEMENTS("achievements"), ID_PROFILE("profile");

    try {
        Register tmp = *Context::root[ID_TMP];
        Register ach = *tmp[ID_ACHIEVEMENTS];
        if (ach.type != Register::OBJECT)
            return false;

        Register function = *ach[ID_PROFILE];

        if (function.type != Register::FUNCTION)
            return false;

        RegList::Pointer names(NEW);
        for (PCMemoryInterface *pci : shown)
            names->push_back(Register(pci->getName()));

        Scripting::Object *listObj = &Scripting::Object::manager->allocate();
        listObj->setHandler(names);

        RegisterList args;
        args.push_back(Register(listObj));

        Register result = function.toFunction()->invoke(ach, args);
        if (result.type == Register::NONE)
            return false;

        out = JsonUtils::fromRegister(result);
        return out.isArray();

    } catch (const ::Exception &e) {
        FeniaManager::getThis()->croak(0, Register(DLString("achievements.profile")), e);
        return false;
    }
}

// The achievements block costs tens of milliseconds of Fenia per shown
// character on the game's only thread, and this endpoint is public. Each
// account's answer is kept for ACH_CACHE_TTL seconds, and at most one fresh
// computation runs per second across all accounts: past that budget a stale
// copy is served, or the field is left out. Entries are one per profile-able
// account, so the map stays small.
static const time_t ACH_CACHE_TTL = 120;

struct AchievementsCacheEntry {
    time_t at;
    bool has;
    Json::Value value;
};

static map<DLString, AchievementsCacheEntry> achievements_cache;
static time_t achievements_last_compute = 0;

static bool profile_achievements_cached(const DLString &account, const list<PCMemoryInterface *> &shown, Json::Value &out)
{
    // Keyed by the shown names too: hiding a character makes a new key, so an
    // answer naming it can never be served after the hide.
    DLString id = account;
    for (PCMemoryInterface *pci : shown)
        id << " " << pci->getName();

    time_t now = dreamland->getCurrentTime();
    auto it = achievements_cache.find(id);
    bool cached = it != achievements_cache.end();

    if (!cached || now - it->second.at >= ACH_CACHE_TTL) {
        if (achievements_last_compute != now) {
            achievements_last_compute = now;
            AchievementsCacheEntry &e = achievements_cache[id];
            e.at = now;
            e.has = profile_achievements(shown, e.value);
            it = achievements_cache.find(id);
            cached = true;
        }
    }

    if (!cached || !it->second.has)
        return false;

    out = it->second.value;
    return true;
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
    body["account"] = profile_account_stats(shown);
    body["chars"] = Json::Value(Json::arrayValue);

    // The main character: the one played the longest over all its lives.
    PCMemoryInterface *mainChar = 0;
    int mainHours = -1;
    for (PCMemoryInterface *pci : shown) {
        int h = profile_char_hours(pci);
        if (h > mainHours) {
            mainHours = h;
            mainChar = pci;
        }
    }

    for (PCMemoryInterface *pci : shown) {
        bool isOnline = false;
        for (PCharacter *pch : online)
            if (pch->getName() == pci->getName()) {
                isOnline = true;
                break;
            }
        Json::Value entry = profile_char_entry(pci, isOnline);
        if (pci == mainChar)
            entry["main"] = true;
        body["chars"].append(entry);
    }

    Json::Value achievements;
    if (profile_achievements_cached(id, shown, achievements))
        body["achievements"] = achievements;

    servlet_response_200_json(response, body);
}

SERVLET_HANDLE(api_profile, "/profile")
{
    profile_servlet(request, response);
}
