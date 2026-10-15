/*
 * Server-side home of the web client's macros (a key combination -> commands),
 * edited on the Macros page of the settings window and with #hotkey.
 *
 * Two lists, character and account (see WebBlobPair in webblob.h); the server
 * never parses the macros, the client does. Files:
 * db/webmacros_char/<name>.json and db/webmacros_acct/<accountId>.json.
 *
 * macros_get                   -> macros_data
 * macros_put <acct|char> <b64> -> macros_saved
 */
#include "webblob.h"
#include "rpccommandmanager.h"

/** 32 KiB per list, decoded; one put per second per connection and list. */
static WebBlobPair macros("macros", "webmacros_char", "webmacros_acct", 32768, 1);

RPCRUN(macros_get)
{
    macros.get(ch);
}

RPCRUN(macros_put)
{
    macros.put(ch, args);
}
