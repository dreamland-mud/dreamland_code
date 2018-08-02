/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MOVE_UTILS_H__
#define __MOVE_UTILS_H__

#include <stdio.h>

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


void strip_camouflage( Character *ch );
void check_camouflage( Character *ch, Room *to_room );

Room * get_random_room( Character *ch );
Room * get_random_room_vanish( Character *ch );

bool is_flying( Character *ch );
bool can_fly( Character *ch );

#endif
