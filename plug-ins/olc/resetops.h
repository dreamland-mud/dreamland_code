#ifndef __RESETOPS_H__
#define __RESETOPS_H__

#include "dlstring.h"

class Character;
class RoomIndexData;

/*
 * Reset list edits of one room, shared by the 'resets' command and the OLC ops.
 * Each one tells ch what it did or why not, marks the area changed and returns
 * true when the reset list changed. 'number' is the 1-based reset number shown
 * by 'resets'. A new reset goes after reset 'after': 0 puts it first, a number
 * past the end (or below 0) appends it.
 */
bool reset_add_mob(Character *ch, RoomIndexData *pRoom, int after, int vnum, int maxArea, int maxRoom);
bool reset_add_obj_room(Character *ch, RoomIndexData *pRoom, int after, int vnum);
bool reset_add_obj_inside(Character *ch, RoomIndexData *pRoom, int after, int vnum,
                          int container, int limit, int count);
/** Equip on (or with wearloc 'none', give to) the mob of the reset before it. */
bool reset_add_obj_wear(Character *ch, RoomIndexData *pRoom, int after, int vnum, const DLString &wearloc);
bool reset_delete(Character *ch, RoomIndexData *pRoom, int number);
bool reset_toggle_flags(Character *ch, RoomIndexData *pRoom, int number, const DLString &flags);
bool reset_set_rand(Character *ch, RoomIndexData *pRoom, int number, const DLString &rand);
bool reset_set_tier(Character *ch, RoomIndexData *pRoom, int number, int tier);
bool reset_set_wearloc(Character *ch, RoomIndexData *pRoom, int number, const DLString &wearloc);
/** Shuffle the room's exits on every reset ('resets void'). */
bool reset_randomize_exits(Character *ch, RoomIndexData *pRoom);

#endif
