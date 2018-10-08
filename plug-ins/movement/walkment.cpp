/* $Id$
 *
 * ruffina, 2004
 */
#include "walkment.h"
#include "movetypes.h"
#include "move_utils.h"
#include "terrains.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "stats_apply.h"
#include "act.h"
#include "loadsave.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(mount_drive);
GSN(riding);
GSN(web);
GSN(camouflage_move);

Walkment::Walkment( Character *ch )
            : Movement( ch )
{
    silence = false;
    boat_type = boat_get_type( horse ? horse : ch );
    boat = boat_object_find( horse ? horse : ch );
}

bool Walkment::moveAtomic( )
{
    if (horse) {
	if (!canControlHorse( ))
	    return false;

	visualize( horse );
	visualize( ch );

	if (!canMove( horse ) || !canMove( ch ))
	    return false;
	
	if (!tryMove( horse ) || !tryMove( ch ))
	    return false;

	place( horse );
	place( ch );
    }
    else if (rider) {
	visualize( rider );
	visualize( ch );

	if (!canMove( ch ) || !canMove( rider ))
	    return false;

	if (!tryMove( ch ) || !tryMove( rider ))
	    return false;

	place( ch );
	place( rider );
    }
    else {
	visualize( ch );

	if (!canMove( ch ))
	    return false;

	if (!tryMove( ch ))
	    return false;
	
	place( ch );
    }
    
    setWaitstate( );
    return true;
}

bool Walkment::canLeaveMaster( Character *wch )
{
    if (!IS_AFFECTED(wch, AFF_CHARM))
	return true;
    
    if (wch->master == 0)
	return true;
    
    if (from_room != wch->master->in_room)
	return true;
    
    if (wch->master == wch->mount)
	return true;
    
    msgSelfParty( wch, 
	          "Что? И покинуть своего хозяина?",
		  "%^C1 не может покинуть своего хозяина." );
    return false;
}

void Walkment::setWaitstate( )
{
}

static bool rprog_cant_move( Character *ch, Room *to_room, const char *movetype )
{
    FENIA_CALL(ch->in_room, "CantMove", "CRs", ch, to_room, movetype );
    return false;
}

static bool mprog_cant_move( Character *ch, Room *to_room, const char *movetype )
{
    FENIA_CALL( ch, "CantMove", "Rs", to_room, movetype );
    FENIA_NDX_CALL( ch->getNPC( ), "CantMove", "CRs", ch, to_room, movetype );
    return false;
}

static bool mprog_cant_leave( Character *ch )
{
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (rch != ch) {
	    FENIA_CALL( rch, "CantLeave", "C", ch );
	    FENIA_NDX_CALL( rch->getNPC( ), "CantLeave", "CC", rch, ch );
	}
    
    return false;
}

bool Walkment::autoDismount( Character *wch ) 
{
    bool rc;
    
    if (wch == ch)
	return false;
    if (!MOUNTED(wch))
	return false;
    if (wch->mount == wch->master)
	return false;

    silence = true;
    rc = canLeaveMaster( wch );
    silence = false;

    if (rc)
	return false;

    interpret_raw( wch, "dismount" );
    horse = MOUNTED(ch);
    rider = RIDDEN(ch);
    return true;
}

bool Walkment::canLeave( Character *wch )
{
    if (autoDismount( wch ))
	return true;

    return canLeaveMaster( wch )
	    && checkPosition( wch )
	    && !mprog_cant_leave( wch )
	    && checkTrap( wch );
}

bool Walkment::canMove( Character *wch )
{
    return checkVisibility( wch )
	    && checkCyclicRooms( wch )
	    && !mprog_cant_move( wch, to_room, movetypes[movetype].name )
	    && !rprog_cant_move( wch, to_room, movetypes[movetype].name )
            && checkLawzone( wch )
	    && checkClosedDoor( wch )
	    && checkRoomCapacity( wch )
	    && checkGuild( wch )
	    && checkSafe( wch )
	    && checkAir( wch )
	    && checkWater( wch );
}

bool Walkment::checkPosition( Character *wch )
{
    if (horse) {
	if (wch == horse)
	    return checkPositionHorse( );
    }
    else if (rider) {
	if (wch == rider)
	    return checkPositionRider( );
    }
    else {
	return checkPositionWalkman( );
    }

    return true;
}


bool Walkment::checkPositionHorse( )
{
    if (horse->fighting) {
	msgSelf( ch, "Ты долж%1$Gно|ен|на cперва спешиться." ); 
	return false;
    }

    if (horse->position <= POS_RESTING) {
	msgSelfParty( horse,
		      "Исходное положение для ходьбы - стоя!",
		      "%2$^C1 долж%2$Gно|ен|на сначала встать." );
	return false;
    }

    return true;
}

bool Walkment::checkPositionRider( )
{
    if (rider->fighting) {
	msgSelfMaster( ch, 
		       "Твой седок сражается! Сбрось его для начала.",
		       "Седок %1$C2 сражается, прикажи его сбросить!" );
	return false;
    }

    return true;
}

bool Walkment::checkPositionWalkman( )
{
    if (ch->fighting) {
	rc = RC_MOVE_FIGHTING;
	msgSelf( ch, "Куда? Ты же сражаешься!" );
	return false;
    }
    
    if (ch->position < POS_STANDING) {
	rc = RC_MOVE_RESTING;
	msgSelf( ch, "Исходное положение для ходьбы - стоя!" );
	return false;
    }

    return true;
}

void Walkment::visualize( Character *wch )
{
    if (IS_AFFECTED( wch, AFF_HIDE|AFF_FADE ) && !IS_AFFECTED(wch, AFF_SNEAK)) {
	REMOVE_BIT(wch->affected_by, AFF_HIDE|AFF_FADE);
	wch->println( "Ты выходишь из тени." );
	act( "$c1 выходит из тени.", wch, 0, 0, TO_ROOM );
    }

    if (IS_AFFECTED( wch, AFF_CAMOUFLAGE )) {
	if (number_percent( ) < gsn_camouflage_move->getEffective( wch )) {
	    gsn_camouflage_move->improve( wch, true );
	}
	else {
	    strip_camouflage( wch );
	    gsn_camouflage_move->improve( wch, false );
	}
    }
}

bool Walkment::canControlHorse( )
{
    if (!canOrderHorse( )) {
	act( "Ты не можешь управлять $C5.", ch, 0, horse, TO_CHAR );
	return false;
    }
    
    /* horrible XXX until riding skills are available for all */
    if (horse->is_npc( ) 
	    && (   (horse->getNPC( )->pIndexData->vnum >= 50000
	           && horse->getNPC( )->pIndexData->vnum <= 51000)
		|| (horse->getNPC( )->pIndexData->vnum >= 550
		   && horse->getNPC( )->pIndexData->vnum <= 560)))
	return true;

    if (number_percent( ) > gsn_riding->getEffective( ch ) && !ch->isCoder( )) {
	act( "Тебе не хватает мастерства управлять $C5.", ch, 0, horse, TO_CHAR );
	gsn_riding->improve( ch, false );
	return false; 
    }

    /* XXX more checks */
    gsn_riding->improve( ch, true );
    return true;
}

bool Walkment::checkCyclicRooms( Character *wch )
{
    if (from_room == to_room) {
	msgSelfParty( wch, 
		      "С непонятными намерениями ты топчешься на месте... и что дальше?",
	              "%2$^C1 с непонятными намерениями топчется на одном месте." );
	return false;
    }

    return true;
}
    
bool Walkment::checkTrap( Character *wch )
{
    if (wch->death_ground_delay > 0 && wch->trap.isSet( TF_NO_MOVE )) {
	msgSelfParty( wch, 
		     "Ты не можешь покинуть это место - без посторонней помощи!",
	             "%2$^C1 не может покинуть это место без посторонней помощи!" );
	return false;
    }

    return true;
}


bool Walkment::checkVisibility( Character *wch )
{
    if (!wch->can_see( to_room )) {
	msgSelfParty( wch,
		      "Жаль, но ты не можешь туда идти.",
	              "Жаль, но %2$C1 не может туда идти." );
	return false;
    }

    return true;
}

bool Walkment::checkSafe( Character *wch )
{
    if (!IS_SET( to_room->room_flags, ROOM_SAFE ))
	return true;
	
    if (IS_BLOODY(wch)) {
	msgSelfRoom( wch, 
		     "Божественные силы не позволяют тебе войти туда в таком состоянии.",
		     "%2$^C1 предпринимает героическую... и бесполезную попытку войти в комнату." );
	return false;
    }

    return true;
}

bool Walkment::checkGuild( Character *wch )
{
    if (wch->is_immortal( )) 
	return true;
    
    if (wch->is_npc( ))
	return true;
    
    if (to_room->guilds.empty( ))
	return true;

    if (!to_room->guilds.isSet( wch->getProfession( ) )) {
	msgSelfParty( wch, 
		      "Ты не можешь войти в чужую гильдию.", 
		      "%2$^C1 не может войти в чужую гильдию." );
	return false;
    }

    if (IS_BLOODY(wch)) {
	msgSelfParty( wch, 
		      "Твоя гильдия не может сейчас служить тебе укрытием.",
		      "Гильдия не может сейчас служить укрытием для %2$C2." );
	return false;
    }
    
    return true;
}

bool Walkment::checkAir( Character *wch )
{
    if (from_room->sector_type != SECT_AIR && to_room->sector_type != SECT_AIR)
	return true;

    if (MOUNTED(wch))
	return true;

    if (is_flying( wch ))
	return true;

    if (wch->is_immortal( ) || wch->is_mirror( ))
	return true;

    if (IS_GHOST(wch))
	return true;
    
    rc = RC_MOVE_AIR;
    msgSelfParty( wch, 
	          "Ты не умеешь летать.", 
	          "%2$^C1 не умеет летать." );
    return false;
}

bool Walkment::checkWater( Character *wch )
{
    if (from_room->sector_type != SECT_WATER_NOSWIM
	 && to_room->sector_type != SECT_WATER_NOSWIM)
	return true;
    
    if (MOUNTED(wch))
	return true;

    if (boat_type != BOAT_NONE) 
	return true;
    
    rc = RC_MOVE_WATER;
    msgSelfParty( wch, 
	          "Чтоб идти дальше тебе нужна лодка.",
	          "%2$^C1 не умеет ходить по воде." );
    return false;
}

bool Walkment::checkRoomCapacity( Character *wch )
{
    int capacity;
    
    if (wch->is_immortal( ))
	return true;
    
    if (MOUNTED(wch))
	return true;
    
    capacity = to_room->getCapacity( );
    
    if (capacity < 0)
	return true;

    if (capacity < (RIDDEN(wch) ? 2 : 1)) {
	msgSelfParty( wch, 
		      "Для тебя нет там сейчас места.",
		      "Для вас с %2$^C5 нет там сейчас места." );
	return false;
    }

    return true;
}

bool Walkment::checkLawzone( Character *wch )    
{
    if (!IS_SET(to_room->room_flags, ROOM_LAW))
	return true;
    
    if (!wch->is_npc( ) || !IS_SET(wch->act, ACT_AGGRESSIVE))
	return true;
    
    if (!(wch->master && IS_AFFECTED( wch, AFF_CHARM )))
	return true;
    
    rc = RC_MOVE_LAWZONE;
    msgSelfMaster( wch, 
	           "Ты не можешь отправиться в город.", 
                   "Ты не можешь взять с собой %1$C4 в город." );
    return false; 
}

bool Walkment::tryMove( Character *wch )
{
    return applyMovepoints( wch )
	   && applyWeb( wch )
	   && applyCamouflage( wch );
}

bool Walkment::applyMovepoints( Character *wch )
{
    int move;
    
    if (MOUNTED( wch ))
	return true;

    if (IS_GHOST( wch ))
	return true;

    if (wch->is_npc( ))
	return true;
    
    if (( move = getMoveCost( wch ) ) == 0)
	return true;

    if (wch->move < move) {
	msgSelfParty( wch,
		      "Ты слишком уста%1$Gло|л|ла.",
		      "%2$^C1 слишком уста%2$Gло|л|ла." );
	return false;
    }

    ch->move -= move;
    return true;
}

bool Walkment::checkMovepoints( Character *wch )
{
    if (wch->move < getMoveCost( wch )) {
	msgSelfParty( wch,
	              "У тебя не хватает сил шевелить ногами.",
		      "У %2$C2 не хватает сил шевелить ногами." );
	return false;
    }
    return true;
}


bool Walkment::applyCamouflage( Character *wch )
{
    if (IS_AFFECTED(wch, AFF_CAMOUFLAGE)
	    && to_room->sector_type != SECT_FIELD
	    && to_room->sector_type != SECT_FOREST
	    && to_room->sector_type != SECT_MOUNTAIN
	    && to_room->sector_type != SECT_HILLS)
    {
	strip_camouflage( wch );
    }	

    return true;
}

bool Walkment::applyWeb( Character *wch )
{
    if (MOUNTED(wch))
	return true;

    if (!CAN_DETECT(wch, ADET_WEB))
	return true;

    wch->setWaitViolence( 1 );

    int chance = get_str_app(wch).web;
    if (IS_SET(wch->form, FORM_MIST))
        chance *= 2;

    if (number_percent( ) < chance) {
	affect_strip(wch, gsn_web);
	msgSelfRoom( wch,
		     "Ты разрываешь сети, которые мешали тебе покинуть это место.",
	             "%2$^C1 разрывает сети, которые мешали %2$P3 покинуть это место." );
	return true;
    }
    else {
	rc = RC_MOVE_WEB;
	msgSelfRoom( wch,
		     "Ты пытаешься разорвать сети, которые мешают пройти, но терпишь неудачу.",
	             "%2$^C1 пытается разорвать сети, преграждающие %2$P3 дорогу, но терпит неудачу." );
	return false;
    }
}

void Walkment::moveFollowers( Character *wch )
{
    list<Character *> followers;
    list<Character *>::iterator f;
    Character *fch;

    if (!wch)
	return;

    for (fch = from_room->people; fch != 0; fch = fch->next_in_room) 
	if (fch->master == wch && fch->position == POS_STANDING)
	    followers.push_back( fch );

    for (f = followers.begin( ); f != followers.end( ); f++) 
	if ((*f)->in_room == from_room)
	    moveOneFollower( wch, *f );
}

bool Walkment::canHear( Character *victim, Character *wch )
{
    return !silence && Movement::canHear( victim, wch );
}

