/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __TERRAINS_H__
#define __TERRAINS_H__

class Object;
class Character;

struct terrain_t {
    int sector;
    int move;
    int wait;
    const char * hit;
    const char * fall;
    const char * where;
};

extern const struct terrain_t terrains [];

enum {
    BOAT_NONE,
    BOAT_FLY,
    BOAT_EQ,
    BOAT_INV,
    BOAT_SWIM
};

Object * boat_object_find( Character *ch );
int boat_get_type( Character *ch );

#endif
