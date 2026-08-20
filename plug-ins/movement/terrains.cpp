/* $Id$
 *
 * ruffina, 2004
 */
#include "move_utils.h"
#include "terrains.h"

#include "pcharacter.h"
#include "object.h"

#include "merc.h"
#include "def.h"

WEARLOC(none);

const struct terrain_t terrains [SECT_MAX] = {
/* type           move  wait   hit           fall         where         whereEn            whereUa */
 { SECT_INSIDE,       1,  0, "об пол",      "на пол",   "на полу",   "on the floor",  "на підлозі" },  // inside
 { SECT_CITY,         2,  0, "об мостовую", "на пол",   "на полу",   "on the floor",  "на підлозі" },  // city
 { SECT_FIELD,        2,  0, "об траву",    "на траву", "на траве",  "on the grass",  "на траві"   },  // field
 { SECT_FOREST,       3,  0, "об землю",    "на землю", "на земле",  "on the ground", "на землі"   },  // forest
 { SECT_HILLS,        4,  0, "об землю",    "на землю", "на земле",  "on the ground", "на землі"   },  // hills
 { SECT_MOUNTAIN,     6,  0, "об землю",    "на землю", "на земле",  "on the ground", "на землі"   },  // mountain
 { SECT_WATER_SWIM,   4,  1, "об дно",      "в воду",   "на воде",   "on the water",  "на воді"    },  // water_swim
 { SECT_WATER_NOSWIM, 1,  1, "об воду",     "в воду",   "на воде",   "on the water",  "на воді"    },  // water_noswim
 { SECT_UNUSED,       6,  1, "об землю",    "на землю", "на земле",  "on the ground", "на землі"   },  // <gap>
 { SECT_AIR,          10, 0, "об облако",   "в облака", "в воздухе", "in the air",    "у повітрі"  },  // air
 { SECT_DESERT,       6,  1, "об песок",    "на песок", "на песке",  "on the sand",   "на піску"   },  // desert
 { SECT_UNUSED,       6,  1, "об землю",    "на землю", "на дне",    "on the bottom", "на дні"     },  // underwater
};

/*-----------------------------------------------------------------------------
 * boats
 *----------------------------------------------------------------------------*/
Object * boat_object_find( Character *ch )
{
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content)
        if (obj->item_type == ITEM_BOAT)
            return obj;
               
    return NULL;
}

int boat_get_types( Character *ch )
{
    Object *boat;
    int types = BOAT_NONE;
    
    if (ch->is_immortal( ) || ch->is_mirror( )){
        SET_BIT(types, BOAT_FLY);
        SET_BIT(types, BOAT_SWIM);
    }

    if (is_flying( ch ))
        SET_BIT(types, BOAT_FLY);

    if(IS_GHOST(ch)){
        SET_BIT(types, BOAT_FLY);
        SET_BIT(types, BOAT_SWIM);
    }

    if (IS_AFFECTED(ch, AFF_SWIM))
        SET_BIT(types, BOAT_SWIM);

    boat = boat_object_find( ch );
    if (boat) {
        if (boat->wear_loc == wear_none)
            SET_BIT(types, BOAT_INV);
        else
            SET_BIT(types, BOAT_EQ);
    }

    return types;
}


