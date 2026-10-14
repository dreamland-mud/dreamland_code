/*
 * Server-side home of the web client's triggers (pattern -> highlight, sound,
 * commands), edited on the Triggers page of the settings window.
 *
 * Two lists, and the player picks per trigger which one it lives in: the
 * character's own, or the account's, shared by every character of the account.
 * A character without an account has only its own list.
 *
 * Each list is one opaque blob (see webblob.h): the server never parses the
 * triggers, the client does. Files: db/webtriggers_char/<name>.json and
 * db/webtriggers_acct/<accountId>.json.
 *
 * triggers_get                  -> triggers_data  {account, acct:{b64,stamp}, char:{b64,stamp}}
 * triggers_put <acct|char> <b64> -> triggers_saved {scope, ok, stamp, reason}
 *                                   reason: no_account | bad_scope | too_big | bad_data | rate | io
 * An empty <b64> clears that list.
 */
#include "webblob.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "rpccommandmanager.h"
#include "merc.h"
#include "def.h"

/** 64 KiB per list, decoded. One put per second per connection and list, so
 *  saving both lists after a move between them is not refused. */
static WebBlobTable triggers_char("webtriggers_char", 65536, 1);
static WebBlobTable triggers_acct("webtriggers_acct", 65536, 1);

/** The character's file id: its lowercase name, as the pfile is named. "" for a
 *  name that cannot be a file name. */
static DLString triggers_char_id(Character *ch)
{
    DLString id = ch->getPC()->getName().toLower();

    for (DLString::size_type i = 0; i < id.size(); i++) {
        char c = id[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
            return DLString::emptyString;
    }

    return id;
}

/** {b64, stamp} of one list, empty when there is none. */
static Json::Value triggers_list(WebBlobTable &table, const DLString &id)
{
    Json::Value list;
    list["b64"] = "";
    list["stamp"] = 0;

    Json::Value stored;
    if (!id.empty() && table.load(id, stored)) {
        list["b64"] = stored["b64"];
        list["stamp"] = stored["stamp"];
    }

    return list;
}

RPCRUN(triggers_get)
{
    if (!web_blob_caller_ok(ch))
        return;

    DLString acct = web_blob_account(ch);

    Json::Value msg;
    msg["command"] = "triggers_data";
    Json::Value &data = msg["args"][0];
    data["account"] = !acct.empty();
    data["acct"] = triggers_list(triggers_acct, acct);
    data["char"] = triggers_list(triggers_char, triggers_char_id(ch));

    ch->desc->writeWSCommand(msg);
}

RPCRUN(triggers_put)
{
    if (!web_blob_caller_ok(ch))
        return;

    const DLString &scope = args.size() > 0 ? args[0] : DLString::emptyString;
    const DLString &b64 = args.size() > 1 ? args[1] : DLString::emptyString;

    Json::Value msg;
    msg["command"] = "triggers_saved";
    Json::Value &data = msg["args"][0];
    data["scope"] = scope.c_str();
    data["ok"] = false;
    data["stamp"] = 0;
    data["reason"] = "";

    if (scope == "acct") {
        DLString acct = web_blob_account(ch);
        if (acct.empty())
            data["reason"] = "no_account";
        else
            triggers_acct.put(ch, acct, b64, data);
    } else if (scope == "char") {
        DLString id = triggers_char_id(ch);
        if (id.empty())
            data["reason"] = "bad_scope";
        else
            triggers_char.put(ch, id, b64, data);
    } else {
        data["reason"] = "bad_scope";
    }

    ch->desc->writeWSCommand(msg);
}
