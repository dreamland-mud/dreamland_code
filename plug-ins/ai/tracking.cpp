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
#include "loadsave.h"
#include "fight.h"
#include "magic.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "exitsmovement.h"
#include "interp.h"
#include "merc.h"

#include "def.h"

GSN(pass_door);
GSN(track);
GSN(giant_strength);
GSN(fly);
GSN(gills);

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

    if (IS_CHARMED(ch))
        return false;

    if (IS_AFFECTED(ch, AFF_CALM|AFF_SCREAM))
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

    ch->setWait( gsn_track->getBeats(ch) );
    oldact("$c1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM);

    d = room->history.went( wch );
    pexit = (d == -1 ? NULL : room->exit[d]);

    if (!pexit) {
        oldact("Ты не видишь здесь следов $C2.", ch, 0, wch, TO_CHAR);
        lostTrack = true;
        return true;
    }
    
    oldact("Следы $C2 ведут $t.", ch, dirs[d].leave, wch, TO_CHAR);
    
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

    case RC_MOVE_OUTWATER:
    case RC_MOVE_PASS_NEVER:
        return false;

    case RC_MOVE_PASS_NEEDED:
        return assistSpell( ch, gsn_pass_door, whosHunted );

    case RC_MOVE_AIR:
    case RC_MOVE_WATER:
        return assistSpell( ch, gsn_fly, whosHunted );
    
    case RC_MOVE_UNDERWATER:
        return assistSpell( ch, gsn_gills, whosHunted );

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

void BasicMobileBehavior::shot( Character *attacker, int door )
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

