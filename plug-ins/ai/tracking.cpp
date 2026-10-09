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
#include "core/object.h"

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
#include "aitrigger.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"

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

    int ai = ai_trigger(false, ch, "onTrackAI", "CC", ch, wch);
    if (ai != AI_UNHANDLED)
        return ai;

    if (trackCaster( wch ))
        return true;

    if (trackLastFought( wch ))
        return true;

    return false;
}

/*
 * Fenia mob AI: one step of the chase, a summon or a step along the trail
 */
bool BasicMobileBehavior::aiTrackStep( Character *quarry )
{
    if (!ch->in_room || !quarry->in_room || quarry->in_room == ch->in_room)
        return false;

    if (trackCaster( quarry ))
        return true;

    return trackLastFought( quarry );
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

/*
 * Where a trail portal leads, or 0 when nobody can tell in advance
 * (random and buggy portals).
 */
static Room * trail_portal_target( Object *portal )
{
    if (IS_SET(portal->value2( ), GATE_RANDOM|GATE_BUGGY) || portal->value3( ) == -1)
        return 0;

    return get_room_instance( portal->value3( ) );
}

/*
 * May the trail be followed into room `to`? Handles the refusal itself.
 */
bool BasicMobileBehavior::trailOpen( Character *wch, Room *to )
{
    Room *room = ch->in_room;

    // A stay-area mob won't follow the trail out of its home zone: the hunt
    // ends at the border instead of leaking an aggressive tracker into another
    // area (e.g. Shalafi ghosts kited into Midgaard). The ACT_STAY_AREA wander
    // gate (specials.cpp) already blocks random moves across the boundary, but
    // the hunt path steps via move_char, which does not check it -- so mirror
    // the gate here. Setting lostTrack (rather than clearing memory) lets a
    // caster mob fall through to summoning the quarry back home next tick.
    if (IS_SET(ch->act, ACT_STAY_AREA) && to->area != room->area) {
        if (!lostTrack)
            oldact(_("$c1 теряет след на границе своих владений и прекращает погоню."),
                   ch, 0, wch, TO_ROOM);
        lostTrack = true;
        return false;
    }

    // Same leak for no_mob rooms (arena lobby, temples): move_char does not
    // check ROOM_NO_MOB, only the wander and flee paths do, so a tracker used
    // to chase a fleeing player straight into a no_mob room. Mobs that live in
    // a no_mob room themselves (lair guardians) keep chasing through the no_mob
    // rooms of their own area.
    // Everyone else gives up and goes home, like checkLastFoughtHiding, rather
    // than parking at the border or escalating to summon via lostTrack.
    Room *resetRoom = get_room_instance( ch->reset_room );

    if (IS_SET(to->room_flags, ROOM_NO_MOB)
        && !(resetRoom && IS_SET(resetRoom->room_flags, ROOM_NO_MOB)
             && resetRoom->area == to->area))
    {
        oldact(_("$c1 упирается в невидимую преграду и прекращает погоню."),
               ch, 0, wch, TO_ROOM);
        clearLastFought( );
        if (isHomesick( ))
            backHome( false );
        return false;
    }

    return true;
}

bool BasicMobileBehavior::trackLastFought( Character *wch )
{
    Room *room = ch->in_room;
    EXIT_DATA *pexit = 0;
    EXTRA_EXIT_DATA *peexit = 0;
    Object *portal = 0;
    Room *to = 0;
    int d = -1;

    ch->setWait( gsn_track->getBeats(ch) );
    oldact(_("$c1 всматривается в землю в поисках следов."),ch,0,0,TO_ROOM);

    // The trail goes on through a door, a portal or an extra exit.
    const RoomHistoryEntry *h = room->history.find( wch );

    if (h && h->went < DIR_SOMEWHERE) {
        d = h->went;
        if (( pexit = room->exit[d] ))
            to = pexit->u1.to_room;
    }
    else if (h && h->portal) {
        portal = trail_portal( room, h->portal );
        if (portal && !ch->can_see( portal ))
            portal = 0;
        if (portal)
            to = trail_portal_target( portal );
    }
    else if (h && h->eexit) {
        if (( peexit = trail_eexit( room, h->eexit ) ))
            to = peexit->u1.to_room;
    }

    if (!pexit && !portal && !peexit) {
        oldact(_("Ты не видишь здесь следов $C2."), ch, 0, wch, TO_CHAR);
        lostTrack = true;
        return true;
    }

    // A random portal leads anywhere but to the quarry: the trail ends here.
    if (portal && !to) {
        lostTrack = true;
        return true;
    }

    // A broken exit (no room behind it) just fails the step below.
    if (to && !trailOpen( wch, to ))
        return true;

    if (portal) {
        oldact(_("Следы $C2 ведут в $o4."), ch, portal, wch, TO_CHAR);

        if (IS_SET(portal->value1( ), EX_CLOSED)) {
            if (!open_portal( ch, portal ))
                lostTrack = true;
            return true;
        }

        // PortalMovement reports every refusal as a plain failure, so the
        // one recovery that matters is done up front.
        if (ch->position < POS_STANDING) {
            interpret_cmd( ch, "wake", "" );
            return true;
        }

        if (move_char( ch, portal ) != RC_MOVE_OK)
            lostTrack = true;
        return true;
    }

    if (peexit) {
        oldact(_("Следы $C2 ведут в потайной проход."), ch, 0, wch, TO_CHAR);
        if (!afterMove( move_char( ch, peexit ), DIR_SOMEWHERE, peexit, wch ))
            lostTrack = true;
        return true;
    }

    oldact(_("Следы $C2 ведут $t."), ch, dirs[d].leave, wch, TO_CHAR);
    
    if (!move( d, pexit, wch )) 
        lostTrack = true;

    return true;
}

bool BasicMobileBehavior::move( int d, EXIT_DATA *pexit, Character *whosHunted )
{
    return afterMove( move_char( ch, d ), d, pexit, whosHunted );
}

/*
 * React to a failed step along the trail: tear a web, cast what the way
 * needs, open the door, get up. True if the hunt goes on. For an extra
 * exit d is DIR_SOMEWHERE and pexit the extra exit, as open_door_extra takes.
 */
bool BasicMobileBehavior::afterMove( int rc, int d, void *pexit, Character *whosHunted )
{
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
        if (!pexit)
            return false;
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

