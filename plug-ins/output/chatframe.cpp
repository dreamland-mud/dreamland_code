/* Structured chat frames: the emitter. See chatframe.h for why it lives here. */
#include <ctime>
#include <sstream>

#include "jsoncpp/json/json.h"

#include "grammar_entities_impl.h"
#include "chatframe.h"
#include "mudtags.h"
#include "descriptor.h"
#include "outofband.h"
#include "character.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

/* Web clients subscribe with the chat RPC. A telnet client that negotiated GMCP
 * always gets chat, as Comm.Channel.Text (plug-ins/gmcp). */
bool chat_subscribed( Descriptor *d )
{
    if (!d)
        return false;

    if (d->websock.state == WS_ESTABLISHED)
        return IS_SET(d->oob_proto, OOB_CHAT);

    return IS_SET(d->oob_proto, OOB_GMCP);
}

bool chat_subscribed( Character *ch )
{
    return ch && !ch->is_npc( ) && chat_subscribed( ch->desc );
}

/** Who the other side is, rendered the way the text was.
 *
 *  The console line is already anonymity-aware, and a structured field that
 *  leaked the real name would be worse than the console ever was, because that
 *  is where the eye goes. So the name comes from toNoun with the same flags the
 *  output path uses -- an immortal in wizinvis is "Immortal", an unseen speaker
 *  is "someone", a doppelganger is whoever it is pretending to be -- and the
 *  login name rides along only when the viewer can actually see the speaker.
 *
 *  Without that last rule a panel could correlate "someone" with a real player
 *  across messages, which is exactly what the game refuses to let it do. */
static void chat_peer( Json::Value &body, Character *to, Character *peer )
{
    bool seen = to->can_see( peer );

    // The name goes out as plain text: the panel draws its own header and has
    // no palette to resolve "{CImmortal{x" with, which is exactly the string a
    // wizinvis immortal declines to. Colour is stripped, nothing else is --
    // quotes and angle brackets in a player name are the client's to escape.
    ostringstream name;
    mudtags_convert( peer->toNoun( to, FMT_INVIS | FMT_DOPPEL )->decline( '1' ).c_str( ),
                     name,
                     TAGS_CONVERT_VIS | TAGS_CONVERT_COLOR | TAGS_ENFORCE_NOWEB
                     | TAGS_ENFORCE_NOCOLOR | TAGS_ENFORCE_RAW );

    Json::Value out;
    out["name"] = name.str( );
    out["anon"] = !seen;

    // Nothing about a speaker the viewer cannot see -- not even whether it is a
    // mob. The console says "someone" and stops there; a panel that could tell
    // a hidden thief from a wandering beast would be telling the player
    // something the game deliberately withheld.
    if (seen) {
        out["npc"] = peer->is_npc( );

        // The login name: stable across languages and cases, never displayed.
        // A mob has none.
        if (!peer->is_npc( ))
            out["key"] = peer->getNameC( );
    }

    body["peer"] = out;
}

void chat_emit( Character *to, Character *peer, bool own,
                const DLString &id, const DLString &kind, const DLString &text,
                const DLString &area, int quest, int step )
{
    if (!chat_subscribed( to ))
        return;

    if (text.empty( ))
        return;

    // The line has to reach the panel in the very shape the console received
    // it: the same colour scheme this player chose, the same escaping of what
    // another player typed. That is one call, and it is the same call
    // Character::send_to makes on its way out -- anything else here would be a
    // second, drifting renderer for the same text.
    ostringstream rendered;
    mudtags_convert( text.c_str( ), rendered, TAGS_CONVERT_VIS | TAGS_CONVERT_COLOR, to );

    Json::Value body;
    body["id"] = id;
    body["kind"] = kind;
    body["dir"] = own ? "out" : "in";
    body["text"] = rendered.str( );
    body["at"] = Json::Int64(time( 0 ));

    if (peer)
        chat_peer( body, to, peer );

    if (!area.empty( ))
        body["area"] = area;

    if (quest >= 0) {
        body["quest"] = quest;
        body["step"] = step;
    }

    if (to->desc->websock.state != WS_ESTABLISHED) {
        outOfBandManager->run( "chat", ChatArgs( to->desc, body ) );
        return;
    }

    Json::Value frame;
    frame["command"] = "chat";
    frame["args"][0] = body;

    to->desc->writeWSCommand( frame );
}
