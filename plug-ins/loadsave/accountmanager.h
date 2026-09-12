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
class PCharacter;

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

    // The player-facing account name (a generated fantasy title, e.g. "Ashen Warden
    // of Old Thalos"). Falls back to the raw id for a legacy account minted before
    // titles existed. The id stays the internal key; players only ever see the title.
    static DLString titleOf(const DLString &id);

    // Make an externally-supplied string (a redeemer's display, a Discord username)
    // safe to echo THROUGH the mudtag renderer -- doubles '{', drops control bytes,
    // clamps length. One escaper shared by every account surface (the minter echo,
    // the in-game adopt). See the definition for why colourStrip is the wrong tool.
    static DLString echoSafe(const DLString &raw);

    // A DIFFERENT, mortal, in-world character on the same account as charName, or
    // "" (the same-account simultaneous-login block). Returns "" for an unattached
    // char; never reports the char itself (a reconnect is not a conflict) nor an
    // immortal (gods switch/test). The caller still exempts an immortal logging in.
    static DLString conflictingOnlineChar(const DLString &charName);

    // --- account-wide config (screenreader, colour, language, spam toggles) ---
    // The account is the source of truth for these keys; per-character config
    // (prompt, wimpy, auto-flags, aliases) stays on the pfile, untouched. getConfig
    // returns the account["config"] object (null when none). applyConfigToChar is
    // called on login (CON_PLAYING) to push the account's keys onto the entering
    // character. propagateConfigKey pushes one just-changed key to the account's
    // OTHER online characters live. See ACCOUNTS_NANNY_ROADMAP.md.
    static Json::Value getConfig(const DLString &id);
    static void applyConfigToChar(PCharacter *ch);
    static void propagateConfigKey(const DLString &id, const DLString &key,
                                   const Json::Value &value, PCharacter *except);

    // --- mutations (persist immediately) ---
    // No callers until the linking-code / redeem surface lands in a later phase.
    // Caller contract (the redeem surface must honour it):
    //  - create/addIdentity mark the identity verified, so call them ONLY after the
    //    identity has actually been proven (emailed-code round-trip, bot DM, OAuth).
    //  - attachChar does NOT check for an existing link on the char; the surface decides
    //    the re-attach policy (refuse, or detach first) so a second code can't steal a char.
    //  - create() returns "" if the on-disk write fails (db/account must exist).
    static DLString create(const DLString &type, const DLString &value, const DLString &display); // -> new id, "" on failure
    static bool addIdentity(const DLString &id, const DLString &type, const DLString &value, const DLString &display);
    static bool attachChar(const DLString &id, const DLString &charName);
    static bool detachChar(const DLString &charName);

    // Write one account-wide config key and persist. Used by the `config` command's
    // write-through when a linked character changes an account-wide option.
    static bool setConfigKey(const DLString &id, const DLString &key, const Json::Value &value);

private:
    // Apply a single account-wide config key to a character (maps the key to the
    // right flag/attribute). Shared by applyConfigToChar and propagateConfigKey.
    static void applyConfigKeyToChar(PCharacter *ch, const DLString &key, const Json::Value &value);

    static DLString mintId();
    static DLString mintTitle();
    static bool saveAccount(const DLString &id);
    static DLString identityKey(const DLString &type, const DLString &value);
    static void indexIdentities(const DLString &id, const Json::Value &account);

    static const DLString ACCOUNT_TABLE;
    static const DLString ACCOUNT_EXT;

    static std::map<DLString, Json::Value> accounts;    // id -> account record
    static std::map<DLString, DLString> identityIndex;  // "type\tvalue" -> id
};

#endif
