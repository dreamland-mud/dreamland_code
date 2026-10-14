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
 * One file per account, db/webscript/<accountId>.json = {stamp, b64}; the file
 * handling is shared with the triggers in webblob.cpp.
 *
 * script_get        -> script_data  {account, b64, stamp}
 * script_put <b64>  -> script_saved {ok, stamp, reason}
 *                      reason: no_account | too_big | bad_data | rate | io
 * An empty <b64> clears the server copy (the client sends it when the script is
 * back to the stock defaults).
 */
#include "webblob.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "rpccommandmanager.h"
#include "merc.h"
#include "def.h"

/** 64 KiB of script text, decoded; 3 seconds between two puts on one connection. */
static WebBlobTable webscript_table("webscript", 65536, 3);

RPCRUN(script_get)
{
    if (!web_blob_caller_ok(ch))
        return;

    DLString id = web_blob_account(ch);

    Json::Value msg;
    msg["command"] = "script_data";
    Json::Value &data = msg["args"][0];
    data["account"] = !id.empty();
    data["b64"] = "";
    data["stamp"] = 0;

    Json::Value stored;
    if (!id.empty() && webscript_table.load(id, stored)) {
        data["b64"] = stored["b64"];
        data["stamp"] = stored["stamp"];
    }

    ch->desc->writeWSCommand(msg);
}

RPCRUN(script_put)
{
    if (!web_blob_caller_ok(ch))
        return;

    DLString id = web_blob_account(ch);

    Json::Value msg;
    msg["command"] = "script_saved";
    Json::Value &data = msg["args"][0];

    if (id.empty()) {
        data["ok"] = false;
        data["stamp"] = 0;
        data["reason"] = "no_account";
    } else {
        webscript_table.put(ch, id, args.empty() ? DLString::emptyString : args[0], data);
    }

    ch->desc->writeWSCommand(msg);
}
