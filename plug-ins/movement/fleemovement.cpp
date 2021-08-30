/* $Id$
 *
 * ruffina, 2004
 */
#include "fleemovement.h"
#include "movetypes.h"

#include "skillreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"

#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

FleeMovement::FleeMovement( Character *ch )
                 : ExitsMovement( ch, MOVETYPE_FLEE )
{
}

bool FleeMovement::canFlee( Character *wch )
{
    if (!ExitsMovement::canMove( wch ))
        return false;
    
    if (IS_SET(exit_info, EX_NOFLEE))
        return false;
    
    if ( IS_AFFECTED(wch, AFF_IMMOBILIZED) )
		return false;
      
    if (wch->is_npc( ) 
            && !RIDDEN(wch) 
            && IS_SET(to_room->room_flags, ROOM_NO_MOB))
        return false;

    return true;
}

bool FleeMovement::findTargetRoom( )
{
    EXIT_DATA *targetExit = NULL;
    EXTRA_EXIT_DATA *targetExtraExit = NULL;
    int targetDoor = -1;
    int count = 0;

    silence = true;

    peexit = NULL;

    for (door = 0; door < DIR_SOMEWHERE; door++) {
        if (!( pexit = from_room->exit[door] ))
            continue;

        to_room = pexit->u1.to_room;
        exit_info = pexit->exit_info;
        
        if (!canFlee( ch ) || (ch->mount && !canFlee( ch->mount )))
            continue;
                
        if (number_range( 0, count++ ) == 0) {
            targetExit = pexit;
            targetDoor = door;
        }
    }
    
    door = DIR_SOMEWHERE;
    pexit = NULL;

    for (auto &eexit: from_room->extra_exits) {
        to_room = eexit->u1.to_room;
        exit_info = eexit->exit_info;

        if (!canFlee( ch ) || (ch->mount && !canFlee( ch->mount )))
            continue;

        if (number_range( 0, count++ ) == 0) {
            targetExtraExit = eexit;
            targetDoor = door;
        }
    }

    silence = false;

    switch (targetDoor) {
    case -1:
        msgSelf( ch, "{RПАНИКА!{x Ты не находишь выхода!" );
        return false;

    case DIR_SOMEWHERE:
        pexit = NULL;
        peexit = targetExtraExit;
        to_room = peexit->u1.to_room;
        door = DIR_SOMEWHERE;
        exit_info = peexit->exit_info;
        return true;

    default:
        peexit = NULL;
        pexit = targetExit;
        to_room = pexit->u1.to_room;
        door = targetDoor;
        exit_info = pexit->exit_info;
        return true;
    }
}

bool FleeMovement::canMove( Character * )
{
    return true;
}

bool FleeMovement::checkPositionHorse( )
{
    if (horse->position <= POS_RESTING) {
        msgSelf( ch, "%2$^C1 долж%2$Gно|ен|на сначала встать." );
        return false;
    }

    return true;
}

bool FleeMovement::checkPositionRider( )
{
    return true;
}

bool FleeMovement::checkPositionWalkman( )
{
    return ch->position > POS_RESTING;
}

void FleeMovement::callProgs( Character *wch )
{
    if (wch && wch->is_npc( ) && wch->getNPC( )->behavior)
        wch->getNPC( )->behavior->flee( );

    ExitsMovement::callProgs( wch );
}

void FleeMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (fLeaving) {
        if (wch == rider)
            msgSelf( wch, "%2$^C1 выносит тебя с поля битвы!" );
        else if (wch == horse) 
            msgSelf( wch, "%1$^C1 покидает поле битвы верхом на тебе!" );
        else
            msgSelf( wch, "Ты убегаешь с поля битвы!" );
    }
    
    ExitsMovement::msgOnMove( wch, fLeaving );

    if (fLeaving) 
        msgRoomNoParty( wch, 
                        "%1$^C1 убегает!",
                        "%1$^C1 и %2$C1 убегают!" );
}

bool FleeMovement::applySkill( SkillReference &skill ) 
{
    bool fSuccess = true;;
    
    if (number_percent() > skill->getEffective( ch )) {
        msgSelf( ch, "У тебя не получается сбежать." );
        fSuccess = false;
    }

    skill->improve( ch, fSuccess );        
    return fSuccess;
}
