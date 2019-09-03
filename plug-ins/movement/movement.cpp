/* $Id$
 *
 * ruffina, 2004
 */
#include "movement.h"
#include "movetypes.h"
#include "move_utils.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "affecthandler.h"
#include "skillmanager.h"
#include "skill.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"

#include "interp.h"
#include "worldknowledge.h"
#include "fight_position.h"
#include "fight_exception.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

Movement::Movement( Character *ch ) 
{
    this->ch = ch;
    this->horse = MOUNTED(ch);
    this->rider = RIDDEN(ch);
    this->from_room = ch->in_room;
    this->to_room = NULL;
    this->movetype = MOVETYPE_WALK;
}
    
int Movement::move( )
{
    rc = RC_MOVE_UNDEF;
    
    if (moveRecursive( ))
        rc = RC_MOVE_OK;
    else if (rc == RC_MOVE_UNDEF)
        rc = RC_MOVE_FAIL;

    return rc;
}

bool Movement::moveRecursive( )
{
    if (!canLeave( ch ) || (ch->mount && !canLeave( ch->mount )))
        return false;

    if (!findTargetRoom( ))
        return false;

    if (!moveAtomic( )) 
        return false;

    try {
        callProgs( ch );
        if (ch->in_room != to_room)
            return false;
            
        callProgs( ch->mount );
        moveFollowers( ch );
        moveFollowers( ch->mount );
        callProgsFinish( ch );
        callProgsFinish( ch->mount );
    } 
    catch (const VictimDeathException &) {
    }

    return true;
}

bool Movement::canLeave( Character *wch )
{
    return true;
}

void Movement::place( Character *wch )
{
    Room *old_room = wch->in_room;
    msgOnMove( wch, true );
    
    if (wch->fighting)
        stop_fighting( wch, true );

    char_from_room( wch );
    char_to_room( wch, to_room );

    if (old_room->area != to_room->area 
            && IS_AWAKE( wch ) 
            && !eyes_blinded( wch )
            && !eyes_darkened( wch ))
    {
        wch->printf("Ты попадаешь в местность '{c{hh%s{x'.\r\n\r\n", to_room->area->name);
    }
 
    interpret_raw( wch, "look", "move" );

    msgOnMove( wch, false );
#if 0
    if (!wch->is_npc( ))
        worldKnowledge->visit( wch->getPC( ) );
#endif    
}

static void rafprog_leave( Room *room, Character *ch )
{
    Affect *paf, *paf_next;

    for (paf = room->affected; paf; paf = paf_next) {
        paf_next = paf->next;
        if (paf->type->getAffect( ))
            paf->type->getAffect( )->leave( room, ch, paf );
    }
}

static void rafprog_entry( Room *room, Character *ch )
{
    Affect *paf, *paf_next;

    for (paf = room->affected; paf; paf = paf_next) {
        paf_next = paf->next;
        if (paf->type->getAffect( ))
            paf->type->getAffect( )->entry( room, ch, paf );
    }
}

static void rprog_leave(Room *from_room, Character *walker, Room *to_room, const char *movetype)
{
    FENIA_VOID_CALL( from_room, "Leave", "CRs", walker, to_room, movetype );
}

static bool mprog_greet( Character *rch, Character *walker )
{
    FENIA_CALL( rch, "Greet", "C", walker );
    FENIA_NDX_CALL( rch->getNPC( ), "Greet", "CC", rch, walker );
    BEHAVIOR_VOID_CALL( rch->getNPC( ), greet, walker );
    return false;
}

static bool oprog_greet( Object *obj, Character *walker )
{
    FENIA_CALL( obj, "Greet", "C", walker );
    FENIA_NDX_CALL( obj, "Greet", "OC", obj, walker );
    BEHAVIOR_VOID_CALL( obj, greet, walker );
    return false;
}

static bool oprog_entry( Object *obj )
{
    FENIA_CALL( obj, "Entry", "" );
    FENIA_NDX_CALL( obj, "Entry", "O", obj );
    BEHAVIOR_VOID_CALL( obj, entry );
    return false;
}

void Movement::callProgs( Character *wch )
{
    Character *fch, *fch_next;
    Object *obj;

    if (!wch)
        return;
    
    rafprog_leave( from_room, wch );
    rprog_leave( from_room, wch, to_room, movetypes[movetype].name );
        
    for (fch = to_room->people; fch!=0; fch = fch_next) {
        fch_next = fch->next_in_room;

        /* greet progs for items carried by people in room */
        for ( obj = fch->carrying; obj != 0; obj = obj->next_content )
                oprog_greet( obj, wch );

        /* greet programs for people */
	if (IS_AWAKE(fch))
            mprog_greet( fch, wch );
    }

    /* entry programs for items */
    for (obj = wch->carrying; obj; obj=obj->next_content)
            oprog_entry( obj );
}

static void rprog_greet( Room *to_room, Character *ch, Room *from_room, const char *movetype )
{
    FENIA_VOID_CALL( to_room, "Greet", "CRs", ch, from_room, movetype );
}

static void mprog_leave( Character *ch, Room *from_room, Room *to_room, const char *movetype )
{
    for (Character *rch = from_room->people; rch; rch = rch->next_in_room) {
        FENIA_VOID_CALL( rch, "Leave", "CRs", ch, to_room, movetype );
        FENIA_NDX_VOID_CALL( rch->getNPC( ), "Leave", "CCRs", rch, ch, to_room, movetype );
    }
}

static void afprog_entry( Character *ch )
{
    Affect *paf, *paf_next;

    for (paf = ch->affected; paf; paf = paf_next) {
        paf_next = paf->next;

        if (paf->type->getAffect( ))
            paf->type->getAffect( )->entry( ch, paf );
    }
}

static bool rprog_dive( Character *wch, int danger )
{
    FENIA_CALL( wch->in_room, "Dive", "Ci", wch, danger );
    return false;
}

static bool mprog_entry( Character *walker )
{
    FENIA_CALL( walker, "Entry", "" );
    FENIA_NDX_CALL( walker->getNPC( ), "Entry", "C", walker );
    BEHAVIOR_VOID_CALL( walker->getNPC( ), entry );
    return false;
}

void Movement::callProgsFinish( Character *wch )
{
    Object *obj, *obj_next;
    
    if (!wch)
        return;

    mprog_leave( wch, from_room, to_room, movetypes[movetype].name );

    for (obj = to_room->contents; obj; obj = obj_next) {
        obj_next = obj->next_content;
        oprog_greet( obj, wch );
    }

    mprog_entry( wch );
    rprog_greet( to_room, wch, from_room, movetypes[movetype].name );
    rafprog_entry( to_room, wch );
    afprog_entry( wch );
    rprog_dive( wch, movetypes[movetype].danger );
}

void Movement::msgSelfParty( Character *wch, const char *msgSelf, const char *msgParty )
{
    msgEcho( wch, wch, msgSelf );
    
    if (wch->mount == ch)
        msgEcho( wch->mount, wch->mount, msgParty );
}

void Movement::msgSelfRoom( Character *wch, const char *msgSelf, const char *msgOther )
{
    msgEcho( wch, wch, msgSelf );
    msgRoom( wch, msgOther ); 
}

void Movement::msgRoomNoParty( Character *wch, const char *msgRoom )
{
    for (Character *rch = wch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != wch && rch != wch->mount)
            msgEcho( rch, wch, msgRoom );
}

void Movement::msgRoomNoParty( Character *wch, const char *msgRoomSingle, const char *msgRoomMounted )
{
    for (Character *rch = wch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != wch && rch != wch->mount) {
            if (ch->mount) {
                if (ch->mount == wch)
                    msgEcho( rch, wch, msgRoomMounted );
            }
            else
                msgEcho( rch, wch, msgRoomSingle );
        }
}

void Movement::msgRoom( Character *wch, const char *msg )
{
    for (Character *rch = wch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != wch)
            msgEcho( rch, wch, msg );
}

void Movement::msgSelfMaster( Character *wch, const char *msgSelf, const char *msgMaster )
{
    msgEcho( wch, wch, msgSelf );
    
    if (IS_CHARMED(wch))
        msgEcho( wch->master, wch, msgMaster );
}

void Movement::msgSelf( Character *wch, const char *msgSelf )
{
    msgEcho( wch, wch, msgSelf );
}

bool Movement::canHear( Character *victim, Character *wch )
{
    if (!IS_AWAKE(victim))
        return false;

    return victim->can_sense( wch );
}

void Movement::msgEcho( Character *victim, Character *wch, const char *msg )
{
    if (canHear( victim, wch ))
        victim->pecho( msg, 
                       (RIDDEN(wch) ? wch->mount : wch),
                       (MOUNTED(wch) ? wch->mount : wch),
                       wch );
}

bool Movement::canOrderHorse( )
{
    if (IS_CHARMED(horse)) 
        return !horse->master || horse->master == ch;
    else
        return horse->is_npc( );
}

