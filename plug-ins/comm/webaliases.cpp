/*
 * Account-wide aliases of the web client, edited on the Aliases page of the
 * settings window next to the character's in-game aliases (alias.cpp).
 *
 * The server only stores them: the client expands them before a line is sent,
 * so they work for every character of the account and never reach the
 * interpreter as aliases. One opaque blob per account (see webblob.h), file
 * db/webaliases_acct/<accountId>.json.
 *
 * aliases_acct_get        -> aliases_acct       {account, b64, stamp}
 * aliases_acct_put <b64>  -> aliases_acct_saved {ok, stamp, reason}
 *                            reason: no_account | too_big | bad_data | rate | io
 * An empty <b64> clears the list.
 */
#include "webblob.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "rpccommandmanager.h"
#include "merc.h"
#include "def.h"

/** 64 KiB per account, decoded; one put per second per connection. */
static WebBlobTable aliases_acct("webaliases_acct", 65536, 1);

RPCRUN(aliases_acct_get)
{
    if (!web_blob_caller_ok(ch))
        return;

    DLString acct = web_blob_account(ch);

    Json::Value msg;
    msg["command"] = "aliases_acct";
    Json::Value &data = msg["args"][0];
    data["account"] = !acct.empty();
    data["b64"] = "";
    data["stamp"] = 0;

    Json::Value stored;
    if (!acct.empty() && aliases_acct.load(acct, stored)) {
        data["b64"] = stored["b64"];
        data["stamp"] = stored["stamp"];
    }

    ch->desc->writeWSCommand(msg);
}

RPCRUN(aliases_acct_put)
{
    if (!web_blob_caller_ok(ch))
        return;

    const DLString &b64 = args.size() > 0 ? args[0] : DLString::emptyString;

    Json::Value msg;
    msg["command"] = "aliases_acct_saved";
    Json::Value &data = msg["args"][0];
    data["ok"] = false;
    data["stamp"] = 0;
    data["reason"] = "";

    DLString acct = web_blob_account(ch);
    if (acct.empty())
        data["reason"] = "no_account";
    else
        aliases_acct.put(ch, acct, b64, data);

    ch->desc->writeWSCommand(msg);
}
