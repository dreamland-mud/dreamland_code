/* $Id$
 *
 * ruffina, 2004
 */
#include "exitsmovement.h"
#include "terrains.h"
#include "directions.h"
#include "movetypes.h"
#include "move_utils.h"
#include "doors.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "wearlocation.h"

#include "affectflags.h"
#include "itemflags.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(confuse);
WEARLOC(none);

ExitsMovement::ExitsMovement( Character *ch, int movetype )
                   : Walkment( ch )
{
    init( );
    this->movetype = movetype;
}

void ExitsMovement::init( )
{
    pexit = NULL;
    peexit = NULL;
    exit_info = 0;
    door = -1;
}

ExitsMovement::ExitsMovement( Character *ch, int door, int movetype )
                   : Walkment( ch )
{
    init( );
    this->door = door;
    this->movetype = movetype;
    
    if (door < DIR_SOMEWHERE) {
        pexit = ch->in_room->exit[door];

        if (pexit)
            exit_info = pexit->exit_info;
    }
}

ExitsMovement::ExitsMovement( Character *ch, EXTRA_EXIT_DATA *peexit, int movetype )
                   : Walkment( ch )
{
    init( );
    this->movetype = movetype;
    this->door = DIR_SOMEWHERE;
    this->peexit = peexit;
    exit_info = peexit->exit_info;
}

static DLString rprog_exit_location(Room *room, Character *wch, Room *to_room, int door)
{
    FENIA_STR_CALL(room, "ExitLocation", "CRi", wch, to_room, door);
    FENIA_STR_CALL(to_room, "EntryLocation", "CRi", wch, room, door);
    return "";
}

bool ExitsMovement::findTargetRoom( )
{
    if (door < 0 || door > DIR_SOMEWHERE)
        return false;

    if (!pexit && !peexit) {
        ch->println( "Извини, но ты не можешь туда идти." );
        return false;
    }
    
    randomizeExits( ); 

    if (pexit)
        to_room = pexit->u1.to_room;
    else
        to_room = peexit->u1.to_room;
    
    if(to_room) {
        DLString rc = rprog_exit_location(from_room, ch, to_room, door);
        int targetVnum = (rc.empty( ) || !rc.isNumber( ) ? 0 : rc.toInt( ));

        if (targetVnum != 0) {
            to_room = get_room_instance(targetVnum);
        }
    }

    if (!to_room) { /* sanity check, will be re-checked in checkVisibility */
        ch->println( "Жаль, но ты не можешь туда идти." );
        return false;
    }

    return true;
}
    

void ExitsMovement::randomizeExits( )
{
    int count = 0;

    if (!IS_ROOM_AFFECTED(from_room, AFF_ROOM_RANDOMIZER)
        && !ch->isAffected( gsn_confuse ))
        return;
    
    for (int d = 0; d < DIR_SOMEWHERE; d++) {
        EXIT_DATA *pe;

        if (!( pe = from_room->exit[d] ))
            continue;

        if (!ch->can_see( pe ))
            continue;
        
        if (number_range( 0, count++ ) == 0) {
            pexit = pe;
            door = d;
        }
    }
    
    for (EXTRA_EXIT_DATA *pee = from_room->extra_exit; pee; pee = pee->next) {
        if (!ch->can_see( pee ))
            continue;

        if (number_range( 0, count++ ) == 0) {
            peexit = pee;
            door = DIR_SOMEWHERE;
        }
    }

    if (door == DIR_SOMEWHERE)
        pexit = NULL;
    else
        peexit = NULL;
}


bool ExitsMovement::canMove( Character *wch )
{
    return Walkment::canMove( wch )
           && checkExtraExit( wch );
}

int ExitsMovement::getDoorStatus(Character *wch)
{
    if (!IS_SET(exit_info, EX_CLOSED))
        return RC_MOVE_OK;
        
    if (wch->get_trust( ) >= ANGEL)
        return RC_MOVE_PASS_ALWAYS;

    if (wch->is_mirror( ))
        return RC_MOVE_PASS_ALWAYS;
            
    if (IS_GHOST( wch )) 
        if (!IS_SET(to_room->room_flags, ROOM_MANSION) 
            || IS_SET(from_room->room_flags, ROOM_MANSION))
        return RC_MOVE_PASS_ALWAYS;

    if (IS_AFFECTED(wch, AFF_PASS_DOOR)) {
        if (IS_SET(exit_info, EX_NOPASS))
            return RC_MOVE_PASS_NEVER;
        else
            return RC_MOVE_PASS_POSSIBLE;
    }

    if (IS_SET(exit_info, EX_LOCKED))
        return RC_MOVE_PASS_NEEDED;
    else
        return RC_MOVE_CLOSED;
}

bool ExitsMovement::checkClosedDoor( Character *wch )
{
    rc = getDoorStatus(wch);

    if (rc == RC_MOVE_OK || rc == RC_MOVE_PASS_ALWAYS)
        return true;

    if (rc == RC_MOVE_PASS_NEVER) {
        msgSelfRoom( wch,
                     "Через %4$N4 невозможно пройти насквозь.",
                     "%2$^C1 стукается лбом о %4$N4." );
        return false;
    }

    // Attempt to open closed door when running.
    if (movetype == MOVETYPE_RUNNING && pexit && !IS_SET(pexit->exit_info, EX_LOCKED)) 
    {
        open_door(ch, door);
        rc = RC_MOVE_OK;
        exit_info = pexit->exit_info;
        return true;
    } 

    if (rc == RC_MOVE_PASS_POSSIBLE)
        return true;

    msgSelfParty( wch,
                    "Тут закрыто, попробуй {y{hcоткрыть %4$N4{x.",
                    "Тут закрыто, попробуй {y{hcоткрыть %4$N4{x." );
    return false;
}

bool ExitsMovement::checkVisibility( Character *wch )
{
    if ((pexit && !wch->can_see( pexit ))
        || (peexit && !wch->can_see( peexit )))
    {
        msgSelfParty( wch, 
                      "Жаль, но ты не можешь туда идти.",
                      "Жаль, но %2$C1 не может туда идти.");
        return false;
    }

    return Walkment::checkVisibility( wch );
}

static bool rprog_cant_walk( Room *room, Character *wch, const char *eename )
{
    FENIA_CALL(room, "CantWalk", "Cs", wch, eename);
    return false;
}

bool ExitsMovement::checkExtraExit( Character *wch )
{
    int total_size;
    
    if (!peexit)
        return true;
    
    if (rprog_cant_walk( from_room, wch, peexit->keyword ))
        return false;

    if (MOUNTED(wch))
        return true;
    
    total_size = wch->size;
    
    if (rider) 
        total_size += rider->size / 2;
    
    if (total_size > peexit->max_size_pass) {
        msgSelfParty( wch, 
                      "Чтобы это сделать, надо быть чуууточку поменьше размером.",
                      "Вам с %2$C5 стоит быть чуточку поменьше размером." );
        return false;
    }
    
    if (IS_SET(peexit->exit_info, EX_NOFLY) && is_flying( wch )) {
        msgSelfParty( wch, 
                      "Ты не сможешь здесь пролететь.",
                      "%2$^C1 не может здесь пролететь." );
        return false;
    }

    if (IS_SET(peexit->exit_info, EX_NOWALK) && !is_flying( wch )) {
        msgSelfParty( wch, 
                      "Ты не сможешь здесь пройти.",
                      "%2$^C1 не сможет здесь пройти." );
        return false;
    }

    if (IS_SET(peexit->exit_info, EX_SWIM_ONLY) && boat_type == BOAT_NONE) {
        msgSelfParty( wch, 
                      "Здесь ты можешь только проплыть.",
                      "%2$^C1 сможет здесь только проплыть." );
        return false;
    }

    return true;
}

bool ExitsMovement::tryMove( Character *wch )
{
    return Walkment::tryMove( wch )
           && applyPassDoor( wch );
}

bool ExitsMovement::applyPassDoor( Character *wch )
{
    int doorLevel, chance;
    int delta, minRange, maxRange;
    
    if (!IS_AFFECTED(wch, AFF_PASS_DOOR))
        return true;

    if (!IS_SET(exit_info, EX_CLOSED))
        return true;

    doorLevel = (peexit ? peexit->level : pexit->level);

    delta = 10;
    minRange = doorLevel - delta;
    maxRange = doorLevel + delta;
    chance = (getPassDoorLevel( wch ) - minRange) * 100 / (maxRange - minRange);
    chance = URANGE( 25 / max( 1, ch->getModifyLevel( ) / 20), 
                     chance, 
                     95 );

    if (number_percent( ) < chance) {
        msgSelf( wch, "Ты просачиваешься сквозь %4$N4." );
        wch->setWait( 2 );
        return true;        
    }
    
    rc = RC_MOVE_PASS_FAILED;
    wch->setWait( 4 );
    msgSelfRoom( wch,
                 "Твоя попытка просочиться сквозь %4$N4 закончилась неудачей.", 
                 "%2$^C1 стукается лбом о %4$N4." );
    return false;
}

int ExitsMovement::getPassDoorLevel( Character *wch )
{
    int eqLevel = -1, castLevel = -1;

    if (wch->getRace( )->getAff( ).isSet( AFF_PASS_DOOR ))
        return wch->getModifyLevel( ) * 11 / 10; 

    for (Affect *paf = wch->affected; paf; paf = paf->next)
        if (paf->where == TO_AFFECTS && IS_SET(paf->bitvector, AFF_PASS_DOOR))
            castLevel = max( castLevel, (int)paf->level );

    for (Object *obj = wch->carrying; obj; obj = obj->next_content)
        if (obj->wear_loc != wear_none) {
            for (Affect *paf = obj->affected; paf; paf = paf->next)
                if (paf->where == TO_AFFECTS && IS_SET(paf->bitvector, AFF_PASS_DOOR))
                    eqLevel = max( eqLevel, obj->level );

            if (!obj->enchanted)
                for (Affect *paf = obj->pIndexData->affected; paf; paf = paf->next)
                    if (paf->where == TO_AFFECTS && IS_SET(paf->bitvector, AFF_PASS_DOOR))
                        eqLevel = max( eqLevel, obj->level );
        }

    return max( 0, max( eqLevel, castLevel ) );
}

int ExitsMovement::getMoveCost( Character *wch )
{
    int move;
    
    move = terrains[from_room->sector_type].move
            + terrains[to_room->sector_type].move;

    move /= 2;  /* i.e. the average */

    /* conditional effects */
    if (is_flying( wch ) || IS_AFFECTED(wch, AFF_HASTE))
        move /= 2;

    if (IS_AFFECTED(wch, AFF_SLOW))
        move *= 2;
    
    return move;
}

void ExitsMovement::setWaitstate( )
{
    int waittime = 0;
    
    waittime += movetypes[movetype].wait;
    waittime += terrains[from_room->sector_type].wait;
    
    ch->setWait( waittime );
}

void ExitsMovement::msgOnMove( Character *wch, bool fLeaving )
{
    ostringstream buf;
    
    if (MOUNTED(wch))
        return;

    if (wch->is_mirror( ))
        return;

    if (ch->invis_level >= LEVEL_HERO || wch->invis_level >= LEVEL_HERO)
        return;
    
    if (peexit) {
        if (fLeaving)
            buf << "%1$^C1 " << extra_move_ru[peexit->moving_from] << " "
                << extra_move_rt[peexit->moving_mode_from] << " "
                << russian_case( peexit->short_desc_from,
                                 extra_move_rtum[peexit->moving_mode_from] );
        else
            buf << "%1$^C1 " << extra_move_rp[peexit->moving_to] << " сюда " 
                << extra_move_rt[peexit->moving_mode_to] << " "
                << russian_case( peexit->short_desc_to,
                                 extra_move_rtpm[peexit->moving_mode_to] );
    }
    else {
        int mt = adjustMovetype( wch );

        if (IS_AFFECTED(wch, AFF_SNEAK | AFF_CAMOUFLAGE) && movetypes[mt].sneak)
            return;

        buf << "%1$^C1 "
            << (fLeaving ? movetypes[mt].leave : movetypes[mt].enter)
            << " " 
            << (fLeaving ? dirs[door].leave : dirs[dirs[door].rev].enter);

        if ((mt == MOVETYPE_SWIMMING || mt == MOVETYPE_WATER_WALK) && boat) {
            int ncase = 0;
            DLString part, prep;
                    
            switch (boat->value1()) {
            case POS_RESTING:  part = "лежа"; break;
            case POS_SITTING:  part = "сидя"; break;
            case POS_STANDING: part = "стоя"; break;
            }
            
            switch (boat->value2()) {
            case PUT_IN:     prep = "в";      ncase = 6; break;
            case PUT_ON:     prep = "на";     ncase = 6; break;
            case PUT_AT:     prep = "у";      ncase = 2; break;
            case PUT_INSIDE: prep = "внутри"; ncase = 2; break;
            }

            if (!prep.empty( )) {
                if (!part.empty( ))
                    buf << ", " << part;

                buf << " " << prep << " %5$O" << ncase;
            }
        }
    }
    
    if (RIDDEN(wch))
        buf << ", верхом на %2$C6";

    buf << "."; 

    msgRoomNoParty( wch, buf.str( ).c_str( ) );
}            

int ExitsMovement::adjustMovetype( Character *wch )
{
    if (IS_GHOST( wch ))
        return MOVETYPE_FLYING;

    if (from_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM)
        switch (boat_type) {
        case BOAT_INV:
        case BOAT_EQ:
            return boat->value0();

        case BOAT_FLY:
            return MOVETYPE_FLYING;
        }
    
    switch (movetype) {
    case MOVETYPE_WALK:
    case MOVETYPE_RUNNING:
    case MOVETYPE_FLEE:
        if (is_flying( wch ))
            return MOVETYPE_FLYING;
        else if (IS_SET(wch->parts, PART_FOUR_HOOVES | PART_TWO_HOOVES))
            return MOVETYPE_RIDING;
        else if (IS_SET(wch->form, FORM_SNAKE))
            return MOVETYPE_SLINK;
        break;
    }

    return movetype;
}

void ExitsMovement::moveOneFollower( Character *wch, Character *fch )
{
    act( "Ты следуешь за $C5.", fch, 0, wch, TO_CHAR );

    if (peexit)
        ExitsMovement( fch, peexit, movetype ).moveRecursive( ); 
    else
        ExitsMovement( fch, door, movetype ).moveRecursive( ); 
}

void ExitsMovement::place( Character *wch )
{
    Walkment::place( wch );
    from_room->history.record( wch, door );
}

void ExitsMovement::msgEcho( Character *victim, Character *wch, const char *msg )
{
    if (canHear( victim, wch ))
        victim->pecho( msg, 
                       (RIDDEN(wch) ? wch->mount : wch),
                       (MOUNTED(wch) ? wch->mount : wch),
                       wch, 
                       peexit ? peexit->short_desc_from : direction_doorname(pexit),
                       boat );
}

