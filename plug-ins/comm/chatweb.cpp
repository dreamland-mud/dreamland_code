/* Chat panel support for the web client.
 *
 * The emitter and the subscription flag live lower down (descriptor/chatframe,
 * and the chat_subscribe verb in the websocket dispatcher, which has to answer
 * before there is a character). What is left here is the one thing that needs a
 * plugin: telling a subscribed panel when somebody enters or leaves the world.
 */
#include "jsoncpp/json/json.h"

#include "chatweb.h"
#include "chatframe.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

void ChatWebStateListener::run( int oldState, int newState, Descriptor *d )
{
    if (!chat_subscribed( d ))
        return;

    if (newState != CON_PLAYING && oldState != CON_PLAYING)
        return;

    PCharacter *pch = d->character ? d->character->getPC( ) : 0;

    Json::Value body;
    body["on"] = 1;
    body["playing"] = newState == CON_PLAYING ? 1 : 0;

    // Who it is, so a panel holding messages from the previous character can
    // throw them away rather than mixing two people's history into one thread.
    if (newState == CON_PLAYING && pch)
        body["who"] = pch->getName( );

    Json::Value frame;
    frame["command"] = "chat_state";
    frame["args"][0] = body;

    d->writeWSCommand( frame );
}
