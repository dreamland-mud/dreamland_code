/*
 * Server-side home of the web client's triggers (pattern -> highlight, sound,
 * commands), edited on the Triggers page of the settings window.
 *
 * Two lists, character and account (see WebBlobPair in webblob.h); the server
 * never parses the triggers, the client does. Files:
 * db/webtriggers_char/<name>.json and db/webtriggers_acct/<accountId>.json.
 *
 * triggers_get                   -> triggers_data
 * triggers_put <acct|char> <b64> -> triggers_saved
 */
#include "webblob.h"
#include "rpccommandmanager.h"

/** 64 KiB per list, decoded. One put per second per connection and list, so
 *  saving both lists after a move between them is not refused. */
static WebBlobPair triggers("triggers", "webtriggers_char", "webtriggers_acct", 65536, 1);

RPCRUN(triggers_get)
{
    triggers.get(ch);
}

RPCRUN(triggers_put)
{
    triggers.put(ch, args);
}
