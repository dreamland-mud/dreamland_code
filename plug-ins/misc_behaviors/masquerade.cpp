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
#include "mercdb.h"
#include "act.h"
#include "def.h"

#include "logstream.h"
#define log(x) LogStream::sendNotice() << "Masquerade: " << x << endl

GSN(manacles);
GSN(jail);

/*----------------------------------------------------------------------------
 * Masquerade area behavior (handles objects resets)
 *----------------------------------------------------------------------------*/
void Masquerade::setArea( AREA_DATA *area ) {
    AreaBehavior::setArea( area );
}

typedef std::vector<NPCharacter *> MobileVector;
typedef std::map<int, MobileVector> MobileHash;

void Masquerade::update( ) {
    MobileHash masquers;

    for (Character *wch = char_list; wch; wch = wch->next) {
	NPCharacter *mob = wch->getNPC();

	if (!mob || mob->pIndexData->area != area)
	    continue;
	
	masquers[ mob->pIndexData->vnum ].push_back( mob );
    }

    for (map<int, Room *>::iterator i = area->rooms.begin( ); i != area->rooms.end( ); i++) {
	Room *room = i->second;

	for (RESET_DATA *pReset = room->reset_first; pReset; pReset = pReset->next) {
	    RESET_DATA *old_reset;
	    
	    if (pReset->command == 'M') {
		MobileHash::iterator ipos = masquers.find( pReset->arg1 );
		
		if (ipos == masquers.end( )) 
		    continue;

		old_reset = pReset;

		for (pReset = pReset->next; pReset; pReset = pReset->next) {
		    if (pReset->command != 'G' && pReset->command != 'E') {
			break;
		    }

		    for (MobileVector::iterator iter = ipos->second.begin( ); iter != ipos->second.end( ); iter++) {
			OBJ_INDEX_DATA *pObjIndex;
			Object *o;
			NPCharacter *mob = *iter;
			char cmd = pReset->command;
			int loc = pReset->arg3;

			pObjIndex = get_obj_index( pReset->arg1 );

			if (!pObjIndex) 
			    continue;

			if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit)
			    continue;

			for (o = mob->carrying; o; o = o->next_content) 
    			    if (o->pIndexData == pObjIndex) 
				break;

			if (o == NULL) {
			    o = create_object( pObjIndex, 0 );
			    obj_to_char( o, mob );

			    if (cmd == 'E' && !get_eq_char( mob, loc ))
				wearlocationManager->find( loc )->equip( o );

			    continue;
			}

			if (o->wear_loc == wear_none) {
			    if (cmd == 'E' && !get_eq_char( mob, loc )) {
				wearlocationManager->find( loc )->equip( o );
			    } 
			}
			else if (o->wear_loc != loc) {
			    if (cmd == 'E' && !get_eq_char( mob, loc )) {
				o->wear_loc->unequip( o );
				wearlocationManager->find( loc )->equip( o );
			    }
			    if (cmd == 'G') {
				o->wear_loc->unequip( o );
			    }
			}
		    }
		}

		pReset = old_reset;
	    }
	}
    }
    
}

/*----------------------------------------------------------------------------
 * Masquer 
 *----------------------------------------------------------------------------*/
void Masquer::speech( Character *victim, const char *speech ) 
{
    Object *obj;
    char buf [256];
    char charName[MAX_INPUT_LENGTH], objName[MAX_INPUT_LENGTH];
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
    AREA_DATA *pArea, *area;
    Room *room;
    int vnum, count;
    
    if (!obj)
	return false;
    
    if (--current > 0)
	return false;

    vnum = obj->pIndexData->vnum;
    area = NULL;
    room = NULL;
    
    for (count = 0, pArea = area_first; pArea; pArea = pArea->next) {
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

    for (map<int, Room *>::iterator i = area->rooms.begin( ); i != area->rooms.end( ); i++) {
	if (number_range( 0, count++ ) == 0) 
	    room = i->second;
    }

    if (!room)
	return false;

    obj_from_room( obj );
    obj_to_room( obj, room );

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

    room = get_room_index( recall.getValue( ) );
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
	    || victim->in_room->vnum == 4)) 
    {
	act("Кошачьий глаз ярко вспыхивает зеленым и исчезает.", victim, 0, 0, TO_ALL);
	act("Кошачий Бог не хочет тебя сейчас видеть.", victim, 0, 0, TO_CHAR);
    } else {
	act("Кошачьий глаз ярко вспыхивает зеленым и исчезает.", victim, 0, 0, TO_CHAR);

	transfer_char( victim, victim, room,
	               "Кошачьий глаз ярко вспыхивает и исчезает, увлекая %1$^C4 за собой в зеленом вихре.",
		       "Могущество Кошачьего Бога переносит тебя к его Алтарю.", 
	               "%1$^C1 появил%1$Gось|ся|ась в комнате." );
    }
   
    return true;
}
