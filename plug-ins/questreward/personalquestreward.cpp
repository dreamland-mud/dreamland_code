/* $Id: personalquestreward.cpp,v 1.1.2.10.6.3 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */
#include "logstream.h"
#include "personalquestreward.h"
#include "class.h"
#include "pcharacter.h"
#include "core/object.h"
#include "room.h"
#include "act.h"
#include "loadsave.h"

#include "vnum.h"
#include "def.h"

void PersonalQuestReward::get( Character *ch ) 
{ 
    if (!canEquip( ch ))
        return;

    oldact_p("{BМерцающая аура окружает $o4.\n\r{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
}


bool PersonalQuestReward::hourly()
{
    if (!obj->in_room)
        return false;

    if (IS_SET(obj->in_room->room_flags, ROOM_MANSION|ROOM_GODS_ONLY))
        return false;

    if (obj->in_room->vnum == ROOM_VNUM_BUREAU_1 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_2 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_3)
        return false;

    if (!obj->getProperty("keepHere").empty())
        return false;

    if (obj->getOwner().empty())
        return false;

    if (!obj->can_wear(ITEM_TAKE))
        return false;

    notice("[cleanup] Item %d %lld of %s transferred from room [%d] [%s] to lost&found.",
            obj->pIndexData->vnum, obj->getID(), obj->getOwner().c_str(),
            obj->in_room->vnum, obj->in_room->getName());
    obj_from_room(obj);
    obj_to_room(obj, get_room_instance(ROOM_VNUM_BUREAU_2));
    return true;
}

bool PersonalQuestReward::isLevelAdaptive( ) 
{
   return true; 
}

