/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MOVE_UTILS_H__
#define __MOVE_UTILS_H__

#include <stdio.h>

#include "multimessage.h"

class Character;
class Room;
struct extra_exit_data;
class Object;

int  move_char( Character *ch, int door, const char *argument = NULL );
int  move_char( Character *ch, struct extra_exit_data *peexit, const char *argument = NULL );
int  move_char( Character *ch, Object *portal );

void transfer_char( Character *ch, Character *actor, Room *to_room,
                    const char *msgRoomLeave = NULL, const char *msgSelfLeave = NULL,
                    const char *msgRoomEnter = NULL, const char *msgSelfEnter = NULL );

// Trilingual transfer: leave/self/enter lines as MultiMessages, resolved per
// viewer. First three required -- keeps the message-less 3-arg calls on the
// const-char* overload above with no ambiguity.
void transfer_char( Character *ch, Character *actor, Room *to_room,
                    const MultiMessage &msgRoomLeave, const MultiMessage &msgSelfLeave,
                    const MultiMessage &msgRoomEnter, const MultiMessage &msgSelfEnter = MultiMessage() );


Room * get_random_room( Character *ch );
Room * get_random_room_vanish( Character *ch );

bool is_flying( Character *ch );
bool can_fly( Character *ch );

#endif
