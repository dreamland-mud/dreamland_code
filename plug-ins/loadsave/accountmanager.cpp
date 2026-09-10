/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
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
#include "dlfileop.h"
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
        DLString type = (*i)["type"].asString();
        DLString value = (*i)["value"].asString();
        if (!type.empty() && !value.empty())
            identityIndex[identityKey(type, value)] = id;
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
            DLFile entry = dir.nextTypedEntry(ACCOUNT_EXT);

            ostringstream buf;
            DLFileStream(dir, entry).toStream(buf);

            Json::Value account;
            JsonUtils::fromString(buf.str(), account);

            DLString id = account["id"].asString();
            if (id.empty()) {
                LogStream::sendError() << "Accounts: file " << entry.getFileName() << " has no id, skipping." << endl;
                continue;
            }

            accounts[id] = account;
            indexIdentities(id, account);
            count++;
        }
    } catch (const ExceptionDBIOEOF &) {
        // normal end of directory
    }

    dir.close();
    LogStream::sendNotice() << "Accounts: loaded " << count << "." << endl;

    // Reconcile only when there is something to reconcile -- ships dark == no scan.
    if (!accounts.empty()) {
        for (auto &p : PCharacterManager::getPCM()) {
            Json::Value acc;
            if (get_json_attribute(p.second, "account", acc)) {
                DLString id = acc["id"].asString();
                if (!id.empty() && accounts.find(id) == accounts.end())
                    LogStream::sendWarning() << "Accounts: character " << p.first
                        << " links to missing account " << id << "." << endl;
            }
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
        DLFileStream(dir, id, ACCOUNT_EXT).fromString(JsonUtils::toString(a->second));
        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << "Accounts: saving " << id << " failed: " << e.what() << endl;
        return false;
    }
}

DLString AccountManager::create(const DLString &type, const DLString &value, const DLString &display)
{
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
    saveAccount(id);

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
    return saveAccount(id);
}

bool AccountManager::attachChar(const DLString &id, const DLString &charName)
{
    if (!exists(id))
        return false;

    PCMemoryInterface *pc = PCharacterManager::find(charName);
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
    PCMemoryInterface *pc = PCharacterManager::find(charName);
    if (pc == 0)
        return false;

    pc->getAttributes().eraseAttribute("account");
    PCharacterManager::saveMemory(pc);
    return true;
}

DLString AccountManager::accountOf(const DLString &charName)
{
    PCMemoryInterface *pc = PCharacterManager::find(charName);
    if (pc == 0)
        return DLString::emptyString;

    Json::Value acc;
    if (get_json_attribute(pc, "account", acc))
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
