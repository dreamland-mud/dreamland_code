/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <jsoncpp/json/json.h>

#include "accountmanager.h"
#include "commonattributes.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "pcharacter.h"
#include "character.h"
#include "merc.h"
#include "def.h"
#include "math_utils.h"
#include "json_utils.h"

#include "dreamland.h"
#include "dldirectory.h"
#include "dlfilestream.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"
#include "logstream.h"

using namespace std;

const DLString AccountManager::ACCOUNT_TABLE = "account";
const DLString AccountManager::ACCOUNT_EXT = ".json";

map<DLString, Json::Value> AccountManager::accounts;
map<DLString, DLString> AccountManager::identityIndex;

DLString AccountManager::identityKey(const DLString &type, const DLString &value)
{
    return type + "\t" + value;
}

void AccountManager::indexIdentities(const DLString &id, const Json::Value &account)
{
    const Json::Value &identities = account["identities"];
    for (Json::Value::const_iterator i = identities.begin(); i != identities.end(); ++i) {
        if (!(*i).isObject())
            continue;

        const Json::Value &type = (*i)["type"];
        const Json::Value &value = (*i)["value"];
        if (!type.isString() || !value.isString())
            continue;

        DLString key = identityKey(type.asString(), value.asString());
        map<DLString, DLString>::iterator existing = identityIndex.find(key);
        if (existing != identityIndex.end() && existing->second != id)
            LogStream::sendWarning() << "Accounts: identity " << type.asString() << ":" << value.asString()
                << " maps to both " << existing->second << " and " << id << " (last wins)." << endl;

        identityIndex[key] = id;
    }
}

void AccountManager::load()
{
    accounts.clear();
    identityIndex.clear();

    DLDirectory dir(dreamland->getDbDir(), ACCOUNT_TABLE);

    try {
        dir.open();
    } catch (const ExceptionDBIO &e) {
        LogStream::sendNotice() << "Accounts: no db/account directory yet, ships dark." << endl;
        return;
    }

    int count = 0;
    try {
        for (;;) {
            // nextTypedEntry throws ExceptionDBIOEOF at the end -> outer catch breaks.
            DLFile entry = dir.nextTypedEntry(ACCOUNT_EXT);

            // One malformed or unreadable file must never take down the boot: a non-object
            // JSON root or a numeric id throws Json::LogicError, an unreadable file throws
            // ExceptionDBIO -- both are std::exception and would otherwise escape to
            // std::terminate. Skip the offending file instead.
            try {
                ostringstream buf;
                DLFileStream(dir, entry).toStream(buf);

                Json::Value account;
                JsonUtils::fromString(buf.str(), account);
                if (!account.isObject() || !account["id"].isString()) {
                    LogStream::sendError() << "Accounts: file " << entry.getFileName()
                        << " is not a valid account object, skipping." << endl;
                    continue;
                }

                DLString id = account["id"].asString();
                accounts[id] = account;
                indexIdentities(id, account);
                count++;
            } catch (const std::exception &e) {
                LogStream::sendError() << "Accounts: skipping " << entry.getFileName()
                    << ": " << e.what() << endl;
            }
        }
    } catch (const ExceptionDBIOEOF &) {
        // normal end of directory
    }

    dir.close();
    LogStream::sendNotice() << "Accounts: loaded " << count << "." << endl;

    // Reconcile only when there is something to reconcile -- ships dark == no scan.
    // Probe with findAttr, NOT get_json_attribute: the latter goes through getAttr, which
    // CREATES an empty "account" attribute on every scanned player and churns it into
    // their pfile on next save.
    if (!accounts.empty()) {
        for (auto &p : PCharacterManager::getPCM()) {
            XMLStringAttribute::Pointer attr = p.second->getAttributes().findAttr<XMLStringAttribute>("account");
            if (!attr)
                continue;

            Json::Value acc;
            JsonUtils::fromString(attr->getValue(), acc);
            if (!acc.isObject() || !acc["id"].isString())
                continue;

            DLString id = acc["id"].asString();
            if (accounts.find(id) == accounts.end())
                LogStream::sendWarning() << "Accounts: character " << p.first
                    << " links to missing account " << id << "." << endl;
        }
    }
}

bool AccountManager::exists(const DLString &id)
{
    return accounts.find(id) != accounts.end();
}

Json::Value AccountManager::get(const DLString &id)
{
    map<DLString, Json::Value>::iterator i = accounts.find(id);
    if (i == accounts.end())
        return Json::Value();
    return i->second;
}

DLString AccountManager::findByIdentity(const DLString &type, const DLString &value)
{
    map<DLString, DLString>::iterator i = identityIndex.find(identityKey(type, value));
    if (i == identityIndex.end())
        return DLString::emptyString;
    return i->second;
}

DLString AccountManager::mintId()
{
    DLString id;
    for (;;) {
        id = create_secure_nonce(6);
        if (accounts.find(id) == accounts.end())
            break;
    }
    return id;
}

// Player-facing account titles. The id stays the internal key (filename, pfile
// attr, `account admin` reference); a player only ever sees this fantasy title.
// Titles do NOT need to be unique -- the id is the key -- so a collision is
// harmless and we never track used combinations. In-world Thera register.
// ~84 x ~84 x ~56 = ~395k combinations from three static lists, composed at
// create() with the merc RNG. No LLM, no data file, no per-creation cost.
static const char *ACCOUNT_ADJ[] = {
    "Ashen", "Silent", "Grey", "Pale", "Hollow", "Sundered", "Gilded", "Shattered",
    "Weeping", "Fallen", "Crimson", "Obsidian", "Ivory", "Molten", "Frostbound",
    "Thornbound", "Withered", "Ancient", "Nameless", "Wandering", "Ember", "Cinder",
    "Verdant", "Dusken", "Stormborn", "Riven", "Umbral", "Argent", "Sable", "Feral",
    "Hallowed", "Cursed", "Dread", "Iron", "Bronze", "Leaden", "Glass", "Starlit",
    "Moonlit", "Sunless", "Bleak", "Mournful", "Restless", "Forsaken", "Hidden",
    "Veiled", "Grim", "Vagrant", "Solemn", "Tarnished", "Kindled", "Quiet", "Scarred",
    "Wan", "Bitter", "Hoary", "Dim", "Radiant", "Ruined", "Drowned", "Blighted",
    "Wintering", "Wayworn", "Sombre", "Ragged", "Gaunt", "Hushed", "Shrouded",
    "Errant", "Twilit", "Nightbound", "Stonewrought", "Rimebound", "Wolfish",
    "Ravenous", "Emberclad", "Palewrought", "Stormworn", "Ghostlit", "Direful",
    "Lorn", "Wroth", "Waning", "Everdark",
};
static const char *ACCOUNT_NOUN[] = {
    "Warden", "Sidhe", "Herald", "Seeker", "Wyrm", "Raven", "Oracle", "Fox",
    "Pilgrim", "Sentinel", "Shade", "Revenant", "Wanderer", "Lantern", "Serpent",
    "Griffin", "Wolf", "Stag", "Sparrow", "Owl", "Mantis", "Reaper", "Scribe",
    "Keeper", "Hermit", "Vagabond", "Marauder", "Witness", "Mourner", "Harbinger",
    "Exile", "Nomad", "Ferryman", "Gravedigger", "Bellringer", "Watcher", "Dreamer",
    "Sleepwalker", "Cartographer", "Wisp", "Basilisk", "Chimera", "Manticore",
    "Direwolf", "Nightjar", "Heron", "Crane", "Adder", "Viper", "Lynx", "Boar",
    "Hound", "Kestrel", "Falcon", "Vulture", "Magpie", "Jackdaw", "Wight", "Lich",
    "Ghoul", "Banshee", "Drake", "Cockatrice", "Salamander", "Golem", "Effigy",
    "Idol", "Pallbearer", "Almoner", "Beadle", "Verger", "Warlock", "Templar",
    "Corsair", "Outrider", "Sellsword", "Gravewalker", "Lampwright", "Bonesetter",
    "Nightwarden", "Stormcaller", "Ashwalker", "Moonhound", "Fenwyrm",
};
static const char *ACCOUNT_PLACE[] = {
    "Old Thalos", "the Elder Days", "the Sundered Marches", "Midgaard's Gate",
    "the Hollow Vale", "the Frost Marches", "Ninefold Dusk", "the Weeping Vale",
    "the Ashen Wastes", "the Drowned Coast", "the Silent Fen", "the Broken Spire",
    "the Last Bastion", "the Grey Expanse", "the Withered Wood", "the Umbral Deep",
    "the Starless Reach", "the Forgotten Ford", "the Bleeding Hills", "the Kindled Waste",
    "the Riven Peaks", "the Mournful Shore", "the Endless Steppe", "the Shrouded Isles",
    "the Dying Light", "the Iron Marches", "the Glass Desert", "the Sleeping Deep",
    "the Twilit Span", "the Gallows Road", "the Salt Wastes", "the Cinder Reach",
    "the Thornwood", "the Pale Meridian", "the Long Dark", "the Shattered Crown",
    "the Wandering Stars", "the Amber Vault", "the Hushed Hollow", "the Nine Gates",
    "the Sunless Sea", "the Ember Marches", "the Rimebound North", "the Verdant Ruin",
    "the Widow's Watch", "the Crooked Mile", "the Fallow Reach", "the Sable Fen",
    "the Quiet Lands", "the Waning Moon", "the Broken Oath", "the First Dark",
    "the Hanged Wood", "the Weeping Gate", "the Grey Reach", "the Sombre Steppe",
};

DLString AccountManager::mintTitle()
{
    int na = sizeof(ACCOUNT_ADJ) / sizeof(ACCOUNT_ADJ[0]);
    int nn = sizeof(ACCOUNT_NOUN) / sizeof(ACCOUNT_NOUN[0]);
    int np = sizeof(ACCOUNT_PLACE) / sizeof(ACCOUNT_PLACE[0]);
    DLString adj = ACCOUNT_ADJ[number_range(0, na - 1)];
    DLString noun = ACCOUNT_NOUN[number_range(0, nn - 1)];
    DLString place = ACCOUNT_PLACE[number_range(0, np - 1)];
    return adj + " " + noun + " of " + place;
}

DLString AccountManager::titleOf(const DLString &id)
{
    map<DLString, Json::Value>::iterator i = accounts.find(id);
    if (i != accounts.end()) {
        // .get (not operator[]) so a title-less legacy record is not MUTATED with a
        // null "title" that a later saveAccount would then persist.
        Json::Value t = i->second.get("title", Json::Value());
        if (t.isString() && !t.asString().empty())
            return t.asString();
    }
    // Legacy account minted before titles, or an unknown id: the id itself is the
    // only stable thing to show. (No such accounts exist on the live registry.)
    return id;
}

// Make an externally-supplied string safe to echo THROUGH the mudtag renderer:
// send_to re-parses the composed message (args included) for mudtags, so
// colourStrip is the WRONG tool -- it un-escapes {{ -> { and re-arms every tag.
// Instead clamp the raw first (so the clamp can't split a doubled brace), then
// double every '{' to '{{' (mudtags renders '{{' as a literal '{', so no lone
// '{'+letter tag can survive), and drop control bytes so no newline/ANSI reaches
// the player's terminal. KOI8 high bytes (>= 0x80, Cyrillic) are kept.
DLString AccountManager::echoSafe(const DLString &raw)
{
    DLString clamped = raw;
    if (clamped.size() > 40)
        clamped = clamped.substr(0, 40) + "...";

    DLString out;
    for (int i = 0; i < (int)clamped.size(); i++) {
        char c = clamped[i];
        if (c == '{')
            out += "{{";
        else if ((unsigned char)c >= 0x20)
            out += c;
    }
    return out;
}

bool AccountManager::saveAccount(const DLString &id)
{
    map<DLString, Json::Value>::iterator a = accounts.find(id);
    if (a == accounts.end())
        return false;

    try {
        DLDirectory dir(dreamland->getDbDir(), ACCOUNT_TABLE);
        // db/account may not exist on a fresh environment: DLFileStream opens the
        // output path directly (no mkdir), so the write would throw and the account
        // would evaporate on the next reboot. Create the table dir first.
        if (!dir.exist())
            ::mkdir(dir.getCPath(), 0775);
        DLFileStream(dir, id, ACCOUNT_EXT).fromString(JsonUtils::toString(a->second));
        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << "Accounts: saving " << id << " failed: " << e.what() << endl;
        return false;
    }
}

DLString AccountManager::create(const DLString &type, const DLString &value, const DLString &display)
{
    // Belt-and-braces: an identity belongs to at most one account. The redeem
    // surface find-before-creates, so this only fires on a caller bug -- return
    // the existing owner rather than mint a duplicate that would corrupt the
    // last-wins identity index (indexIdentities warns and overwrites).
    DLString owner = findByIdentity(type, value);
    if (!owner.empty()) {
        LogStream::sendWarning() << "Accounts: create() for already-owned identity "
            << type << ":" << value << " -> returning existing " << owner << "." << endl;
        return owner;
    }

    DLString id = mintId();

    Json::Value identity;
    identity["type"] = type;
    identity["value"] = value;
    identity["display"] = display;
    identity["verified"] = true;

    Json::Value account;
    account["id"] = id;
    account["title"] = mintTitle();
    account["identities"].append(identity);

    accounts[id] = account;
    indexIdentities(id, account);

    // A create that never reached disk must not hand out a "linked" account that
    // evaporates on the next reboot -- roll back the RAM state and report failure.
    // (db/account must exist; see the deploy note in the header.)
    if (!saveAccount(id)) {
        accounts.erase(id);
        for (map<DLString, DLString>::iterator i = identityIndex.begin(); i != identityIndex.end(); ) {
            if (i->second == id)
                identityIndex.erase(i++);
            else
                ++i;
        }
        return DLString::emptyString;
    }

    return id;
}

bool AccountManager::addIdentity(const DLString &id, const DLString &type, const DLString &value, const DLString &display)
{
    map<DLString, Json::Value>::iterator a = accounts.find(id);
    if (a == accounts.end())
        return false;

    // An identity belongs to at most one account.
    DLString owner = findByIdentity(type, value);
    if (!owner.empty())
        return owner == id;

    Json::Value identity;
    identity["type"] = type;
    identity["value"] = value;
    identity["display"] = display;
    identity["verified"] = true;

    a->second["identities"].append(identity);
    identityIndex[identityKey(type, value)] = id;

    // Roll back the RAM/index append if the write never reached disk, so the
    // registry does not claim a "verified" identity that vanishes on reboot.
    if (!saveAccount(id)) {
        Json::Value &ids = a->second["identities"];
        if (ids.isArray() && ids.size() > 0)
            ids.resize(ids.size() - 1);
        identityIndex.erase(identityKey(type, value));
        return false;
    }

    return true;
}

bool AccountManager::attachChar(const DLString &id, const DLString &charName)
{
    if (!exists(id))
        return false;

    // PCharacterManager::find is an exact lookup keyed by capitalize()d Latin names.
    DLString name = charName;
    PCMemoryInterface *pc = PCharacterManager::find(name.capitalize());
    if (pc == 0)
        return false;

    Json::Value acc;
    acc["id"] = id;
    set_json_attribute(pc, "account", acc);
    PCharacterManager::saveMemory(pc);
    return true;
}

bool AccountManager::detachChar(const DLString &charName)
{
    DLString name = charName;
    PCMemoryInterface *pc = PCharacterManager::find(name.capitalize());
    if (pc == 0)
        return false;

    pc->getAttributes().eraseAttribute("account");
    PCharacterManager::saveMemory(pc);
    return true;
}

DLString AccountManager::accountOf(const DLString &charName)
{
    DLString name = charName;
    PCMemoryInterface *pc = PCharacterManager::find(name.capitalize());
    if (pc == 0)
        return DLString::emptyString;

    // Probe with findAttr, not get_json_attribute, to avoid creating an empty attribute.
    XMLStringAttribute::Pointer attr = pc->getAttributes().findAttr<XMLStringAttribute>("account");
    if (!attr)
        return DLString::emptyString;

    Json::Value acc;
    JsonUtils::fromString(attr->getValue(), acc);
    if (acc.isObject() && acc["id"].isString())
        return acc["id"].asString();

    return DLString::emptyString;
}

list<DLString> AccountManager::charsOf(const DLString &id)
{
    list<DLString> names;
    for (PCMemoryInterface *pc : find_players_by_json_attribute("account", "id", id))
        names.push_back(pc->getName());
    return names;
}

DLString AccountManager::conflictingOnlineChar(const DLString &charName)
{
    DLString name = charName;
    name.capitalize();

    DLString myAccount = accountOf(name);
    if (myAccount.empty())
        return DLString::emptyString;   // an unattached char never conflicts

    // char_list is the in-world set: a descriptor still in the nanny (typing name
    // or password) is not in it yet, so a half-logged-in connection is naturally
    // exempt. The char itself (a reconnect/reanimate) and immortals are skipped.
    for (Character *wch = char_list; wch != 0; wch = wch->next) {
        if (wch->is_npc())
            continue;

        PCharacter *pch = wch->getPC();
        if (pch == 0 || pch->getName() == name)
            continue;
        if (pch->get_trust() >= LEVEL_IMMORTAL)
            continue;

        if (accountOf(pch->getName()) == myAccount)
            return pch->getName();
    }

    return DLString::emptyString;
}

Json::Value AccountManager::getConfig(const DLString &id)
{
    map<DLString, Json::Value>::iterator i = accounts.find(id);
    if (i == accounts.end())
        return Json::Value();
    // .get (not operator[]) so a config-less record is never mutated with a null
    // "config" that a later saveAccount would then persist.
    return i->second.get("config", Json::Value());
}

bool AccountManager::setConfigKey(const DLString &id, const DLString &key, const Json::Value &value)
{
    map<DLString, Json::Value>::iterator i = accounts.find(id);
    if (i == accounts.end())
        return false;
    // operator[] creates the "config" object on first write -- intended.
    i->second["config"][key] = value;
    return saveAccount(id);
}

// Map ONE account-wide config key onto a character. Keys mirror the config option
// EN names (screenreader / fightspam / skillspam / noweaponspam), plus the special
// tri-state "color" and the "lang" attribute. An unknown key is ignored, so a newer
// build can add keys without an older one choking on them.
void AccountManager::applyConfigKeyToChar(PCharacter *ch, const DLString &key, const Json::Value &value)
{
    if (ch == 0)
        return;

    if (key == "screenreader") {
        if (value.asBool()) SET_BIT(ch->config, CONFIG_SCREENREADER);
        else REMOVE_BIT(ch->config, CONFIG_SCREENREADER);
    }
    else if (key == "fightspam") {
        if (value.asBool()) SET_BIT(ch->config, CONFIG_FIGHTSPAM);
        else REMOVE_BIT(ch->config, CONFIG_FIGHTSPAM);
    }
    else if (key == "skillspam") {
        if (value.asBool()) SET_BIT(ch->config, CONFIG_SKILLSPAM);
        else REMOVE_BIT(ch->config, CONFIG_SKILLSPAM);
    }
    else if (key == "noweaponspam") {
        // Key is the option name (see config.xml): the CONFIG_WEAPONSPAM bit is
        // inverted -- set = HIDE weapon-flag effects. We store/apply the raw bit
        // under the option's own name, so the semantics stay self-consistent.
        if (value.asBool()) SET_BIT(ch->config, CONFIG_WEAPONSPAM);
        else REMOVE_BIT(ch->config, CONFIG_WEAPONSPAM);
    }
    else if (key == "color") {
        // Tri-state across two bits, mirroring config_color: off / on / mild.
        DLString c = value.asString();
        if (c == "off") {
            REMOVE_BIT(ch->act, PLR_COLOR);
            REMOVE_BIT(ch->comm, COMM_MILDCOLOR);
        } else if (c == "mild") {
            SET_BIT(ch->act, PLR_COLOR);
            SET_BIT(ch->comm, COMM_MILDCOLOR);
        } else if (c == "on") {
            SET_BIT(ch->act, PLR_COLOR);
            REMOVE_BIT(ch->comm, COMM_MILDCOLOR);
        }
    }
    else if (key == "lang") {
        DLString l = value.asString();
        if (l == "en" || l == "ru" || l == "ua")
            ch->getAttributes().getAttr<XMLStringAttribute>("lang")->setValue(l);
    }
}

void AccountManager::applyConfigToChar(PCharacter *ch)
{
    if (ch == 0)
        return;

    DLString id = accountOf(ch->getName());
    if (id.empty())
        return;

    Json::Value cfg = getConfig(id);
    if (!cfg.isObject())
        return;

    // A hand-edited account file could hold a wrong-typed value (asBool/asString on
    // the wrong JSON type throws Json::LogicError); the login path has no upstream
    // std::exception catch, so guard here -- a corrupt file costs a log line, not the
    // boot. Same defensive stance as load() (see the malformed-file note there).
    try {
        for (Json::Value::const_iterator i = cfg.begin(); i != cfg.end(); ++i)
            applyConfigKeyToChar(ch, i.key().asString(), *i);
    } catch (const std::exception &e) {
        LogStream::sendError() << "Accounts: bad config value applying to "
            << ch->getName() << ": " << e.what() << endl;
    }
}

void AccountManager::propagateConfigKey(const DLString &id, const DLString &key,
                                        const Json::Value &value, PCharacter *except)
{
    // Push a just-changed account-wide key to the account's OTHER online characters,
    // live. char_list is the in-world set; the char that made the change already has
    // it (set by the config command), so it is excepted.
    for (Character *wch = char_list; wch != 0; wch = wch->next) {
        if (wch->is_npc())
            continue;
        PCharacter *pch = wch->getPC();
        if (pch == 0 || pch == except)
            continue;
        if (accountOf(pch->getName()) == id)
            applyConfigKeyToChar(pch, key, value);
    }
}
