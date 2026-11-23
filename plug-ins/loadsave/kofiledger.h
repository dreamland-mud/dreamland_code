/* Dream Land, Ko-fi supporter ledger, 2026.
 */
#ifndef KOFILEDGER_H
#define KOFILEDGER_H

#include <list>
#include <jsoncpp/json/json.h>
#include "dlstring.h"

/**
 * One JSON file per Ko-fi transaction in db/kofi/<txn>.json.
 *
 * The file is the idempotency key: a webhook retry for a transaction already on
 * disk changes nothing. Each entry keeps the whitelisted payment fields (as the
 * broker sent them), how it matched an account, the account id (or null), the
 * months granted and a status:
 *   applied   -- Donation/Subscription credited to an account (months may be 0)
 *   unmatched -- Donation/Subscription with no account found; claimable in-game
 *   manual    -- Shop Order/Commission/unknown type, or a credit that failed to
 *                persist; an immortal handles it
 * The email is stored (it is a match key) but never shown in-game or logged.
 */
namespace KofiLedger {
    // Lower-case [a-z0-9-] only, 1..64 chars; "" when the input has nothing usable
    // or any other byte. Used for the file name and for the player's claim input.
    DLString sanitizeTxn(const DLString &raw);

    // Months of supporter time an amount buys: 12 per whole $50, plus 1 per whole $5
    // of the remainder. Parsed from the integer part of a decimal string ("5.00").
    int monthsFor(const DLString &amount);

    // Seconds in one supporter month (31 days).
    long long monthSeconds();

    bool load(const DLString &txn, Json::Value &entry);

    enum RecordResult { RECORD_OK, RECORD_DUPLICATE, RECORD_FAILED };
    // Store a webhook payment (fields already whitelisted, strings only), match it,
    // credit the account. `payment` must carry a non-empty sanitised "txn". On
    // RECORD_OK `entry` is the stored record. RECORD_FAILED = disk write failed.
    RecordResult record(const Json::Value &payment, Json::Value &entry);

    enum ClaimResult { CLAIM_OK, CLAIM_NOT_FOUND, CLAIM_TAKEN, CLAIM_FAILED };
    // Credit an UNMATCHED entry to accountId on a player's receipt claim.
    // CLAIM_NOT_FOUND = no such txn, CLAIM_TAKEN = not in the unmatched state.
    ClaimResult claim(const DLString &txn, const DLString &accountId,
                      const DLString &charName, Json::Value &entry);

    // The newest `limit` entries in the unmatched or manual state, newest first.
    std::list<Json::Value> recentOpen(int limit);
}

#endif
