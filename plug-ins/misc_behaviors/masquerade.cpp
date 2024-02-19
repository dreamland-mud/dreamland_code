/* $Id: masquerade.cpp,v 1.1.2.10.6.8 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, cradya, 2003
 */

#include "masquerade.h"

#include "class.h"
#include "regexp.h"

#include "skillreference.h"

#include "dlscheduler.h"
#include "mobilebehaviormanager.h"
#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"

#include "interp.h"
#include "merc.h"
#include "handler.h"
#include "move_utils.h"

#include "act.h"
#include "def.h"

#include "logstream.h"
#define log(x) LogStream::sendNotice() << "Masquerade: " << x << endl

GSN(manacles);
GSN(jail);


/*----------------------------------------------------------------------------
 * Masquer 
 *----------------------------------------------------------------------------*/
void Masquer::speech( Character *victim, const char *speech ) 
{
    DLString str = speech;

    // Check mob name is mentioned, for speech prog only.
    if (!arg_contains_someof( str, ch->getNameP( '7' ).c_str( ) )) {
        return;
    }

    tell( victim, speech );
}    

void Masquer::tell( Character *victim, const char *speech ) 
{
    DLString str = speech;

    // Check the magic word is present.
    static RegExp pleasePattern( "pls|пожал[иуй]*ста|please" );
    if (!pleasePattern.match( speech )) {
        return;
    }

    for (Object *obj = ch->carrying; obj; obj = obj->next_content) {
        DLString names = obj->getName( );
        names << obj->getShortDescr( '7' ).colourStrip( );

        if (arg_contains_someof( str, names.c_str( ))) {
            if (!ch->can_see( victim )) {
                tell_dim( victim, ch, "Извини, но я тебя не вижу." );
                return;
            }
            interpret_fmt( ch, "remove '%s'", obj->getName( ) );
            interpret_fmt( ch, "give '%s' %s", obj->getName( ), victim->getName( ).c_str( ) );
            return;
        }
    }

    tell_dim( victim, ch, "Извини, у меня нет этого." );
}

/*----------------------------------------------------------------------------
 * Jumping portal behavior 
 *----------------------------------------------------------------------------*/
RoamingPortal::RoamingPortal( )
{
}

bool RoamingPortal::area( ) 
{
    AreaIndexData *area;
    Room *room;
    int vnum, count;
    
    if (!obj)
        return false;
    
    if (--current > 0)
        return false;

    vnum = obj->pIndexData->vnum;
    area = NULL;
    room = NULL;
    count = 0;
    
    for(auto &pArea: areaIndexes) {
        if (IS_SET(pArea->area_flag, AREA_NOQUEST))
            continue;

        if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
            continue;
        
        if (lowlevel > 0 && pArea->low_range < lowlevel)
            continue;
        
        if (highlevel > 0 && pArea->high_range > highlevel)
            continue;
        
        if (number_range( 0, count++ ) == 0) 
            area = pArea;
    }
    
    if (!area)
        return false;

    count = 0;

    for (map<int, Room *>::iterator i = area->area->rooms.begin( ); i != area->area->rooms.end( ); i++) {
        if (number_range( 0, count++ ) == 0) 
            room = i->second;
    }

    if (!room)
        return false;

    obj->getRoom( )->echo( POS_RESTING, "%1$^O1 {Cярко вспыхивает,{x схлопывается и исчезает!", obj );    
    obj_from_room( obj );
    obj_to_room( obj, room );
    obj->getRoom( )->echo( POS_RESTING, "%1$^O1 {Wпоявляется из ниоткуда,{x приветливо гудя и потрескивая.", obj );    

    current = frequency;
    return false;
}


/*----------------------------------------------------------------------------
 * "Cat's Eye" stone 
 *----------------------------------------------------------------------------*/
void CatsEye::get( Character *victim ) { 
    obj->level = victim->getRealLevel( );
}

bool CatsEye::drop( Character *victim ) { 
    Room *room;

    room = get_room_instance( recall.getValue( ) );
    if (!room) {
        log("wrong recall for cat's eye: " << recall.getValue( ));
        return false;
    }

    extract_obj( obj );

    if (!victim->is_npc( ) 
        && (IS_BLOODY(victim) 
            || IS_SET(victim->act, PLR_WANTED)
            || victim->isAffected(gsn_manacles ) 
            || victim->isAffected(gsn_jail)
            || victim->in_room->vnum == 10)) 
    {
        oldact("Кошачий глаз {Gярко вспыхивает зеленым{x и исчезает.", victim, 0, 0, TO_ALL);
        oldact("{RКошачья Богиня не хочет тебя сейчас видеть.{x", victim, 0, 0, TO_CHAR);
    } else {
        oldact("Кошачий глаз {Gярко вспыхивает зеленым{x и исчезает.", victim, 0, 0, TO_CHAR);

        transfer_char( victim, victim, room,
                       "Кошачий глаз {Gярко вспыхивает{x и исчезает, увлекая %1$^C4 за собой в зеленом вихре.",
                       "Могущество Кошачьей Богини переносит тебя подальше от опасности.", 
                       "%1$^C1 появил%1$Gось|ся|ась в комнате." );
    }
   
    return true;
}
