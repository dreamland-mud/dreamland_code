/* Dream Land, Ko-fi supporter ledger, 2026.
 */
#ifndef KOFILEDGER_H
#define KOFILEDGER_H

#include <list>
#include <set>
#include <jsoncpp/json/json.h>
#include "dlstring.h"

/**
 * One JSON file per Ko-fi transaction in db/kofi/<txn>.json, written atomically.
 *
 * The file is the idempotency key: a webhook retry for a transaction already on
 * disk changes nothing. Each entry keeps the whitelisted payment fields (as the
 * broker sent them), how it matched an account, the account id (or null), the
 * months granted and a status:
 *   pending   -- a credit is in flight; seen after a crash or a failed final
 *                write, it needs an immortal (the credit may or may not have landed)
 *   applied   -- credited to an account (months may be 0)
 *   unmatched -- Donation/Subscription with no account found; claimable in-game
 *   manual    -- Shop Order/Commission/unknown type, non-USD or malformed amount,
 *                or a credit that failed; an immortal handles it
 *   resolved  -- an immortal marked a manual/pending entry handled
 * The email is stored (it is a match key) but never shown in-game or logged.
 */
namespace KofiLedger {
    // Trust needed for `account kofi list/resolve/credit` and to see txn ids on
    // wiznet: whoever holds an unmatched txn id can claim the payment.
    int adminLevel();

    // Lower-case [a-z0-9-] only, 1..64 chars; "" when the input has nothing usable
    // or any other byte. Used for the file name and for the player's claim input.
    DLString sanitizeTxn(const DLString &raw);

    // Months of supporter time a USD amount buys: 12 per whole $50, plus 1 per whole
    // $5 of the remainder. Strict "D.CC" form (no leading zeros, no comma, at most
    // 7 integer digits); -1 when malformed.
    int monthsFor(const DLString &amount);

    // Seconds in one supporter month (31 days).
    long long monthSeconds();

    bool load(const DLString &txn, Json::Value &entry);

    // Ko-fi's "send a test" webhook uses txn ids of this form: never real money.
    bool isTestTxn(const DLString &txn);

    // Accounts a real payment ever credited: applied entries with months > 0,
    // test txns excluded. Feeds the boot backfill of the account "paid" flag.
    std::set<DLString> paidAccounts();

    enum RecordResult { RECORD_OK, RECORD_DUPLICATE, RECORD_FAILED };
    // Store a webhook payment (fields already whitelisted, strings only), match it,
    // credit the account. `payment` must carry a non-empty sanitised "txn". On
    // RECORD_OK `entry` is the stored record. RECORD_FAILED = nothing usable on
    // disk (write failed, or an existing file for this txn does not parse): the
    // caller answers 5xx so Ko-fi retries.
    RecordResult record(const Json::Value &payment, Json::Value &entry);

    enum ClaimResult { CLAIM_OK, CLAIM_NOT_FOUND, CLAIM_TAKEN, CLAIM_FAILED, CLAIM_BAD_AMOUNT };
    // Credit an UNMATCHED entry to accountId on a player's receipt claim.
    // CLAIM_NOT_FOUND = no such txn, CLAIM_TAKEN = not in the unmatched state.
    ClaimResult claim(const DLString &txn, const DLString &accountId,
                      const DLString &charName, Json::Value &entry);

    // Immortal: credit an unmatched, manual or pending entry to accountId.
    // months < 0 = from a USD amount (CLAIM_BAD_AMOUNT when it is malformed or not USD).
    ClaimResult credit(const DLString &txn, const DLString &accountId, int months,
                       const DLString &actor, Json::Value &entry);

    // Immortal: mark a manual or pending entry resolved (handled by hand).
    ClaimResult resolve(const DLString &txn, const DLString &actor, Json::Value &entry);

    // The newest `limit` entries in the unmatched, manual or pending state, newest first.
    std::list<Json::Value> recentOpen(int limit);
}

#endif
