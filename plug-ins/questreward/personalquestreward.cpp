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

bool PersonalQuestReward::canEquip( Character *ch )
{
    if (!obj->hasOwner( ch )) {
        ch->pecho( "Ты не можешь владеть %1$O5 и бросаешь %1$P2.", obj );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}
    
void PersonalQuestReward::get( Character *ch ) 
{ 
    if (ch->is_immortal())
        return;
    
    if (!canEquip( ch ))
        return;

    act_p("{BМерцающая аура окружает $o4.\n\r{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
}


bool PersonalQuestReward::save( ) {
    Character *ch = obj->getCarrier( );

    if (!ch || ch->is_immortal( ))
        return false;
    
    if (obj->hasOwner( ch )) 
        return false;
    
    act_p("$o1 исчезает!", ch, obj, 0, TO_CHAR, POS_RESTING);
    extract_obj(obj);
    return true;
}

bool PersonalQuestReward::hourly()
{
    if (!obj->in_room)
        return false;

    if (IS_SET(obj->in_room->room_flags, ROOM_MANSION))
        return false;

    if (obj->in_room->vnum == ROOM_VNUM_BUREAU_1 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_2 || 
        obj->in_room->vnum == ROOM_VNUM_BUREAU_3)
        return false;

    if (!obj->getProperty("keepHere").empty())
        return false;

    if (!obj->getOwner())
        return false;

    if (!obj->can_wear(ITEM_TAKE))
        return false;

    notice("Item %d %lld of %s transferred from room [%d] [%s] to lost&found.",
            obj->pIndexData->vnum, obj->getID(), obj->getOwner(),
            obj->in_room->vnum, obj->in_room->getName());
    return true;
}

void PersonalQuestReward::delete_( Character *ch ) {
    if (obj->hasOwner( ch )) 
        extract_obj( obj );
}

bool PersonalQuestReward::isLevelAdaptive( ) {
   return true; 
}

bool PersonalQuestReward::canSteal( Character * ) { 
    return false;
}

