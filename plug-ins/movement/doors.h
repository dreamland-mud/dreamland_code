#ifndef DOORS_H
#define DOORS_H

class Character;
class Object;
struct extra_exit_data;

void open_door_extra ( Character *ch, int door, void *pexit );
void open_door( Character *, int );
// Unlocks and opens both sides of the door with a carried key, telling ch why not otherwise.
void unlock_door( Character *, int );
bool open_portal( Character *, Object * );
// Portals, containers and drink containers: the only objects open/close/lock/unlock act on.
bool obj_is_closable( Object * );
// Visible extra exit in ch's room for open/close/lock/unlock, honouring N.keyword after objects in reach.
extra_exit_data * find_extra_exit_here( Character *, const char * );

#endif
