/* See webblob.h. */
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>

#include "webblob.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "accountmanager.h"
#include "dreamland.h"
#include "dldirectory.h"
#include "dlfilestream.h"
#include "exceptiondbio.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

static const DLString BLOB_EXT = ".json";
static const DLString BLOB_TMP_EXT = ".json.tmp";
static const DLString BLOB_PREV_EXT = ".json.prev";

/** Decoded byte count of a base64 string with a valid alphabet and padding, or
 *  -1 when it is not base64 at all. */
static long web_blob_b64_size(const DLString &b64)
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

bool web_blob_caller_ok(Character *ch)
{
    if (ch == 0 || ch->getPC() == 0 || ch->desc == 0)
        return false;

    return ch->desc->connected == CON_PLAYING;
}

DLString web_blob_account(Character *ch)
{
    DLString accountId = AccountManager::accountOf(ch->getPC()->getName());

    // The id becomes a file name. Ids are minted from [A-Z2-9]; anything else
    // (a hand-edited pfile attribute) is treated as no account at all.
    for (DLString::size_type i = 0; i < accountId.size(); i++) {
        char c = accountId[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
            return DLString::emptyString;
    }

    if (!accountId.empty() && !AccountManager::exists(accountId))
        return DLString::emptyString;

    return accountId;
}

WebBlobTable::WebBlobTable(const DLString &table, size_t maxBytes, time_t putGap)
    : table(table), maxBytes(maxBytes), putGap(putGap)
{
}

bool WebBlobTable::load(const DLString &id, Json::Value &out) const
{
    DLDirectory dir(dreamland->getDbDir(), table);

    try {
        DLFile file(dir, id, BLOB_EXT);
        if (!file.exist())
            return false;

        // b64 of maxBytes plus a small JSON wrapper; anything bigger is not ours.
        if (file.getSize() > (long)(maxBytes / 3 * 4 + 1024)) {
            LogStream::sendError() << table << ": " << id << " stored file too large, ignored" << endl;
            return false;
        }

        std::ostringstream buf;
        DLFileStream(dir, id, BLOB_EXT).toStream(buf);

        Json::Value val;
        Json::Reader reader;
        if (!reader.parse(buf.str(), val) || !val.isObject()
            || !val["b64"].isString() || !val["stamp"].isNumeric())
        {
            LogStream::sendError() << table << ": " << id << " stored file is malformed, ignored" << endl;
            return false;
        }

        out = val;
        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << table << ": reading " << id << " failed: " << e.what() << endl;
        return false;
    }
}

/** Write {stamp, b64}: tmp file first, keep the previous copy as .prev, then
 *  rename the tmp file over the live one. */
bool WebBlobTable::store(const DLString &id, const DLString &b64, Json::Int64 stamp) const
{
    try {
        DLDirectory dir(dreamland->getDbDir(), table);
        if (!dir.exist())
            ::mkdir(dir.getCPath(), 0775);

        Json::Value val;
        val["stamp"] = stamp;
        val["b64"] = b64.c_str();

        Json::FastWriter writer;
        DLFileStream(dir, id, BLOB_TMP_EXT).fromString(writer.write(val));

        DLFile cur(dir, id, BLOB_EXT);
        DLFile tmp(dir, id, BLOB_TMP_EXT);
        DLFile prev(dir, id, BLOB_PREV_EXT);

        if (cur.exist() && !cur.copy(prev))
            LogStream::sendError() << table << ": " << id << " could not keep the previous copy" << endl;

        if (!tmp.rename(cur)) {
            LogStream::sendError() << table << ": " << id << " rename into place failed" << endl;
            return false;
        }

        return true;
    } catch (const ExceptionDBIO &e) {
        LogStream::sendError() << table << ": saving " << id << " failed: " << e.what() << endl;
        return false;
    }
}

void WebBlobTable::put(Character *ch, const DLString &id, const DLString &b64, Json::Value &reply)
{
    reply["ok"] = false;
    reply["stamp"] = 0;
    reply["reason"] = "";

    long size = -1;

    // The rate limit is keyed by the connection: no connection, no put.
    if (ch == 0 || ch->desc == 0) {
        reply["reason"] = "io";
        return;
    }

    if (b64.size() > (maxBytes + 2) / 3 * 4) {
        reply["reason"] = "too_big";
        return;
    }
    if ((size = web_blob_b64_size(b64)) < 0) {
        reply["reason"] = "bad_data";
        return;
    }
    if ((size_t)size > maxBytes) {
        reply["reason"] = "too_big";
        return;
    }

    time_t now = time(0);
    const DLString &conn = ch->desc->websock.nonce;

    // Forget connections that are past the gap, so the map stays small.
    for (std::map<DLString, time_t>::iterator i = putAt.begin(); i != putAt.end(); ) {
        if (now - i->second >= putGap)
            putAt.erase(i++);
        else
            ++i;
    }

    if (putAt.count(conn) > 0) {
        reply["reason"] = "rate";
        return;
    }
    putAt[conn] = now;

    // Stamps only ever go up, so a client comparing them never takes an older
    // copy for a newer one, even across a clock step.
    Json::Int64 stamp = (Json::Int64)now;
    Json::Value stored;
    if (load(id, stored) && stored["stamp"].asInt64() >= stamp)
        stamp = stored["stamp"].asInt64() + 1;

    if (!store(id, b64, stamp)) {
        reply["reason"] = "io";
        return;
    }

    reply["ok"] = true;
    reply["stamp"] = stamp;
    LogStream::sendNotice() << table << ": " << ch->getPC()->getName()
                            << " saved " << size << " bytes to " << id << endl;
}
