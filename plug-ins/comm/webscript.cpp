/*
 * Server-side backup of the web client's "gear" script (the player's own JS
 * triggers and key bindings, edited in the settings window).
 *
 * Kept per ACCOUNT, never per character: the script belongs to the person, and
 * a character without an account gets no backup (the client says so).
 *
 * Stored opaque, as base64 of the UTF-8 text produced by the client. Every
 * string in an rpc frame goes through utf-8 -> koi8-u//IGNORE on the way in
 * (iomanager/descriptor.cpp), which would silently eat any character KOI8 has
 * no slot for; base64 is plain ASCII and rides through untouched. The server
 * only checks the alphabet and the size, never decodes or runs it, and never
 * logs it.
 *
 * One file per account, db/webscript/<accountId>.json = {stamp, b64}, read on
 * demand (no RAM cache). Deliberately NOT a field of the account record: the
 * AccountManager keeps every account in RAM and its load() skips a malformed
 * file, so a broken script must never be able to take an account down with it.
 *
 * script_get        -> script_data  {account, b64, stamp}
 * script_put <b64>  -> script_saved {ok, stamp, reason}
 *                      reason: no_account | too_big | bad_data | rate | io
 * An empty <b64> clears the server copy (the client sends it when the script is
 * back to the stock defaults).
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <map>
#include <sstream>
#include <jsoncpp/json/json.h>

#include "pcharacter.h"
#include "descriptor.h"
#include "rpccommandmanager.h"
#include "accountmanager.h"
#include "dreamland.h"
#include "dldirectory.h"
#include "dlfilestream.h"
#include "exceptiondbio.h"
#include "json_utils.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

static const DLString WEBSCRIPT_TABLE = "webscript";
static const DLString WEBSCRIPT_EXT = ".json";
static const DLString WEBSCRIPT_TMP_EXT = ".json.tmp";
static const DLString WEBSCRIPT_PREV_EXT = ".json.prev";

/** 64 KiB of script text, decoded. */
static const size_t WEBSCRIPT_MAX_BYTES = 65536;
/** Base64 length of WEBSCRIPT_MAX_BYTES: 4 * ceil(65536 / 3). */
static const size_t WEBSCRIPT_MAX_B64 = 87384;
/** A stored file larger than this is not ours (b64 plus a small JSON wrapper). */
static const int WEBSCRIPT_MAX_FILE = 128 * 1024;
/** Seconds between two accepted puts on one connection. */
static const time_t WEBSCRIPT_PUT_GAP = 3;

/** Last accepted put per connection, keyed by the websocket nonce (a fresh
 *  random id per connection, unlike a Descriptor pointer that can be reused). */
static std::map<DLString, time_t> webscript_put_at;

/** Only a character actually in the world may ask: in the nanny the name is
 *  typed by whoever is connecting, not authenticated (same gate as the
 *  account_chars rpc). Returns the account id, "" for none. */
static bool webscript_caller(Character *ch, DLString &accountId)
{
    if (ch == 0 || ch->getPC() == 0 || ch->desc == 0)
        return false;

    if (ch->desc->connected != CON_PLAYING)
        return false;

    accountId = AccountManager::accountOf(ch->getPC()->getName());

    // The id becomes a file name. Ids are minted from [A-Z2-9]; anything else
    // (a hand-edited pfile attribute) is treated as no account at all.
    for (DLString::size_type i = 0; i < accountId.size(); i++) {
        char c = accountId[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            accountId.clear();
            break;
        }
    }

    if (!accountId.empty() && !AccountManager::exists(accountId))
        accountId.clear();

    return true;
}

/** Decoded byte count of a base64 string with a valid alphabet and padding, or
 *  -1 when it is not base64 at all. */
static long webscript_b64_size(const DLString &b64)
{
    size_t n = b64.size();
    if (n % 4 != 0)
        return -1;

    size_t pad = 0;
    for (size_t i = 0; i < n; i++) {
        char c = b64[i];
        bool alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                  || (c >= '0' && c <= '9') || c == '+' || c == '/';
        if (alpha) {
            if (pad > 0)
                return -1;      // data after padding
            continue;
        }
        if (c == '=' && i >= n - 2) {
            pad++;
            continue;
        }
        return -1;
    }

    return (long)(n / 4 * 3 - pad);
}

static DLDirectory webscript_dir()
{
    return DLDirectory(dreamland->getDbDir(), WEBSCRIPT_TABLE);
}

/** Read the stored copy. False when there is none or it cannot be read; a
 *  broken file reads as none rather than taking the rpc down. */
static bool webscript_load(const DLString &id, Json::Value &out)
{
    try {
        DLFile file(webscript_dir(), id, WEBSCRIPT_EXT);
        if (!file.exist())
            return false;

        if (file.getSize() > WEBSCRIPT_MAX_FILE) {
            LogStream::sendError() << "webscript: " << id << " stored file too large, ignored" << endl;
            return false;
        }

        std::ostringstream buf;
        DLFileStream(webscript_dir(), id, WEBSCRIPT_EXT).toStream(buf);

        Json::Value val;
        Json::Reader reader;
        if (!reader.parse(buf.str(), val) || !val.isObject()
            || !val["b64"].isString() || !val["stamp"].isNumeric())
        {
            LogStream::sendError() << "webscript: " << id << " stored file is malformed, ignored" << endl;
            return false;
        }

        out = val;
        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << "webscript: reading " << id << " failed: " << e.what() << endl;
        return false;
    }
}

/** Write {stamp, b64}: tmp file first, keep the previous copy as .prev, then
 *  rename the tmp file over the live one. */
static bool webscript_store(const DLString &id, const DLString &b64, Json::Int64 stamp)
{
    try {
        DLDirectory dir = webscript_dir();
        if (!dir.exist())
            ::mkdir(dir.getCPath(), 0775);

        Json::Value val;
        val["stamp"] = stamp;
        val["b64"] = b64.c_str();

        Json::FastWriter writer;
        DLFileStream(dir, id, WEBSCRIPT_TMP_EXT).fromString(writer.write(val));

        DLFile cur(dir, id, WEBSCRIPT_EXT);
        DLFile tmp(dir, id, WEBSCRIPT_TMP_EXT);
        DLFile prev(dir, id, WEBSCRIPT_PREV_EXT);

        if (cur.exist() && !cur.copy(prev))
            LogStream::sendError() << "webscript: " << id << " could not keep the previous copy" << endl;

        if (!tmp.rename(cur)) {
            LogStream::sendError() << "webscript: " << id << " rename into place failed" << endl;
            return false;
        }

        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << "webscript: saving " << id << " failed: " << e.what() << endl;
        return false;
    }
}

RPCRUN(script_get)
{
    DLString id;
    if (!webscript_caller(ch, id))
        return;

    Json::Value msg;
    msg["command"] = "script_data";
    Json::Value &data = msg["args"][0];
    data["account"] = !id.empty();
    data["b64"] = "";
    data["stamp"] = 0;

    Json::Value stored;
    if (!id.empty() && webscript_load(id, stored)) {
        data["b64"] = stored["b64"];
        data["stamp"] = stored["stamp"];
    }

    ch->desc->writeWSCommand(msg);
}

RPCRUN(script_put)
{
    DLString id;
    if (!webscript_caller(ch, id))
        return;

    Json::Value msg;
    msg["command"] = "script_saved";
    Json::Value &data = msg["args"][0];
    data["ok"] = false;
    data["stamp"] = 0;
    data["reason"] = "";

    const DLString &b64 = args.empty() ? DLString::emptyString : args[0];
    long size = -1;

    if (id.empty()) {
        data["reason"] = "no_account";
    } else if (b64.size() > WEBSCRIPT_MAX_B64) {
        data["reason"] = "too_big";
    } else if ((size = webscript_b64_size(b64)) < 0) {
        data["reason"] = "bad_data";
    } else if ((size_t)size > WEBSCRIPT_MAX_BYTES) {
        data["reason"] = "too_big";
    } else {
        time_t now = time(0);
        const DLString &conn = ch->desc->websock.nonce;

        // Forget connections that are past the gap, so the map stays small.
        for (std::map<DLString, time_t>::iterator i = webscript_put_at.begin(); i != webscript_put_at.end(); ) {
            if (now - i->second >= WEBSCRIPT_PUT_GAP)
                webscript_put_at.erase(i++);
            else
                ++i;
        }

        if (webscript_put_at.count(conn) > 0) {
            data["reason"] = "rate";
        } else {
            webscript_put_at[conn] = now;

            // Stamps only ever go up, so a client comparing them never takes an
            // older copy for a newer one, even across a clock step.
            Json::Int64 stamp = (Json::Int64)now;
            Json::Value stored;
            if (webscript_load(id, stored) && stored["stamp"].asInt64() >= stamp)
                stamp = stored["stamp"].asInt64() + 1;

            if (webscript_store(id, b64, stamp)) {
                data["ok"] = true;
                data["stamp"] = stamp;
                LogStream::sendNotice() << "webscript: " << ch->getPC()->getName()
                                        << " saved " << size << " bytes to account " << id << endl;
            } else {
                data["reason"] = "io";
            }
        }
    }

    ch->desc->writeWSCommand(msg);
}
