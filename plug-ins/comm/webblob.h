/* Opaque per-owner blobs the web client keeps on the server: the gear script
 * (webscript.cpp), the player's triggers (webtriggers.cpp), macros
 * (webmacros.cpp) and account aliases (webaliases.cpp).
 *
 * Stored as base64 of the client's UTF-8 text, because every string in an rpc
 * frame goes through utf-8 -> koi8-u//IGNORE on the way in and would lose what
 * KOI8 has no slot for. The server checks the alphabet and the size only, never
 * decodes or runs the blob, and never logs it.
 *
 * One file per owner, db/<table>/<id>.json = {stamp, b64}, read on demand.
 * Never a field of a record kept in RAM: a broken blob must not take an account
 * or a character down with it. */
#ifndef __WEBBLOB_H__
#define __WEBBLOB_H__

#include <time.h>
#include <map>
#include <vector>
#include <jsoncpp/json/json.h>

#include "dlstring.h"

class Character;

class WebBlobTable {
public:
    /** maxBytes: decoded size limit. putGap: seconds between two accepted
     *  puts on one connection. */
    WebBlobTable(const DLString &table, size_t maxBytes, time_t putGap);

    /** Read the stored copy into {stamp, b64}. False when there is none or it
     *  cannot be read: a broken file reads as none. */
    bool load(const DLString &id, Json::Value &out) const;

    /** Validate and store b64 for id, filling a reply's ok/stamp/reason.
     *  reason: too_big | bad_data | rate | io. An empty b64 is a valid blob
     *  (the client sends it to clear the server copy). */
    void put(Character *ch, const DLString &id, const DLString &b64, Json::Value &reply);

private:
    bool store(const DLString &id, const DLString &b64, Json::Int64 stamp) const;

    DLString table;
    size_t maxBytes;
    time_t putGap;
    /** Last accepted put per connection, keyed by the websocket nonce. */
    std::map<DLString, time_t> putAt;
};

/** Only a character actually in the world may ask: in the nanny the name is
 *  typed by whoever is connecting, not authenticated. */
bool web_blob_caller_ok(Character *ch);

/** The caller's account id, "" for none or for an id that cannot be a file name. */
DLString web_blob_account(Character *ch);

/** The character's file id: its lowercase name, as the pfile is named. "" for
 *  a name that cannot be a file name. */
DLString web_blob_char_id(Character *ch);

/** A list the player keeps twice -- the character's own and the account's,
 *  shared by every character of the account -- picking per item which one.
 *  A character without an account has only its own.
 *
 *  get: <prefix>_data  {account, acct:{b64,stamp}, char:{b64,stamp}}
 *  put <acct|char> <b64> -> <prefix>_saved {scope, ok, stamp, reason}
 *       reason: no_account | bad_scope | too_big | bad_data | rate | io */
class WebBlobPair {
public:
    WebBlobPair(const DLString &prefix, const DLString &charTable, const DLString &acctTable,
                size_t maxBytes, time_t putGap);

    void get(Character *ch);
    void put(Character *ch, const std::vector<DLString> &args);

private:
    DLString prefix;
    WebBlobTable charTable;
    WebBlobTable acctTable;
};

#endif
