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
#include "loadsave.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "core/object.h"
#include "save.h"

#include "vnum.h"
#include "def.h"

bool QuestBag::canLock( Character *ch ) 
{ 
    PCMemoryInterface *pcm;
    
    if (obj->getOwner().empty())
        return false;

    pcm = PCharacterManager::find( obj->getOwner( ) );

    if (!pcm)
        return false;

    if (pcm->getAttributes( ).isAvailable( "fullloot" ))
        return true;
    
    return obj->hasOwner( ch );
}

/*
 * Takeable personal items are swept to the Lost and Found by the global
 * lost_and_found_sweep(); what is left here is the old non-takeable chests of
 * players who have not logged in for years -- those go to the storage room.
 */
bool QuestBag::hourly()
{
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

    if (obj->getOwner().empty())
        return false;

    PCMemoryInterface *owner = PCharacterManager::find(obj->getOwner());
    if (owner && owner->getLastAccessTime().getTime() >= 1645679207) // been here since 2022
        return false;

    Room *storage = get_room_instance(ROOM_VNUM_BUREAU_3);
    if (!storage)
        return false;

    Room *from = obj->in_room;

    obj->setProperty("oldRoom", DLString(from->vnum));
    notice("[cleanup] Chest %d %lld of %s transferred from room [%d] [%s] to storage.",
            obj->pIndexData->vnum, obj->getID(), obj->getOwner().c_str(),
            from->vnum, from->getName());

    obj_from_room(obj);
    obj_to_room(obj, storage);

    // Both room snapshots have to reach the disk, otherwise the next reboot
    // loads the chest straight back into the room it was swept out of.
    save_items(from);
    save_items(storage);
    return true;
}

/*
 * The "(Личное)" mark used to be printed here, so only quest bags ever showed
 * it. It is now on every named item, from format_personal_mark in look.cpp.
 */
