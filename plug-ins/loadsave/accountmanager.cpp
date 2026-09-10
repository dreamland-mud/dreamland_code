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
