/* $Id$
 *
 * ruffina, 2004
 */
#include "move_utils.h"
#include "exitsmovement.h"
#include "portalmovement.h"
#include "movetypes.h"
#include "transfermovement.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "character.h"
#include "affect.h"
#include "skill.h"
#include "room.h"
#include "roomutils.h"
#include "core/object.h"
#include "clanreference.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(none);
WEARLOC(none);
WEARLOC(stuck_in);

int move_char( Character *ch, int door, const char *argument )
{
    return ExitsMovement( ch, door, movetype_resolve( ch, argument ) ).move( );
}

int move_char( Character *ch, EXTRA_EXIT_DATA *peexit, const char *argument )
{
    return ExitsMovement( ch, peexit, movetype_resolve( ch, argument ) ).move( );
}

int move_char( Character *ch, Object *portal )
{
    return PortalMovement( ch, portal ).move( );
}


void transfer_char( Character *ch, Character *actor, Room *to_room, 
                    const char *msgRoomLeave, const char *msgSelfLeave, 
                    const char *msgRoomEnter, const char *msgSelfEnter )
{
    TransferMovement( ch, actor, to_room, 
                      msgRoomLeave, msgSelfLeave, 
                      msgRoomEnter, msgSelfEnter ).move( );
}


void strip_camouflage( Character *ch )
{
    if ( IS_AFFECTED( ch, AFF_CAMOUFLAGE ) )
    {
            REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
            ch->ambushing = &str_empty[0];
            ch->send_to("Ты выходишь из своего укрытия.\n\r");
            act_p("$c1 выходит из $s укрытия.", ch, 0, 0,TO_ROOM,POS_RESTING);
    }
}

void check_camouflage( Character *ch, Room *to_room )
{
    if ( IS_AFFECTED(ch, AFF_CAMOUFLAGE)
            && !RoomUtils::isNature(to_room))
    {
            strip_camouflage( ch );
    }        
}


/* random room generation procedure */
Room * get_random_room( Character *ch )
{
    Room *room;

    for ( ; ; )
    {
        room = get_room_instance( number_range( 200, 39999 ) ); /* old stuff, but appears all new interesting areas lie beyond this limit */
        if ( room != 0 )
        if ( ch->can_see(room)
        &&   !room->isPrivate()
        &&   room->isCommon()
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY)
        &&   !IS_SET(room->room_flags, ROOM_SAFE)
        &&   !IS_SET(room->room_flags, ROOM_MANSION)
        &&   (!ch->is_npc() 
               || !IS_SET(ch->act,ACT_AGGRESSIVE)
               || !IS_SET(room->room_flags,ROOM_LAW)))
            break;
    }

    return room;
}

Room * get_random_room_vanish( Character *ch )
{
    Room *target = NULL;
    int count = 0;

    for (auto &room: roomInstances) {
        if (room->area != ch->in_room->area || room == ch->in_room)
            continue;
        
        if (IS_SET(room->room_flags, ROOM_NO_VANISH|ROOM_SAFE)) 
            continue;
        
        if (room->pIndexData->clan != clan_none && room->pIndexData->clan != ch->getClan( )) 
            continue;
        
        if (room->isPrivate( )) 
            continue;
        
        if (!ch->can_see( room )) 
            continue;
    
        if (number_range( 0, count++ ) == 0) 
            target = room;
    }

    return target;
}

bool is_flying( Character *ch )
{
    if (ch->posFlags.isSet(POS_FLY_DOWN))
        return false;

    return can_fly( ch );
}

bool can_fly( Character *ch )
{
    if (IS_AFFECTED(ch, AFF_FLYING))
        return true;
        
    if (ch->getRace( )->getAff( ).isSet( AFF_FLYING ))
        return true;
            
    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content) {
        if (obj->wear_loc == wear_none || obj->wear_loc == wear_stuck_in)
            continue;
        
        if (!obj->affected.findAllWithBits(TO_AFFECTS, AFF_FLYING ).empty())
            return true;
        
        if (!obj->enchanted)
            if (!obj->pIndexData->affected.findAllWithBits(TO_AFFECTS, AFF_FLYING ).empty())
                return true;
    }
    
    return false;
}

