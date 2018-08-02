/* $Id: onehit.cpp,v 1.1.2.6 2010/01/01 15:48:15 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "logstream.h"

#include "onehit.h"

#include "skill.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "room.h"
#include "mobilebehavior.h"

#include "raceflags.h"
#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "effects.h"
#include "interp.h"
#include "stats_apply.h"
#include "material.h"
#include "gsn_plugin.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"


GSN(shadow_shroud);
GSN(soul_lust);

/*-----------------------------------------------------------------------------
 * One Hit (hit one guy once)
 *----------------------------------------------------------------------------*/
OneHit::OneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0, DAMF_WEAPON ) 
{
    skill = 0;
    thac0 = 0;
    victim_ac = 0;
    orig_dam = 0;
}

void OneHit::hit( )
{
    if (!canHit( ))
	return;
    
    ch->move -= move_dec( ch );

    init( );
    
    calcTHAC0( );
    calcArmorClass( );

    if (!diceroll( )) {
	miss( );
	return;
    }
    
    if (checkShadow( ))
	return;
	
    calcDamage( );
   
    if (!canDamage( ))
	return;

    priorDamageEffects( );
    message( );

    if (dam == 0)
	return;

    inflictDamage( );
    postDamageEffects( );
    handlePosition( );
    checkRetreat( );
}

void OneHit::priorDamageEffects( )
{
}

void OneHit::postDamageEffects( )
{
}

void OneHit::damNormalize( )
{
    dam = std::max( dam, 1 );
    orig_dam = dam;
}

void OneHit::protectShadowShroud( )
{
    if (IS_GOOD(ch) 
	&& victim->isAffected( gsn_shadow_shroud ))
    {
	dam -= dam * victim->getModifyLevel( ) / 1200;
    }
}

void OneHit::calcDamage( )
{
    damNormalize( );
    Damage::calcDamage( );
    protectShadowShroud( );
}

void OneHit::miss( )
{
    dam = 0;

    if (canDamage( ))
	message( );
}


/*----------------------------------------------------------------------------
 * The moment of excitement!
 *---------------------------------------------------------------------------*/
bool OneHit::diceroll()
{
    int dice;

    dice = number_range( 0, 19 );

    // critical miss
    if(dice == 0)
	return false;
    
    // critical hit
    if(dice == 19)
	return true;
	
    return dice >= thac0 - victim_ac;
}

bool OneHit::checkShadow( )
{
    if (SHADOW(ch)) {
	msgFightChar( "Ты со всей дури лупишь свою тень." );
	msgFightVict( "%1$^C1 со всей дури лупит свою тень." );
	msgFightRoom( "%1$^C1 со всей дури лупит свою тень." );
	return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
 * 
 *---------------------------------------------------------------------------*/
bool OneHit::canHit( )
{
    // just in case
    if ( ch == 0 || victim == 0 || victim == ch )
	return false;

    // ghosts can't fight
    if ( ( !victim->is_npc() && IS_GHOST( victim ) )
	|| ( !ch->is_npc() && IS_GHOST( ch ) ) )
	return false;
    
    //
    // Can't beat a dead char!
    // Guard against weird room-leavings.
    //
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return false;

    if ( !ch->is_npc() && !ch->move )
	return false;

    return true;
}

/*----------------------------------------------------------------------------
 * Calculate to-hit-armor-class-0 versus armor.
 *---------------------------------------------------------------------------*/
void OneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
}

void OneHit::thacBase( )
{
    int thac0_00, thac0_32;

    thac0_00 = ch->getTrueProfession( )->getThac00( ch );
    thac0_32 = ch->getTrueProfession( )->getThac32( ch );

    thac0  = interpolate( ch->getModifyLevel(), thac0_00, thac0_32 );

    if ( thac0 < 0 )
	thac0 = thac0 / 2;

    if ( thac0 < -5 )
	thac0 = -5 + ( thac0 + 5 ) / 2;
}

void OneHit::thacApplyHitroll( )
{
    int hr;

    hr = ch->hitroll - ch->getRealLevel( ) / 2;
    hr += hr * get_str_app(ch).hit / 100;

    if (IS_GOOD(victim) && ch->isAffected( gsn_soul_lust ))
	hr += 1 + hr / 100;

    thac0 -= hr * skill/100;
}

void OneHit::thacApplySkill( )
{
    thac0 += 5 * (100 - skill) / 100;
}


/*----------------------------------------------------------------------------
 * Calculate victim armor class
 *---------------------------------------------------------------------------*/
void OneHit::calcArmorClass( )
{
    acBase( );
    acApplyArmorUse( );
    acApplyBlindFighting( );
    acApplyPosition( );
}

void OneHit::acBase( )
{
    switch ( dam_type )
    {
	case(DAM_PIERCE):   victim_ac = GET_AC(victim,AC_PIERCE)/10;    break;
	case(DAM_BASH):	    victim_ac = GET_AC(victim,AC_BASH)/10;	break;
	case(DAM_SLASH):    victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	    victim_ac = GET_AC(victim,AC_EXOTIC)/10;    break;
    };
    
    if ( victim_ac < -20 )
	victim_ac = ( ( victim_ac + 20 ) * 2 ) / 3 - 20;
}

void OneHit::acApplyArmorUse( )
{
    int sk_armor_use = gsn_armor_use->getEffective( victim );

    if ( sk_armor_use > 1 )
    {
	if ( number_percent() < sk_armor_use )
	{
	    gsn_armor_use->improve( victim, true, ch );
	    victim_ac -= ( victim->getModifyLevel() ) / 2;
	}
	else
	{
	    gsn_armor_use->improve( victim, false, ch );
	}
    }

}

void OneHit::acApplyBlindFighting( )
{
    if ( !ch->can_see( victim ) )
    {
	if (number_percent() < gsn_blind_fighting->getEffective( ch ))
	{
	    gsn_blind_fighting->improve( ch, true, victim );
	}
	else
	    victim_ac -= 4;
    }
}

void OneHit::acApplyPosition( )
{
    if ( victim->position < POS_FIGHTING )
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    if ( !victim->move )
	victim_ac += 10;
}

/*-----------------------------------------------------------------------------
 * Damage modifiers 
 *----------------------------------------------------------------------------*/
void OneHit::damApplyPosition( )
{
    if ( !IS_AWAKE(victim) )
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;
}

void OneHit::damApplyDamroll( )
{
    dam += ch->damroll * min(100, skill) / 100;
    dam += dam * get_str_app(ch).damage / 100;
}

void OneHit::damApplyAttitude( )
{
    Flags att = ch->getRace( )->getAttitude( *victim->getRace( ) );

    if (att.isSet( RACE_HATES ))
	dam = dam * 120 / 100;
}

/*----------------------------------------------------------------------------
 * message output with spam-control
 *----------------------------------------------------------------------------*/
void OneHit::msgOutput( Character *wch, const char *msg )
{
    wch->pecho( msg, ch, victim );
}

void OneHit::msgEchoNoSpam( Character *wch, const char *msg, int bit, bool fInvert )
{
    if (!wch->is_npc( )) 
	if (fInvert == !!IS_SET( wch->getPC( )->config, bit ))
	    return;
    
    if (wch->position < POS_RESTING)
	return;
    
    msgOutput( wch, msg );
}

void OneHit::msgWeaponVict( const char *msg )
{
    msgEchoNoSpam( victim, msg, CONFIG_WEAPONSPAM, true );
}

void OneHit::msgWeaponChar( const char *msg )
{
    msgEchoNoSpam( ch, msg, CONFIG_WEAPONSPAM, true );
}

void OneHit::msgWeaponRoom( const char *msg )
{
    Character *wch;

    for (wch = victim->in_room->people; wch; wch = wch->next_in_room) 
	if (wch != victim && wch != ch)
	    msgEchoNoSpam( wch, msg, CONFIG_WEAPONSPAM, true );
}

void OneHit::msgFightVict( const char *msg )
{
    msgEchoNoSpam( victim, msg, CONFIG_FIGHTSPAM, false );
}

void OneHit::msgFightChar( const char *msg )
{
    msgEchoNoSpam( ch, msg, CONFIG_FIGHTSPAM, false );
}

void OneHit::msgFightRoom( const char *msg )
{
    Character *wch;

    for (wch = victim->in_room->people; wch; wch = wch->next_in_room) 
	if (wch != victim && wch != ch)
	    msgEchoNoSpam( wch, msg, CONFIG_FIGHTSPAM, false );
}

