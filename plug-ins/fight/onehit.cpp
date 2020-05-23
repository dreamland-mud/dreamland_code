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

#include "raceflags.h"
#include "magic.h"
#include "fight.h"
#include "../anatolia/handler.h"
#include "stats_apply.h"
#include "gsn_plugin.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"
#include "debug_utils.h"
#include "math_utils.h"


GSN(shadow_shroud);
GSN(soul_lust);
RACE(hobbit);

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
    mprog_hit();
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
    Debug d(ch, "debug_onehit", "diceroll");
    int dice;

    dice = number_range( 0, 19 );
    d.log(thac0, "thac0");
    d.log(victim_ac, "victim_ac");

    // critical miss
    // lucky little hobbits do not miss critically. for them, critical miss is treated as a critical hit instead
    if(dice == 0){
        if (ch->getRace() == race_hobbit)
            return true;
        else
            return false;
    }

    // critical hit
    if(dice == 19)
        return true;

    d.log(dice, "dice");
    d.log(thac0-victim_ac, "thac0 - victim_ac");
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
#define OLD_THAC 1

void OneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
#if OLD_THAC  
    thacApplySkill( );
#endif    
}

void OneHit::thacBase( )
{
    Debug d(ch, "debug_onehit", "thacBase");
    int thac0_00, thac0_32;

    thac0_00 = ch->getProfession( )->getThac00( ch );
    thac0_32 = ch->getProfession( )->getThac32( ch );

#if OLD_THAC
    thac0  = interpolate( ch->getModifyLevel(), thac0_00, thac0_32 );
#else    
    thac0 = linear_interpolation(ch->getModifyLevel(), 1, 103, thac0_00, thac0_32);
#endif    
    d.log(thac0_00, "thac0_00");
    d.log(thac0_32, "thac0_32");
    d.log(thac0, "thac0");

#if OLD_THAC
    if (thac0 < 0) {
        thac0 = thac0 / 2;
        d.log(thac0, "thac0/2");
    }

    if (thac0 < -5) {
        thac0 = -5 + ( thac0 + 5 ) / 2;
        d.log(thac0, "thac0<-5");
    }
#endif    
}

void OneHit::thacApplyHitroll( )
{
    Debug d(ch, "debug_onehit", "thacHitroll");

#if OLD_THAC
    int hr;

    hr = ch->hitroll - ch->getRealLevel( ) / 2;
    hr += hr * get_str_app(ch).hit / 100;

    if (IS_GOOD(victim) && ch->isAffected( gsn_soul_lust ))
        hr += 1 + hr / 100;

    thac0 -= hr * skill/100;

    d.log(hr, "hr");
    d.log(skill, "skill");
    d.log(hr*skill/100, "sub");
    d.log(thac0, "thac0");
#else    
    int hr, mod;

    hr = ch->hitroll;
    hr += hr * get_str_app(ch).hit / 100;

    if (IS_GOOD(victim) && ch->isAffected( gsn_soul_lust ))
        hr += 1 + hr / 100;

    mod = hr * skill / 100;
    mod = mod / ch->getModifyLevel();
    mod = min(mod, 3);

    thac0 -= mod;

    d.log(hr, "hr");
    d.log(mod, "mod");
    d.log(thac0, "thac0");
#endif     
}

void OneHit::thacApplySkill( )
{
    Debug d(ch, "debug_onehit", "thacSkill");
    thac0 += 5 * (100 - skill) / 100;
    d.log(skill, "skill");
    d.log(5 * (100 - skill) / 100, "plus");
    d.log(thac0, "thac0");
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
    Debug d(ch, "debug_onehit", "acBase");

    switch ( dam_type )
    {
        case(DAM_PIERCE):   victim_ac = GET_AC(victim,AC_PIERCE)/10;    break;
        case(DAM_BASH):     victim_ac = GET_AC(victim,AC_BASH)/10;        break;
        case(DAM_SLASH):    victim_ac = GET_AC(victim,AC_SLASH)/10;        break;
        default:            victim_ac = GET_AC(victim,AC_EXOTIC)/10;    break;
    };
    
    d.log(victim_ac, "GET_AC");
    if (victim_ac < -20) {
        victim_ac = ( ( victim_ac + 20 ) * 2 ) / 3 - 20;
        d.log(victim_ac, "ac<-20");        
    }
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

