#include "itemutils.h"
#include "core/object.h"
#include "room.h"
#include "character.h"
#include "merc.h"
#include "def.h"

bitstring_t Item::furnitureFlags(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value2();
    
    case ITEM_FOUNTAIN:
        return obj->value4();
    }
}    
 
bitstring_t Item::furnitureMaxPeople(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value0();
    
    case ITEM_FOUNTAIN:
        return obj->value3();
    }
}

/* 
 * returns number of people on an object 
 */
int Item::countUsers(Object *obj)
{
    Character *fch;
    int count = 0;

    if (obj->in_room == 0)
        return 0;

    for (fch = obj->in_room->people; fch != 0; fch = fch->next_in_room)
        if (fch->on == obj)
            count++;

    return count;
}

bool Item::canDrop( Character *ch, Object *obj, bool verbose )
{
    if (!IS_SET(obj->extra_flags, ITEM_NODROP))
        return true;

    if (!ch->is_npc() && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
        return true;

    if (verbose)
        ch->pecho("Ты не можешь избавиться от этого.");

    return false;
}

