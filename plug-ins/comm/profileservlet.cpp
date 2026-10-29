/* profileservlet -- the public, read-only account profile behind
 * dreamland.rocks/player/<id>. Trello dIW1M9t8.
 *
 *   /profile {args:{id}} or {args:{char}} -> account title + its visible characters
 *   GET /playermeta/<id or name> -> the page's <title> and og:* tags (nginx SSI)
 *
 * No token: this is the page anyone can open, same footing as the public /who.
 * Linking characters into one account is information `who`/`whois` never give
 * out, so the profile only exists when all three hold:
 *   - at least one character on the account is past the newbie stage
 *     (Player::isNewbie: reborn, or more than 50 questor tasks), hidden
 *     characters included, so hiding that one cannot leave fresh alts on show;
 *   - the owner has not switched it off (account config "profile": false);
 *   - at least one character is not hidden (pfile json attribute
 *     "profile": {"hidden": true}, kept on the char so a rename keeps it hidden).
 * Anything else answers 404, indistinguishable from a missing account.
 *
 * Each character carries what an anonymous `whois` shows: race, class, remorts,
 * Legendary/Immortal rank, a non-hidden clan, titles and online state. Level
 * stays out (who conceals it, see can_see_level). The account block sums the
 * SHOWN characters only (playing since, hours across every life, lives lived,
 * the class with the most hours), so a hidden alt never moves a number. Earned
 * account achievements are the exception: they belong to the whole account and
 * stay on the page when the alt holding them is hidden (Kit 2026-10-24).
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
#include "hometown.h"
#include "room.h"
#include "area.h"
#include "commonattributes.h"
#include "fenia/register-impl.h"
#include "fenia/object.h"
#include "feniamanager.h"
#include "wrapperbase.h"
#include "reglist.h"
#include "json_utils_ext.h"
#include "dreamland.h"
#include "iconvmap.h"
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

// At least one character on the account is not a newbie (Kit 2026-10-25:
// the engine's own definition, so a long-time player who never remorted
// gets a page too).
bool account_profile_eligible(const DLString &id)
{
    for (const DLString &name : AccountManager::charsOf(id)) {
        PCMemoryInterface *pci = PCharacterManager::find(name);
        if (pci != 0 && !Player::isNewbie(pci))
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
// Shared with accountservlet.cpp (roster order).
int profile_char_hours(PCMemoryInterface *pci)
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

    // The hometown's area file keys the zone banner behind the profile header.
    if (pci->getHometown()->isValid()) {
        RoomIndexData *pRoom = get_room_index(pci->getHometown()->getRecall());
        if (pRoom && pRoom->areaIndex && pRoom->areaIndex->area_file)
            c["home"] = pRoom->areaIndex->area_file->file_name.c_str();
    }

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

// .tmp.achievements.hideBlocker(name, lang): "" when the character may leave
// the public page, else the name of one achievement only it brings to the
// account (Kit 2026-10-24: only alts that add nothing new may hide). No
// script or a throw answer "" -- hiding is the owner's privacy switch, a
// broken script must not take it away.
DLString account_profile_hide_blocker(const DLString &charName, lang_t lang)
{
    using namespace Scripting;

    if (!FeniaManager::wrapperManager)
        return DLString::emptyString;

    static IdRef ID_TMP("tmp"), ID_ACHIEVEMENTS("achievements"), ID_HIDEBLOCKER("hideBlocker");

    try {
        Register tmp = *Context::root[ID_TMP];
        Register ach = *tmp[ID_ACHIEVEMENTS];
        if (ach.type != Register::OBJECT)
            return DLString::emptyString;

        Register function = *ach[ID_HIDEBLOCKER];
        if (function.type != Register::FUNCTION)
            return DLString::emptyString;

        RegisterList args;
        args.push_back(Register(charName));
        args.push_back(Register((int)lang));

        Register result = function.toFunction()->invoke(ach, args);
        if (result.type != Register::STRING)
            return DLString::emptyString;
        return result.toString();

    } catch (const ::Exception &e) {
        FeniaManager::getThis()->croak(0, Register(DLString("achievements.hideBlocker")), e);
        return DLString::emptyString;
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

// A character by login first, then by any name the game shows for it
// (English, Russian, Ukrainian): the site's search box sends whatever the
// player typed. A name match must be visible and on an account.
static PCMemoryInterface *profile_find_char(const DLString &name)
{
    DLString login = name;
    PCMemoryInterface *pci = PCharacterManager::find(login.capitalize());
    if (pci != 0)
        return pci;

    for (auto &pair: PCharacterManager::getPCM()) {
        PCMemoryInterface *p = pair.second;
        if (!(name ^ p->getNameP('1', LANG_RU)) && !(name ^ p->getNameP('1', LANG_UA))
            && !(name ^ p->getNameP('1', LANG_EN)))
            continue;
        if (account_profile_char_hidden(p) || AccountManager::accountOf(p->getName()).empty())
            continue;
        return p;
    }
    return 0;
}

// The account whose page an id or a character name opens, and its shown
// characters. False when there is no page: a missing or hidden character, an
// unknown account, a page switched off or not open yet. By name the character
// must itself be visible, so a hidden alt's name never leads to the account.
static bool profile_resolve(DLString id, const DLString &charName, DLString &account,
                            PCMemoryInterface *&byChar, list<PCMemoryInterface *> &shown)
{
    byChar = 0;
    if (id.empty() && !charName.empty()) {
        byChar = profile_find_char(charName);
        if (byChar == 0 || account_profile_char_hidden(byChar))
            return false;
        id = AccountManager::accountOf(byChar->getName());
    }

    id.toUpper();
    if (!profile_valid_id(id) || !AccountManager::exists(id)
        || !account_profile_enabled(id) || !account_profile_eligible(id))
        return false;

    for (const DLString &name : AccountManager::charsOf(id)) {
        PCMemoryInterface *pci = PCharacterManager::find(name);
        if (pci != 0 && !account_profile_char_hidden(pci))
            shown.push_back(pci);
    }

    account = id;
    return !shown.empty();
}

// The main character: the one played the longest over all its lives.
static PCMemoryInterface *profile_main_char(const list<PCMemoryInterface *> &shown)
{
    PCMemoryInterface *mainChar = 0;
    int mainHours = -1;
    for (PCMemoryInterface *pci : shown) {
        int h = profile_char_hours(pci);
        if (h > mainHours) {
            mainHours = h;
            mainChar = pci;
        }
    }
    return mainChar;
}

static void profile_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    DLString id, charName;
    servlet_get_arg(params, "id", id);
    servlet_get_arg(params, "char", charName);

    PCMemoryInterface *byChar;
    list<PCMemoryInterface *> shown;
    if (!profile_resolve(id, charName, id, byChar, shown)) {
        servlet_response_404(response, "Not found");
        return;
    }

    list<PCharacter *> online = who_find_online(0);

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

    PCMemoryInterface *mainChar = profile_main_char(shown);

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

/*
 * GET /playermeta/<id or login> -> <title>, description and og:* tags for
 * the head of /player/<...>. Link previews (Telegram, Discord, Facebook) never
 * run the page's script, so nginx pulls this block in with SSI; on a 404 its
 * stub keeps the generic tags. The key follows player.js: six capitals and
 * digits are an account id, anything else a character name. A name previews
 * that character, an id the main one. No Fenia, nothing cached: lookups only.
 */
static IconvMap meta_koi2utf("koi8-u", "utf-8");
static const DLString::size_type META_DESC_MAX = 200;

static DLString meta_escape(const DLString &text)
{
    DLString out;
    for (char ch : text) {
        switch (ch) {
        case '&': out << "&amp;"; break;
        case '<': out << "&lt;"; break;
        case '>': out << "&gt;"; break;
        case '"': out << "&quot;"; break;
        case '\'': out << "&#39;"; break;
        case '\n': case '\r': case '\t': out << ' '; break;
        default: out << ch;
        }
    }
    return out;
}

// The character's own description, English first, cut at a word under
// META_DESC_MAX (KOI8 is one byte a letter, so this counts letters).
static DLString meta_description(PCMemoryInterface *pci)
{
    DLString desc = pci->getDescription(LANG_EN);
    if (desc.empty())
        desc = pci->getDescription(LANG_UA);
    if (desc.empty())
        desc = pci->getDescription(LANG_RU);
    desc = desc.colourStrip();
    desc.replaces("\r", "");
    desc.replaces("\n", " ");
    while (desc.find("  ") != DLString::npos)
        desc.replaces("  ", " ");
    desc.stripWhiteSpace();

    if (desc.size() > META_DESC_MAX) {
        DLString::size_type cut = desc.rfind(' ', META_DESC_MAX);
        if (cut == DLString::npos || cut < META_DESC_MAX / 2)
            cut = META_DESC_MAX;
        desc = desc.substr(0, cut);
        desc << "...";
    }
    return desc;
}

SERVLET_HANDLE(api_playermeta, "/playermeta")
{
    DLString key = request.uri.substr(DLString("/playermeta").size());
    DLString::size_type q = key.find('?');
    if (q != DLString::npos)
        key = key.substr(0, q);
    while (!key.empty() && key.at(0) == '/')
        key.erase(0, 1);
    while (!key.empty() && key.at(key.size() - 1) == '/')
        key.erase(key.size() - 1);

    bool isId = key.size() == 6;
    for (char ch : key)
        if (!isupper((unsigned char)ch) && !isdigit((unsigned char)ch))
            isId = false;

    DLString account;
    PCMemoryInterface *byChar;
    list<PCMemoryInterface *> shown;
    if (key.empty() || key.size() > 16
        || !profile_resolve(isId ? key : DLString::emptyString, isId ? DLString::emptyString : key,
                            account, byChar, shown)) {
        servlet_response_404(response, "Not found");
        return;
    }

    PCMemoryInterface *pci = byChar != 0 ? byChar : profile_main_char(shown);
    DLString name = pci->getNameP('1', LANG_EN);
    DLString title = AccountManager::titleOf(account);
    DLString desc = meta_description(pci);
    if (desc.empty())
        desc = "A DreamLand MUD player's characters: races, classes, rebirths and clans.";

    DLString head = name;
    if (!title.empty())
        head << ", " << title;
    head = meta_escape(head);
    desc = meta_escape(desc);
    DLString url = "https://dreamland.rocks/player/" + account;

    DLString html;
    html << "<title>" << head << " &#8212; DreamLand</title>\n"
         << "<meta name=\"description\" content=\"" << desc << "\"/>\n"
         << "<meta property=\"og:type\" content=\"profile\"/>\n"
         << "<meta property=\"og:site_name\" content=\"DreamLand MUD\"/>\n"
         << "<meta property=\"og:title\" content=\"" << head << "\"/>\n"
         << "<meta property=\"og:description\" content=\"" << desc << "\"/>\n"
         << "<meta property=\"og:url\" content=\"" << url << "\"/>\n"
         << "<meta property=\"og:image\" content=\"https://dreamland.rocks/site/assets/og-card.jpg\"/>\n"
         << "<meta property=\"og:image:width\" content=\"1200\"/>\n"
         << "<meta property=\"og:image:height\" content=\"630\"/>\n"
         << "<meta property=\"og:image:alt\" content=\"The DreamLand logo over a purple map of Thera\"/>\n"
         << "<meta name=\"twitter:card\" content=\"summary_large_image\"/>\n"
         << "<link rel=\"canonical\" href=\"" << url << "\"/>\n";

    servlet_response_200(response, meta_koi2utf(html));
    response.headers["content-type"] = "text/html; charset=utf-8";
}

/*
 * /leaderboard -> {"open": bool, "at": unix, "rows": [{"id", "title", "name":
 * {login, en, ru, ua}, "score", "legendary", "secrets", "earned"}, ...]}
 *
 * The account pages by achievement score, from the daily sums Fenia keeps
 * (.tmp.achievements.boardRows, private world scripts). Only accounts whose
 * page /profile would show are listed, under their main character (most hours
 * among the shown ones). "open" is false until the list opens (Fenia decides).
 * Built at most once per BOARD_CACHE_TTL: the rows only change daily.
 */
static const time_t BOARD_CACHE_TTL = 600;
static const unsigned int BOARD_MAX = 200;

static Json::Value board_cache;
static time_t board_cache_at = 0;

static bool board_rows(Json::Value &out)
{
    using namespace Scripting;

    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP("tmp"), ID_ACHIEVEMENTS("achievements"), ID_BOARDROWS("boardRows");

    try {
        Register tmp = *Context::root[ID_TMP];
        Register ach = *tmp[ID_ACHIEVEMENTS];
        if (ach.type != Register::OBJECT)
            return false;

        Register function = *ach[ID_BOARDROWS];
        if (function.type != Register::FUNCTION)
            return false;

        RegisterList args;
        Register result = function.toFunction()->invoke(ach, args);
        if (result.type == Register::NONE)
            return false;

        out = JsonUtils::fromRegister(result);
        return out.isArray();

    } catch (const ::Exception &e) {
        FeniaManager::getThis()->croak(0, Register(DLString("achievements.boardRows")), e);
        return false;
    }
}

static Json::Value board_build()
{
    Json::Value body;
    Json::Value rows;

    body["at"] = (Json::Int64)dreamland->getCurrentTime();
    body["rows"] = Json::Value(Json::arrayValue);
    body["open"] = board_rows(rows);
    if (!body["open"].asBool())
        return body;

    vector<Json::Value> listed;
    for (const Json::Value &r : rows) {
        DLString id = r["id"].asString();
        id.toUpper();
        if (!profile_valid_id(id) || !AccountManager::exists(id)
            || !account_profile_enabled(id) || !account_profile_eligible(id))
            continue;

        list<PCMemoryInterface *> shown;
        for (const DLString &name : AccountManager::charsOf(id)) {
            PCMemoryInterface *pci = PCharacterManager::find(name);
            if (pci != 0 && !account_profile_char_hidden(pci))
                shown.push_back(pci);
        }
        PCMemoryInterface *mainChar = profile_main_char(shown);
        if (mainChar == 0)
            continue;

        Json::Value row;
        row["id"] = id.c_str();
        row["title"] = AccountManager::titleOf(id).c_str();
        row["name"]["login"] = mainChar->getName().c_str();
        row["name"]["en"] = mainChar->getNameP('1', LANG_EN).c_str();
        row["name"]["ru"] = mainChar->getNameP('1', LANG_RU).c_str();
        row["name"]["ua"] = mainChar->getNameP('1', LANG_UA).c_str();
        row["score"] = r["score"].asInt();
        row["legendary"] = r["legendary"].asInt();
        row["secrets"] = r["secrets"].asInt();
        row["earned"] = r["earned"].asInt();
        listed.push_back(row);
    }

    sort(listed.begin(), listed.end(), [](const Json::Value &a, const Json::Value &b) {
        if (a["score"].asInt() != b["score"].asInt())
            return a["score"].asInt() > b["score"].asInt();
        if (a["legendary"].asInt() != b["legendary"].asInt())
            return a["legendary"].asInt() > b["legendary"].asInt();
        return a["name"]["login"].asString() < b["name"]["login"].asString();
    });

    for (unsigned int i = 0; i < listed.size() && i < BOARD_MAX; i++)
        body["rows"].append(listed[i]);
    return body;
}

SERVLET_HANDLE(api_leaderboard, "/leaderboard")
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    time_t now = dreamland->getCurrentTime();
    if (board_cache_at == 0 || now - board_cache_at >= BOARD_CACHE_TTL) {
        board_cache = board_build();
        board_cache_at = now;
    }
    servlet_response_200_json(response, board_cache);
}
