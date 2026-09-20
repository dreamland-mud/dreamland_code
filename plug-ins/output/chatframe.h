/* Structured chat frames for the web client: one message, one frame.
 *
 * Speech reaches the web client twice -- as console text, glued together with
 * everything else that happened in the same pulse, and as one frame per
 * message, which is what a chat panel can keep a history out of.
 *
 * The emitter lives here, in the lowest library every speech path already
 * links, because those paths are spread across libraries that do not depend on
 * one another: the channel framework (communication), clan talk (clan), the
 * family channel (mlove), the auction and group talk (comm), Fenia mobs
 * (feniaroot) and say_fmt (output). Anything higher up would be callable from
 * some of them and not the others.
 */
#ifndef __CHATFRAME_H__
#define __CHATFRAME_H__

#include "dlstring.h"

class Character;
class Descriptor;

/** Did this connection ask for chat frames? Every emitter call starts here, so
 *  a player who never opens the panel costs the server nothing. */
bool chat_subscribed( Descriptor *d );
bool chat_subscribed( Character *ch );

/** One message, as one recipient saw it.
 *
 *  @param to    who receives the frame; nothing happens unless they are a
 *               player whose connection subscribed
 *  @param peer  the other side of the conversation -- the speaker for an
 *               incoming line, the addressee for an outgoing one. 0 where
 *               there is no counterpart (an undirected channel)
 *  @param own   this is the recipient's own copy of what they said (dir "out")
 *  @param id    the channel id, from Command::getName() -- never the localised
 *               command name: it is the panel's grouping key and has to
 *               survive a language switch
 *  @param kind  the coarse bucket: room, personal, area, world, race, group,
 *               mob, quest
 *  @param text  the line exactly as the console printed it for this recipient,
 *               colour markup and all
 *  @param area  area name in the recipient's language, for kind "area" only
 *  @param quest, step  the quest this speech belongs to, for kind "quest"
 */
void chat_emit( Character *to, Character *peer, bool own,
                const DLString &id, const DLString &kind, const DLString &text,
                const DLString &area = DLString::emptyString,
                int quest = -1, int step = -1 );

#endif
