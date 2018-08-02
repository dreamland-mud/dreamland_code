/* $Id$
 *
 * ruffina, 2004
 */
#include "transportspell.h"

#include "clanreference.h"
#include "skillreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "interp.h"
#include "save.h"
#include "occupations.h"
#include "fight.h"
#include "magic.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(none);
GSN(mental_block);
GSN(spellbane);

/*------------------------------------------------------------------------
 * GateMovement 
 *-----------------------------------------------------------------------*/
GateMovement::GateMovement( Character *ch, Character *victim, GateSpell::Pointer spell, Character *actor, int level )
               : JumpMovement( ch, actor, victim->in_room )
{
    this->victim = victim;
    this->spell = spell;
    this->level = level;
}

GateMovement::GateMovement( Character *ch, Character *victim )
               : JumpMovement( ch, ch, victim->in_room )
{
    this->victim = victim;
    level = -1;
}

GateMovement::~GateMovement( )
{
}

bool GateMovement::moveAtomic( )
{
    if (!JumpMovement::moveAtomic( )) {
	ch->pecho( "Твоя попытка закончилась неудачей." );
	return false;
    }

    return true;
}

bool GateMovement::canMove( Character *wch )
{
    if (ch != actor)
	return true;
    else if (wch == ch->mount)
	return checkCaster( wch );
    else
	return checkLevel( )
	       && checkCaster( wch )
	       && checkCasterRoom( )
	       && checkVictimRoom( )
	       && checkVictim( );
}

bool GateMovement::tryMove( Character *wch )
{
    if (ch != actor)
	return true;
    else if (wch == ch->mount)
	return applyViolent( wch );
    else
	return applyViolent( wch )
	       && applySavesSpell( );
}

bool GateMovement::checkLevel( )
{
    if (level < 0 || !spell)
	return true;
	
    if (victim->getModifyLevel( ) < level + spell->levelDiff)
	return true;

    return false;
}

bool GateMovement::checkCaster( Character *wch )
{
    if (wch->fighting)
	return false;
	
    if (IS_AFFECTED(wch, AFF_CURSE))
	return false;
    
    if (spell && !spell->gateShadow && HAS_SHADOW( wch ))
	return false;

    if (wch->is_npc( ) && wch == actor && wch->mount)
	return false;

    return true;
}

bool GateMovement::checkCasterRoom( )
{
    if (IS_SET(from_room->room_flags, ROOM_SAFE|ROOM_NO_RECALL|ROOM_NOSUMMON))
	return false;

    if (IS_RAFFECTED(from_room, AFF_ROOM_CURSE))
	return false;

    return true;
}

bool GateMovement::checkVictim( )
{
    if (victim == ch || victim == ch->mount)
	return false;

    if (victim->is_immortal( ) && !ch->is_immortal( )) 
	return false;

    if (DIGGED(victim))
	return false;

    if (victim->is_npc( )) {
	if (IS_SET(victim->getNPC( )->pIndexData->area->area_flag, AREA_NOGATE))
	    return false;

        if (IS_SET(victim->imm_flags, IMM_SUMMON))
	    return false;
	    
	if (IS_AFFECTED(victim, AFF_CHARM))
	    return false;
    } 
    else {
        if (!ch->is_npc( ) 
	    && !ch->getClan( )->isDispersed( ) 
	    && ch->getClan( ) == victim->getClan( ))
	    return true; 
	
        if (!is_safe_nomessage(ch, victim) 
	    && IS_SET(victim->act, PLR_NOSUMMON) 
	    && spell 
	    && from_room->area != to_room->area)
	    return false;
	
	if (is_safe_nomessage(ch, victim) && IS_SET(victim->act, PLR_NOSUMMON))
	    return false;
    }
    
    return true;
}

bool GateMovement::checkVictimRoom( )
{
    if (!ch->can_see( to_room ))
	return false;

    if (IS_SET(to_room->room_flags, 
	       ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY|ROOM_NOSUMMON))
	return false;

    return true;
}

bool GateMovement::applySavesSpell( )
{
    return !saves_spell( ch->getModifyLevel( ), victim, DAM_OTHER, ch, DAMF_SPELL );
}

bool GateMovement::applyViolent( Character *wch )
{
    if (IS_BLOODY( wch )) {
	int chance = 0;

	if (spell && spell->gateViolent) {
	    chance = PK_TIME_VIOLENT - wch->getPC( )->PK_time_v;
	    chance = 100 * chance / PK_TIME_VIOLENT;
	}

	return number_percent( ) < chance;
    }

    return true;
}

void GateMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (!spell)
	return;

    if (fLeaving) {
	if (ch != actor && !spell->msgGroupLeave.empty( ))
	    msgSelf( wch, spell->msgGroupLeave.c_str( ) );
	else {
	    msgSelf( wch, spell->msgSelfLeave.c_str( ) );
	    msgRoomNoParty( wch, spell->msgRoomLeave.c_str( ) );     
	}
    }
    else {
	if (ch != actor && !spell->msgGroupEnter.empty( ))
	    msgSelf( wch, spell->msgGroupEnter.c_str( ) );
	else {
	    msgSelf( wch, spell->msgSelfEnter.c_str( ) );
	    msgRoomNoParty( wch, spell->msgRoomEnter.c_str( ) );
	}
    }
}

void GateMovement::moveFollowers( Character *wch )
{
    if (!wch)
	return;

    if (!spell)
	return;
    
    if (from_room == to_room)
	return;

    if (spell->takePet) {
	NPCharacter *pet;

	if (!wch->is_npc( ) && ( pet = wch->getPC( )->pet ))
	    GateMovement( pet, victim, spell, actor, level ).moveRecursive( );
    }
	
    if (spell->takeGroup) {
	GroupMembers group = group_members_room( ch, from_room );

	for (GroupMembers::iterator g = group.begin( ); g != group.end( ); g++)
	    if ((*g)->in_room == from_room)
		GateMovement( *g, victim, spell, actor, level ).moveRecursive( );
    }
}

void GateMovement::msgEcho( Character *listener, Character *wch, const char *msg )
{
    if (IS_AWAKE(listener))
	listener->pecho( msg, wch, victim, actor );
}

/*------------------------------------------------------------------------
 * GateSpell
 *-----------------------------------------------------------------------*/
void GateSpell::run( Character *ch, Character *victim, int sn, int level )
{
    GateMovement( ch, victim, Pointer( this ), ch, level ).move( );
}



/*------------------------------------------------------------------------
 * SummonMovement 
 *-----------------------------------------------------------------------*/
SummonMovement::SummonMovement( Character *ch, Character *caster, SummonSpell::Pointer spell, int level )
                  : JumpMovement( ch, ch, caster->in_room )
{
    this->caster = caster;
    this->spell = spell;
    this->level = level;
}

SummonMovement::~SummonMovement( )
{
}

bool SummonMovement::moveAtomic( )
{
    if (!JumpMovement::moveAtomic( )) {
	caster->pecho( "Твоя попытка закончилась неудачей." );
	return false;
    }

    return true;
}

bool SummonMovement::canMove( Character *wch )
{
    if (wch == ch->mount)
	return true;
	
    return checkCaster( ) 
           && checkVictim( );
}

bool SummonMovement::tryMove( Character *wch )
{
    if (wch == ch->mount)
	return true;

    return applySavesSpell( ) 
           && applySpellbane( )
	   && applyLazy( );
}

bool SummonMovement::applyLazy( )
{
    if (caster->in_room->area == ch->in_room->area) {
	caster->pecho( "Может, пора пройтись пешком?" );
	return false;
    }

    return true;
}

bool SummonMovement::checkCaster( )
{
    if (caster->is_npc( ) && IS_AFFECTED(caster, AFF_CHARM))
	return false;

    if (IS_VIOLENT( caster ))
	return false;

    if (IS_SET(caster->in_room->room_flags, ROOM_SAFE|ROOM_NOSUMMON))
	return false;
    
    if (caster->in_room->getCapacity( ) == 0)
	return false;

    return true;
}    

bool SummonMovement::checkVictim( )
{
    if (ch == caster)
	return false;
    
    if (ch->is_immortal( ) && !caster->is_immortal( ))
	return false;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE|ROOM_NOSUMMON))
	return false;

    if (ch->fighting != 0)
	return false;

    if (ch->isAffected( gsn_mental_block ))
	return false;

    if (ch->isAffected( gsn_spellbane ))
	return false;

    if (ch->is_npc( )) {
	if (IS_SET(ch->act,ACT_AGGRESSIVE))
	    return false;
	    
	if (IS_SET(ch->imm_flags,IMM_SUMMON))
	    return false;
	
	if (ch->getNPC( )->behavior
	    && IS_SET(ch->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
	    return false;

	if (is_safe_nomessage( caster, ch ))
	    return false;
    }
    else {
	if (!IS_SET(ch->act,PLR_NOSUMMON))
	    return true;
	
	if (!is_safe_nomessage( caster, ch ))
	    return true;
	
	if (!caster->is_npc( )) {
	    if (ch->getClan( )->isDispersed( ))
		return false;

	    if (caster->getClan( ) != ch->getClan( ))
		return false;
	}
    }

    return true;
}

bool SummonMovement::applySavesSpell( )
{
    return !saves_spell( level, ch, DAM_OTHER, caster, DAMF_SPELL );
}

bool SummonMovement::applySpellbane( )
{
    return !spell->spellbane( caster, ch );
}

void SummonMovement::msgEcho( Character *listener, Character *wch, const char *msg )
{
    if (IS_AWAKE(listener))
	listener->pecho( msg, 
			   (RIDDEN(wch) ? wch->mount : wch),
			   (MOUNTED(wch) ? wch->mount : wch),
			   caster );
}

void SummonMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (fLeaving) {
	msgRoomNoParty( wch, "%1$^C1 внезапно исчезает." );     
	msgSelfParty( wch, 
	              "%3$^C1 призывает тебя!",
		      "%3$^C1 призывает %2$^C4!" );
    }
    else {
	msgRoomNoParty( wch, "%1$^C1 появляется рядом с тобой." );
    }
}

/*------------------------------------------------------------------------
 * SummonSpell
 *-----------------------------------------------------------------------*/
void SummonSpell::run( Character *ch, Character *victim, int sn, int level )
{
    SummonMovement( victim, ch, Pointer( this ), level ).move( );
}

