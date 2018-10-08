/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"

#include "skillreference.h"
#include "skill.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"

#include "act.h"
#include "fight.h"
#include "magic.h"
#include "act_move.h"
#include "exitsmovement.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#ifndef AI_STUB
GSN(pass_door);
GSN(track);
GSN(giant_strength);
GSN(fly);

/*
 * Tracking. Called once per 6 seconds from track_update.
 */
bool BasicMobileBehavior::track( )
{
    Character *wch;
    
    if (!hasLastFought( ))
        return false;

    if (!canTrack( )) 
        return false;
    
    wch = getLastFoughtWorld( );

    if (!canTrackLastFought( wch ))
        return false;

    if (trackCaster( wch ))
        return true;

    if (trackLastFought( wch ))
        return true;

    return false;
}

bool BasicMobileBehavior::canTrack( )
{
    if (!IS_AWAKE(ch))
        return false;
    
    if (ch->fighting)
        return false;

    if (IS_SET(ch->act, ACT_NOTRACK))
        return false;

    if (IS_AFFECTED(ch, AFF_CALM|AFF_CHARM|AFF_SCREAM))
        return false;
    
    if (RIDDEN(ch))
        return false;
    
    if (ch->is_mirror( ))
        return false;

    return true;
}

bool BasicMobileBehavior::canTrackLastFought( Character *wch )
{
    if (!wch)
        return false;

    if (ch->in_room == wch->in_room)
        return false;

    return true;
}

bool BasicMobileBehavior::trackLastFought( Character *wch )
{
    Room *room = ch->in_room;
    EXIT_DATA *pexit;
    int d;

    ch->setWait( gsn_track->getBeats( ) );
    act("$c1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM);

    d = room->history.went( wch );
    pexit = (d == -1 ? NULL : room->exit[d]);

    if (!pexit) {
        act("Ты не видишь здесь следов $C2.", ch, 0, wch, TO_CHAR);
        lostTrack = true;
        return true;
    }
    
    act("Следы $C2 ведут $t.", ch, dirs[d].leave, wch, TO_CHAR);
    
    if (!move( d, pexit, wch )) 
        lostTrack = true;

    return true;
}

bool BasicMobileBehavior::move( int d, EXIT_DATA *pexit, Character *whosHunted )
{
    int rc;

    rc = move_char( ch, d );

    switch (rc) {
    case RC_MOVE_OK:
    case RC_MOVE_PASS_FAILED:
        return true;

    case RC_MOVE_WEB:
        assistSpell( ch, gsn_giant_strength, whosHunted );
        return true;

    case RC_MOVE_PASS_NEVER:
        return false;

    case RC_MOVE_PASS_NEEDED:
        return assistSpell( ch, gsn_pass_door, whosHunted );

    case RC_MOVE_AIR:
    case RC_MOVE_WATER:
        return assistSpell( ch, gsn_fly, whosHunted );
    
    case RC_MOVE_CLOSED:
        open_door_extra( ch, d, pexit );
        return true;

    case RC_MOVE_RESTING:
        interpret_cmd( ch, "wake", "" );
        return true;

    default:
        return false;
    }
}


void BasicMobileBehavior::flee( )
{
    clearLastFought( );
}

void BasicMobileBehavior::shooted( Character *attacker, int door )
{
    Room *temp;
    EXIT_DATA *pExit;
    int opdoor;
    int range = 0;

    if (ch->position == POS_DEAD)
        return;
    
    setLastFought( attacker );

    if (door < 0 || door >= DIR_SOMEWHERE) {
        bug("In path_to_track wrong door: %d",door);
        return;
    }
    
    opdoor = dirs[door].rev;
    temp = attacker->in_room;

    while (1) {
        range++;
        
        if ( ch->in_room == temp ) 
            break;
            
        if ((pExit = temp->exit[ door ]) == 0
            || (temp = pExit->u1.to_room ) == 0)
        {
            bug("In path_to_track: couldn't calculate range %d",range);
            return;
        }

        if (range > 100) {
            bug("In path_to_track: range exceeded 100",0);
            return;
        }
    }

    temp = ch->in_room;

    while (--range > 0) {
        temp->history.record( attacker, opdoor );

        if ((pExit = temp->exit[opdoor]) == 0
            || (temp = pExit->u1.to_room ) == 0 )
        {
            break;
        }
    }
}

#if 0
{
    if (ch->in_room->history.traverse( ch->in_room, victim )) {
        int d;
        EXIT_DATA *pexit;
    
        act("$c1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM);
        d = ch->in_room->history.went( victim );
        pexit = ch->in_room->exit[d];

        if (IS_SET(pexit->exit_info, EX_CLOSED)) 
            open_door_extra( ch, d, pexit );

        return move_char(ch, d, false, "normal");
    }
    
    if (ch->in_room->area != ch->zone) 
        return chance( 10 ) && backHome( false );

    if (ch->in_room->area == victim->in_room->area) 
        return chance( 10 ) && trackTraverseSameZone( victim->in_room );

    return trackCaster( victim );
}
#endif
#endif
