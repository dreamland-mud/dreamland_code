/* Dream Land, Ko-fi supporter ledger, 2026.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "kofiledger.h"
#include "accountmanager.h"
#include "accountaudit.h"
#include "json_utils.h"
#include "wiznet.h"
#include "dreamland.h"
#include "dldirectory.h"
#include "dlfilestream.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

using namespace std;

static const DLString KOFI_TABLE = "kofi";
static const DLString KOFI_EXT = ".json";
static const int MAX_AMOUNT_DIGITS = 7;

int KofiLedger::adminLevel()
{
    return CREATOR;
}

// A string field or "" -- the ledger and the payload may be hand-edited, and a
// non-string asString() would throw into a player command.
static DLString kofi_str(const Json::Value &v, const char *key)
{
    if (!v.isObject())
        return DLString::emptyString;
    Json::Value f = v.get(key, Json::Value());
    return f.isString() ? DLString(f.asString()) : DLString::emptyString;
}

static DLDirectory kofi_dir()
{
    DLDirectory dir(dreamland->getDbDir(), KOFI_TABLE);
    // The writer does not mkdir, so a fresh host would fail every first write.
    if (!dir.exist())
        ::mkdir(dir.getCPath(), 0775);
    return dir;
}

static DLFile kofi_file(const DLString &txn)
{
    return DLFile(DLDirectory(dreamland->getDbDir(), KOFI_TABLE), txn, KOFI_EXT);
}

static bool kofi_save(const DLString &txn, const Json::Value &entry)
{
    try {
        DLDirectory dir = kofi_dir();
        if (AccountManager::writeAtomic(DLFile(dir, txn, KOFI_EXT).getPath(), JsonUtils::toString(entry)))
            return true;
    } catch (const std::exception &e) {
        LogStream::sendError() << "Ko-fi: saving " << txn << ": " << e.what() << endl;
    }
    LogStream::sendError() << "Ko-fi: saving " << txn << " failed." << endl;
    return false;
}

DLString KofiLedger::sanitizeTxn(const DLString &raw)
{
    DLString out;
    for (string::size_type i = 0; i < raw.size(); i++) {
        char c = raw[i];
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')
            out += c;
        else
            return DLString::emptyString;
    }
    if (out.size() > 64)
        return DLString::emptyString;
    return out;
}

int KofiLedger::monthsFor(const DLString &amount)
{
    // "5", "5.0", "5.00", "1000.00"; not "05.00", "5,00", "1,000.00", "5.", ".5".
    string::size_type dot = amount.find('.');
    string whole = amount.substr(0, dot);
    if (whole.empty() || (int)whole.size() > MAX_AMOUNT_DIGITS)
        return -1;
    if (whole.size() > 1 && whole[0] == '0')
        return -1;
    for (char c: whole)
        if (c < '0' || c > '9')
            return -1;
    if (dot != string::npos) {
        string frac = amount.substr(dot + 1);
        if (frac.empty() || frac.size() > 2)
            return -1;
        for (char c: frac)
            if (c < '0' || c > '9')
                return -1;
    }

    long long dollars = atoll(whole.c_str());
    return (int)(12 * (dollars / 50) + (dollars % 50) / 5);
}

long long KofiLedger::monthSeconds()
{
    return 31LL * 24 * 60 * 60;
}

bool KofiLedger::load(const DLString &txn, Json::Value &entry)
{
    if (txn.empty())
        return false;
    try {
        DLFile file = kofi_file(txn);
        if (!file.exist())
            return false;
        ostringstream buf;
        DLFileStream(file).toStream(buf);
        JsonUtils::fromString(buf.str(), entry);
        return entry.isObject() && !kofi_str(entry, "status").empty();
    } catch (const std::exception &e) {
        LogStream::sendError() << "Ko-fi: reading " << txn << " failed: " << e.what() << endl;
        return false;
    }
}

// Every parseable ledger entry. The ledger holds one file per payment, so a full
// read stays small; callers are the immortal list and the one-time email index.
static vector<Json::Value> kofi_scan()
{
    vector<Json::Value> all;
    DLDirectory dir(dreamland->getDbDir(), KOFI_TABLE);

    try {
        dir.open();
    } catch (const ExceptionDBIO &) {
        return all;
    }

    try {
        for (;;) {
            DLFile file = dir.nextTypedEntry(KOFI_EXT);
            try {
                ostringstream buf;
                DLFileStream(dir, file).toStream(buf);
                Json::Value entry;
                JsonUtils::fromString(buf.str(), entry);
                if (entry.isObject())
                    all.push_back(entry);
            } catch (const std::exception &e) {
                LogStream::sendError() << "Ko-fi: skipping " << file.getFileName()
                    << ": " << e.what() << endl;
            }
        }
    } catch (const ExceptionDBIOEOF &) {
        // end of directory
    }
    dir.close();
    return all;
}

static long long kofi_received(const Json::Value &e)
{
    Json::Value r = e.get("received", Json::Value(0));
    return r.isIntegral() ? r.asInt64() : 0;
}

static bool kofi_older(const Json::Value &a, const Json::Value &b)
{
    return kofi_received(a) < kofi_received(b);
}

// Payer email -> account, learnt from applied subscription entries. Ko-fi renewals
// arrive without the message the first payment carried, so a subscriber matched
// once by code or claim keeps matching by the email Ko-fi sends with every payment.
// One-off payments teach nothing: a one-off gift carries the friend's code, and
// learning from it would send the giver's own later payments to the friend.
// Built from the ledger on first use, then kept current by kofi_remember_email.
static map<DLString, DLString> kofiEmails;
static bool kofiEmailsLoaded = false;

static DLString kofi_email(const Json::Value &entry)
{
    DLString email = kofi_str(entry["payment"], "email");
    email.toLower();
    return email;
}

static bool kofi_is_subscription(const Json::Value &payment)
{
    Json::Value sub = payment.get("is_subscription_payment", Json::Value());
    return (sub.isBool() && sub.asBool()) || kofi_str(payment, "type") == "Subscription";
}

static void kofi_remember_email(const Json::Value &entry)
{
    if (!kofi_is_subscription(entry["payment"]))
        return;
    DLString email = kofi_email(entry);
    DLString id = kofi_str(entry, "account");
    if (!email.empty() && !id.empty() && kofi_str(entry, "status") == "applied")
        kofiEmails[email] = id;
}

static void kofi_load_emails()
{
    if (kofiEmailsLoaded)
        return;
    kofiEmailsLoaded = true;

    vector<Json::Value> all = kofi_scan();
    sort(all.begin(), all.end(), kofi_older);   // the newest payment wins
    for (const Json::Value &e: all)
        kofi_remember_email(e);
}

// The account's Ko-fi code (its 6-char id, shown in `account` status) anywhere in
// the payer's message, any case. Nothing else in free text matches: a payment
// credited to a stranger is worse than an unmatched one, which the receipt claim
// recovers.
static DLString kofi_match_message(const DLString &msg)
{
    DLString cur;
    for (string::size_type i = 0; i <= msg.size(); i++) {
        char c = i < msg.size() ? msg[i] : ' ';
        bool alnum = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (alnum) {
            cur += c;
            continue;
        }
        if (cur.size() == 6) {
            DLString id = cur.toUpper();
            if (AccountManager::exists(id))
                return id;
        }
        cur.clear();
    }
    return DLString::emptyString;
}

// A code in the message wins over the payer's own identities: it is the payer
// saying which account the money is for, which is how a gift reaches a friend.
static DLString kofi_match(const Json::Value &p, DLString &how)
{
    DLString coded = kofi_match_message(kofi_str(p, "message"));
    if (!coded.empty()) {
        how = "message_code";
        return coded;
    }

    DLString discord = kofi_str(p, "discord_userid");
    if (!discord.empty()) {
        DLString id = AccountManager::findByIdentity("discord", discord);
        if (!id.empty()) {
            how = "discord";
            return id;
        }
    }

    DLString email = kofi_str(p, "email");
    email.toLower();
    if (!email.empty()) {
        DLString id = AccountManager::findByIdentity("email", email);
        if (!id.empty()) {
            how = "email";
            return id;
        }

        kofi_load_emails();
        map<DLString, DLString>::iterator known = kofiEmails.find(email);
        if (known != kofiEmails.end() && AccountManager::exists(known->second)) {
            how = "kofi_email";
            return known->second;
        }
    }

    return DLString::emptyString;
}

// One wiznet line for admins who can act on it (wiznet also logs it as a notice).
// It carries the txn id, which is enough to claim an unmatched payment, so it goes
// only to adminLevel(). Never the email.
static void kofi_notify(const DLString &txn, const Json::Value &entry, const char *what)
{
    const Json::Value &p = entry["payment"];
    wiznet(WIZ_SECURE, 0, KofiLedger::adminLevel(),
           "Ko-fi: %s: %s %s %s %s from '%s', txn %s ('account kofi list').",
           what,
           kofi_str(entry, "status").c_str(),
           kofi_str(p, "type").c_str(),
           kofi_str(p, "amount").c_str(),
           kofi_str(p, "currency").c_str(),
           AccountManager::echoSafe(kofi_str(p, "from_name")).c_str(),
           txn.c_str());
}

static void kofi_audit(const char *event, const DLString &txn, const Json::Value &entry)
{
    Json::Value a;
    a["txn"] = txn.c_str();
    a["status"] = entry["status"];
    a["match"] = entry["match"];
    a["account"] = entry["account"];
    a["months"] = entry["months"];
    a["type"] = entry["payment"]["type"];
    a["amount"] = entry["payment"]["amount"];
    a["currency"] = entry["payment"]["currency"];
    AccountAudit::record(event, a);
}

enum ApplyResult {
    APPLY_OK,           // credited (or 0 months) and recorded applied
    APPLY_NOT_SAVED,    // nothing written, nothing credited
    APPLY_CREDIT_FAILED,// not credited; entry is onFail on disk (or pending if that write failed)
    APPLY_STUCK         // credited, but the entry stayed pending on disk (alerted)
};

// pending -> credit -> applied, so no crash or failed write can leave an entry
// that says applied without the credit behind it. `entry` already carries the
// match fields; on a failed credit it becomes `onFail` (written back best effort,
// else it stays pending, which the immortal list shows).
static ApplyResult kofi_apply(const DLString &txn, Json::Value &entry, const DLString &id,
                              int months, const Json::Value &onFail)
{
    entry["account"] = id.c_str();
    entry["months"] = months;

    if (months > 0) {
        entry["status"] = "pending";
        if (!kofi_save(txn, entry))
            return APPLY_NOT_SAVED;
        if (!AccountManager::extendSupporter(id, months * KofiLedger::monthSeconds())) {
            entry = onFail;
            if (!kofi_save(txn, entry))
                entry["status"] = "pending";
            return APPLY_CREDIT_FAILED;
        }
    }

    entry["status"] = "applied";
    if (!kofi_save(txn, entry)) {
        if (months == 0)
            return APPLY_NOT_SAVED;
        entry["status"] = "pending";
        kofi_notify(txn, entry, "credited but the ledger kept it pending");
        return APPLY_STUCK;
    }
    kofi_remember_email(entry);
    return APPLY_OK;
}

KofiLedger::RecordResult KofiLedger::record(const Json::Value &payment, Json::Value &entry)
{
    DLString txn = sanitizeTxn(kofi_str(payment, "txn"));
    if (txn.empty())
        return RECORD_FAILED;

    if (kofi_file(txn).exist()) {
        Json::Value old;
        if (load(txn, old))
            return RECORD_DUPLICATE;
        // A file that does not parse is not proof the payment was handled: answer
        // 5xx so Ko-fi retries, and get an immortal to look at the file.
        entry = Json::Value(Json::objectValue);
        entry["payment"] = payment;
        entry["status"] = "unreadable";
        kofi_notify(txn, entry, "ledger file exists but does not parse");
        return RECORD_FAILED;
    }

    DLString type = kofi_str(payment, "type");
    DLString currency = kofi_str(payment, "currency");
    currency.toUpper();
    int months = monthsFor(kofi_str(payment, "amount"));

    DLString note;
    // Ko-fi sends "Tip" for one-off payments; "Donation" is the older name.
    if (type != "Tip" && type != "Donation" && type != "Subscription")
        note = "type";
    else if (currency != "USD")
        note = "currency";
    else if (months < 0)
        note = "amount";

    DLString how;
    DLString id = kofi_match(payment, how);

    entry = Json::Value(Json::objectValue);
    entry["txn"] = txn.c_str();
    entry["payment"] = payment;
    entry["received"] = (Json::Int64)dreamland->getCurrentTime();
    entry["match"] = how.empty() ? "none" : how.c_str();
    entry["account"] = id.empty() ? Json::Value(Json::nullValue) : Json::Value(id.c_str());
    entry["months"] = 0;

    if (!note.empty() || id.empty()) {
        if (!note.empty()) {
            entry["status"] = "manual";
            entry["note"] = note.c_str();
        } else {
            entry["status"] = "unmatched";
        }
        if (!kofi_save(txn, entry))
            return RECORD_FAILED;
    } else {
        Json::Value onFail = entry;
        onFail["status"] = "manual";
        onFail["note"] = "credit_failed";
        if (kofi_apply(txn, entry, id, months, onFail) == APPLY_NOT_SAVED)
            return RECORD_FAILED;
    }

    DLString status = kofi_str(entry, "status");
    kofi_audit("kofi", txn, entry);
    LogStream::sendNotice() << "Ko-fi: txn " << txn << " " << type << " "
        << kofi_str(payment, "amount") << " " << currency
        << " -> " << status << " (" << kofi_str(entry, "match")
        << " " << (id.empty() ? "-" : id.c_str()) << ")" << endl;

    if (status != "applied")
        kofi_notify(txn, entry, "needs an immortal");
    return RECORD_OK;
}

// Shared tail of claim() and credit(): credit an open entry to accountId, restore
// the entry as it was when the credit fails.
static KofiLedger::ClaimResult kofi_credit_entry(const DLString &txn, Json::Value &entry,
                                                 const DLString &accountId, int months,
                                                 const char *match, const char *byKey,
                                                 const DLString &by, const char *event)
{
    Json::Value before = entry;
    entry["match"] = match;
    entry[byKey] = by.c_str();
    entry["credited"] = (Json::Int64)dreamland->getCurrentTime();
    entry.removeMember("note");

    switch (kofi_apply(txn, entry, accountId, months, before)) {
    case APPLY_NOT_SAVED:
        entry = before;
        return KofiLedger::CLAIM_FAILED;
    case APPLY_CREDIT_FAILED:
        return KofiLedger::CLAIM_FAILED;
    default:
        break;
    }

    kofi_audit(event, txn, entry);
    LogStream::sendNotice() << "Ko-fi: txn " << txn << " credited by " << by
        << " to " << accountId << ", " << months << " months" << endl;
    return KofiLedger::CLAIM_OK;
}

KofiLedger::ClaimResult KofiLedger::claim(const DLString &txn, const DLString &accountId,
                                          const DLString &charName, Json::Value &entry)
{
    if (!load(txn, entry))
        return CLAIM_NOT_FOUND;
    if (kofi_str(entry, "status") != "unmatched")
        return CLAIM_TAKEN;

    int months = monthsFor(kofi_str(entry["payment"], "amount"));
    if (months < 0)
        return CLAIM_BAD_AMOUNT;   // record() never leaves a malformed amount unmatched
    return kofi_credit_entry(txn, entry, accountId, months, "claim", "claimedBy", charName, "kofi_claim");
}

KofiLedger::ClaimResult KofiLedger::credit(const DLString &txn, const DLString &accountId, int months,
                                           const DLString &actor, Json::Value &entry)
{
    if (!load(txn, entry))
        return CLAIM_NOT_FOUND;
    DLString status = kofi_str(entry, "status");
    if (status != "unmatched" && status != "manual" && status != "pending")
        return CLAIM_TAKEN;

    if (months < 0) {
        // Derive months only from a USD amount; anything else needs them spelled out.
        DLString currency = kofi_str(entry["payment"], "currency");
        currency.toUpper();
        if (currency != "USD")
            return CLAIM_BAD_AMOUNT;
        months = monthsFor(kofi_str(entry["payment"], "amount"));
    }
    if (months < 0)
        return CLAIM_BAD_AMOUNT;
    return kofi_credit_entry(txn, entry, accountId, months, "immortal", "creditedBy", actor, "kofi_credit");
}

KofiLedger::ClaimResult KofiLedger::resolve(const DLString &txn, const DLString &actor, Json::Value &entry)
{
    if (!load(txn, entry))
        return CLAIM_NOT_FOUND;
    DLString status = kofi_str(entry, "status");
    if (status != "manual" && status != "pending")
        return CLAIM_TAKEN;

    Json::Value before = entry;
    entry["status"] = "resolved";
    entry["resolvedFrom"] = status.c_str();
    entry["resolvedBy"] = actor.c_str();
    entry["resolved"] = (Json::Int64)dreamland->getCurrentTime();
    if (!kofi_save(txn, entry)) {
        entry = before;
        return CLAIM_FAILED;
    }
    kofi_audit("kofi_resolve", txn, entry);
    return CLAIM_OK;
}

list<Json::Value> KofiLedger::recentOpen(int limit)
{
    vector<Json::Value> open;
    for (const Json::Value &e: kofi_scan()) {
        DLString status = kofi_str(e, "status");
        if (status == "unmatched" || status == "manual" || status == "pending")
            open.push_back(e);
    }

    sort(open.begin(), open.end(), kofi_older);
    reverse(open.begin(), open.end());
    if ((int)open.size() > limit)
        open.resize(limit);
    return list<Json::Value>(open.begin(), open.end());
}
