/* Dream Land, Ko-fi supporter ledger, 2026.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <vector>
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
static const int MAX_MONTHS = 1200;

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
    // DLFileStream does not mkdir, so a fresh host would fail every first write.
    if (!dir.exist())
        ::mkdir(dir.getCPath(), 0775);
    return dir;
}

static bool kofi_save(const DLString &txn, const Json::Value &entry)
{
    try {
        DLFileStream(kofi_dir(), txn, KOFI_EXT).fromString(JsonUtils::toString(entry));
        return true;
    } catch (const std::exception &e) {
        LogStream::sendError() << "Ko-fi: saving " << txn << " failed: " << e.what() << endl;
        return false;
    }
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
    long long dollars = 0;
    int digits = 0;
    for (string::size_type i = 0; i < amount.size(); i++) {
        char c = amount[i];
        if (c == ',' || c == ' ')
            continue;   // "1,000.00"
        if (c < '0' || c > '9')
            break;      // the decimal point or junk ends the integer part
        if (++digits > 9)
            return MAX_MONTHS;
        dollars = dollars * 10 + (c - '0');
    }
    long long months = 12 * (dollars / 50) + (dollars % 50) / 5;
    return (int)min(months, (long long)MAX_MONTHS);
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
        DLFile file(DLDirectory(dreamland->getDbDir(), KOFI_TABLE), txn, KOFI_EXT);
        if (!file.exist())
            return false;
        ostringstream buf;
        DLFileStream(file).toStream(buf);
        JsonUtils::fromString(buf.str(), entry);
        return entry.isObject();
    } catch (const std::exception &e) {
        LogStream::sendError() << "Ko-fi: reading " << txn << " failed: " << e.what() << endl;
        return false;
    }
}

// Account named in the payer's Ko-fi message. The account id (the "Ko-fi code" from
// `account` status) matches anywhere in the text, any case. A character name matches
// only when the whole message, punctuation dropped and trimmed, is that one Latin
// word: a stranger's credit is worse than an unmatched payment, which the receipt
// claim recovers. Latin only, so PCharacterManager's Cyrillic declension matching
// never runs on free text.
static DLString kofi_match_message(const DLString &msg, DLString &how)
{
    vector<DLString> words;
    DLString cur, bare;
    for (string::size_type i = 0; i <= msg.size(); i++) {
        char c = i < msg.size() ? msg[i] : ' ';
        bool alnum = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        bool space = (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        // bare: the message without punctuation; non-ASCII bytes are letters here.
        if (alnum || space || (unsigned char)c >= 0x80)
            bare += c;
        if (alnum) {
            cur += c;
        } else if (!cur.empty()) {
            words.push_back(cur);
            cur.clear();
        }
    }

    for (const DLString &w: words) {
        if (w.size() != 6)
            continue;
        DLString id = w.toUpper();
        if (AccountManager::exists(id)) {
            how = "message_account";
            return id;
        }
    }

    string::size_type first = bare.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return DLString::emptyString;
    DLString name = bare.substr(first, bare.find_last_not_of(" \t\r\n") - first + 1);
    if (name.size() < 2 || name.size() > 12)
        return DLString::emptyString;
    for (string::size_type i = 0; i < name.size(); i++)
        if (!((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z')))
            return DLString::emptyString;

    DLString id = AccountManager::accountOf(name.toLower());
    if (!id.empty() && AccountManager::exists(id)) {
        how = "message_char";
        return id;
    }
    return DLString::emptyString;
}

static DLString kofi_match(const Json::Value &p, DLString &how)
{
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
    }

    return kofi_match_message(kofi_str(p, "message"), how);
}

// One wiznet line (wiznet also writes it to the log as a notice). Never the email.
static void kofi_notify(const DLString &txn, const Json::Value &entry)
{
    const Json::Value &p = entry["payment"];
    wiznet(WIZ_SECURE, 0, 0, "Ko-fi: %s %s %s %s from '%s', txn %s -- needs an immortal ('account kofi list').",
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

KofiLedger::RecordResult KofiLedger::record(const Json::Value &payment, Json::Value &entry)
{
    DLString txn = sanitizeTxn(kofi_str(payment, "txn"));
    if (txn.empty())
        return RECORD_FAILED;

    if (DLFile(DLDirectory(dreamland->getDbDir(), KOFI_TABLE), txn, KOFI_EXT).exist())
        return RECORD_DUPLICATE;

    DLString type = kofi_str(payment, "type");
    bool credit = (type == "Donation" || type == "Subscription");

    DLString how;
    DLString id = kofi_match(payment, how);

    entry = Json::Value(Json::objectValue);
    entry["txn"] = txn.c_str();
    entry["payment"] = payment;
    entry["received"] = (Json::Int64)dreamland->getCurrentTime();
    entry["match"] = how.empty() ? "none" : how.c_str();
    entry["account"] = id.empty() ? Json::Value(Json::nullValue) : Json::Value(id.c_str());

    int months = 0;
    if (!credit)
        entry["status"] = "manual";
    else if (id.empty())
        entry["status"] = "unmatched";
    else {
        entry["status"] = "applied";
        months = monthsFor(kofi_str(payment, "amount"));
    }
    entry["months"] = months;

    // The ledger file goes to disk BEFORE the credit: a crash or a failed account
    // write can then lose a credit (flagged manual below), never grant it twice on
    // a Ko-fi retry.
    if (!kofi_save(txn, entry))
        return RECORD_FAILED;

    if (months > 0 && !AccountManager::extendSupporter(id, months * monthSeconds())) {
        entry["status"] = "manual";
        entry["note"] = "credit_failed";
        kofi_save(txn, entry);
    }

    kofi_audit("kofi", txn, entry);
    LogStream::sendNotice() << "Ko-fi: txn " << txn << " " << type << " "
        << kofi_str(payment, "amount") << " " << kofi_str(payment, "currency")
        << " -> " << kofi_str(entry, "status") << " (" << kofi_str(entry, "match")
        << " " << (id.empty() ? "-" : id.c_str()) << ", " << months << " months)" << endl;

    if (kofi_str(entry, "status") != "applied")
        kofi_notify(txn, entry);
    return RECORD_OK;
}

KofiLedger::ClaimResult KofiLedger::claim(const DLString &txn, const DLString &accountId,
                                          const DLString &charName, Json::Value &entry)
{
    if (!load(txn, entry))
        return CLAIM_NOT_FOUND;
    if (kofi_str(entry, "status") != "unmatched")
        return CLAIM_TAKEN;

    Json::Value before = entry;
    int months = monthsFor(kofi_str(entry["payment"], "amount"));

    entry["status"] = "applied";
    entry["match"] = "claim";
    entry["account"] = accountId.c_str();
    entry["claimedBy"] = charName.c_str();
    entry["claimed"] = (Json::Int64)dreamland->getCurrentTime();
    entry["months"] = months;

    // Same order as record(): the entry leaves the unmatched state on disk first,
    // so a second claim cannot credit it again; undo it if the credit fails.
    if (!kofi_save(txn, entry)) {
        entry = before;
        return CLAIM_FAILED;
    }
    if (months > 0 && !AccountManager::extendSupporter(accountId, months * monthSeconds())) {
        kofi_save(txn, before);
        entry = before;
        return CLAIM_FAILED;
    }

    kofi_audit("kofi_claim", txn, entry);
    LogStream::sendNotice() << "Ko-fi: txn " << txn << " claimed by " << charName
        << " for " << accountId << ", " << months << " months" << endl;
    return CLAIM_OK;
}

static bool kofi_newer(const Json::Value &a, const Json::Value &b)
{
    Json::Value ra = a.get("received", Json::Value(0));
    Json::Value rb = b.get("received", Json::Value(0));
    long long ta = ra.isIntegral() ? ra.asInt64() : 0;
    long long tb = rb.isIntegral() ? rb.asInt64() : 0;
    return ta > tb;
}

list<Json::Value> KofiLedger::recentOpen(int limit)
{
    vector<Json::Value> open;
    DLDirectory dir(dreamland->getDbDir(), KOFI_TABLE);

    try {
        dir.open();
    } catch (const ExceptionDBIO &) {
        return list<Json::Value>();
    }

    try {
        for (;;) {
            DLFile file = dir.nextTypedEntry(KOFI_EXT);
            try {
                ostringstream buf;
                DLFileStream(dir, file).toStream(buf);
                Json::Value entry;
                JsonUtils::fromString(buf.str(), entry);
                DLString status = kofi_str(entry, "status");
                if (status == "unmatched" || status == "manual")
                    open.push_back(entry);
            } catch (const std::exception &e) {
                LogStream::sendError() << "Ko-fi: skipping " << file.getFileName()
                    << ": " << e.what() << endl;
            }
        }
    } catch (const ExceptionDBIOEOF &) {
        // end of directory
    }
    dir.close();

    sort(open.begin(), open.end(), kofi_newer);
    if ((int)open.size() > limit)
        open.resize(limit);
    return list<Json::Value>(open.begin(), open.end());
}
