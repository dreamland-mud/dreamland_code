/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <map>
#include <list>
#include <jsoncpp/json/json.h>
#include "dlstring.h"

class PCMemoryInterface;

/**
 * The account layer.
 *
 * An account is an internal id plus one or more verified identities
 * (email / telegram / discord). The registry persists as one JSON file per
 * account at db/account/<id>.json and holds ONLY the identities. A character's
 * membership is the JSON attribute "account" ({"id":<id>}) stored on its own
 * pfile -- so renames and deletes need no registry upkeep, the link rides the
 * pfile (find_players_by_json_attribute scans the in-RAM playerbase to enumerate
 * an account's characters).
 *
 * Ships "dark": with an empty registry every mutator is unreachable and the
 * boot reconcile pass is skipped, so nothing observable changes until a redeem
 * surface (a later phase) starts calling create()/attachChar().
 */
class AccountManager {
public:
    // Boot: load db/account/*.json into the in-RAM registry. Only if at least one
    // account exists, log any character whose "account" attr points at a missing id.
    static void load();

    // --- reads (in-RAM registry / cheap pfile scans) ---
    static bool exists(const DLString &id);
    static Json::Value get(const DLString &id);                 // null Value when absent
    static DLString findByIdentity(const DLString &type, const DLString &value); // "" when none
    static DLString accountOf(const DLString &charName);        // "" when unattached
    static std::list<DLString> charsOf(const DLString &id);     // member character names

    // --- mutations (persist immediately) ---
    // No callers until the linking-code / redeem surface lands in a later phase.
    static DLString create(const DLString &type, const DLString &value, const DLString &display); // -> new id
    static bool addIdentity(const DLString &id, const DLString &type, const DLString &value, const DLString &display);
    static bool attachChar(const DLString &id, const DLString &charName);
    static bool detachChar(const DLString &charName);

private:
    static DLString mintId();
    static bool saveAccount(const DLString &id);
    static DLString identityKey(const DLString &type, const DLString &value);
    static void indexIdentities(const DLString &id, const Json::Value &account);

    static const DLString ACCOUNT_TABLE;
    static const DLString ACCOUNT_EXT;

    static std::map<DLString, Json::Value> accounts;    // id -> account record
    static std::map<DLString, DLString> identityIndex;  // "type\tvalue" -> id
};

#endif
