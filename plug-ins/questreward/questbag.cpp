/* $Id: questbag.cpp,v 1.1.2.8.18.1 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */
#include "logstream.h"
#include "questbag.h"
#include "class.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "core/object.h"
#include "vnum.h"
#include "def.h"

bool QuestBag::canLock( Character *ch ) 
{ 
    PCMemoryInterface *pcm;
    
    if (!obj->getOwner( ))
        return false;

    pcm = PCharacterManager::find( obj->getOwner( ) );

    if (!pcm)
        return false;

    if (pcm->getAttributes( ).isAvailable( "fullloot" ))
        return true;
    
    return obj->hasOwner( ch );
}

bool QuestBag::hourly()
{
    if (PersonalQuestReward::hourly())
        return true;

    if (!obj->in_room)
        return false;

    if (IS_SET(obj->in_room->room_flags, ROOM_MANSION|ROOM_GODS_ONLY))
        return false;

    if (obj->in_room->vnum == ROOM_VNUM_BUREAU_1 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_2 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_3)
        return false;

    if (obj->can_wear(ITEM_TAKE))
        return false;

    if (!obj->getProperty("keepHere").empty())
        return false;

    if (!obj->getOwner())
        return false;

    PCMemoryInterface *owner = PCharacterManager::find(obj->getOwner());
    if (owner && owner->getLastAccessTime().getTime() >= 1529205799) // been here since 2018
        return false;

    //obj->properties["oldRoom"] = obj->in_room->vnum;
    notice("Chest %d %lld of %s transferred from room [%d] [%s] to storage.",
            obj->pIndexData->vnum, obj->getID(), obj->getOwner(),
            obj->in_room->vnum, obj->in_room->getName());

    return true;
}

void QuestBag::show( Character *victim, ostringstream &buf )
{
    if (!obj->hasOwner(victim))
        buf << "({YЛичное{x) ";
}
