#include "itemutils.h"
#include "core/object.h"
#include "room.h"
#include "character.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

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
 
const char *Item::FURNITURE_PROPS = "furniture";

const char *Item::furnitureMsgTags[] = {
    "msgSitChar", "msgSitRoom",
    "msgSitRestChar", "msgSitRestRoom",
    "msgRestChar", "msgRestRoom",
    "msgSleepChar", "msgSleepRoom",
    "msgStandChar", "msgStandRoom",
    "msgWakeRestChar", "msgWakeRestRoom",
    "msgWakeSitChar", "msgWakeSitRoom",
    "msgWakeStandChar", "msgWakeStandRoom",
    0
};

DLString Item::furnitureMsgProp(const obj_index_data *pObj, const char *tag)
{
    const Json::Value &props = pObj->props;

    if (!props.isObject() || !props.isMember(FURNITURE_PROPS))
        return DLString::emptyString;

    const Json::Value &msgs = props[FURNITURE_PROPS];
    if (!msgs.isObject() || !msgs.isMember(tag) || !msgs[tag].isString())
        return DLString::emptyString;

    return msgs[tag].asString();
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
        ch->pecho(_("Ты не можешь избавиться от этого."));

    return false;
}

