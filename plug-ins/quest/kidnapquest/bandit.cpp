/* $Id: bandit.cpp,v 1.1.2.16.6.4 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include <math.h>

#include <list>
#include "class.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "dlscheduler.h"

#include "clan.h"
#include "fight.h"
#include "interp.h"
#include "act.h"

#include "kidnapquest.h"
#include "scenario.h"
#include "bandit.h"
#include "prince.h"

KidnapBandit::KidnapBandit( ) : state( BSTAT_HUNT_PRINCE ), prince( 0 )
{
}

bool KidnapBandit::spec_hunt_prince( ) 
{
    PCharacter *hero;

    if (!IS_AWAKE( ch )) {
	state = BSTAT_SLEEP;
	return false;
    }

    if (ch->fighting) {
	state = BSTAT_FIGHT;
	return false;
    }
    
    clearLastFought( );
    memoryFought.clear( );
    
    if (!getPrince( ))
	return true;
    
    if (princeHere( )) {
	princeAttach( );
	return false;
    }

    if ((hero = getHeroRoom( )) && IS_AWAKE( hero )) {
	quest->getScenario( ).actAttackHero( ch, hero );

	if (heroAttack( hero )) 
	    return false;
    }
   
    debug( "Буду охотиться на принца." );
    princeHunt( );

    return true;
}

bool KidnapBandit::spec_kidnap( ) 
{

    if (!IS_AWAKE( ch )) {
	princeDetach( );
	state = BSTAT_SLEEP;
	return false;
    }

    if (ch->fighting) {
	princeDetach( );
	state = BSTAT_FIGHT;
	return false;
    }
    
    clearLastFought( );
    memoryFought.clear( );
    
    if (!princeHere( )) {
	princeDetach( );
	state = BSTAT_HUNT_PRINCE;
	return false;
    }
    
    princeKidnap( );
    return true;
}

bool KidnapBandit::spec_fight( ) 
{
    if (!IS_AWAKE( ch )) {
	state = BSTAT_SLEEP;
	return false;
    }

    if (!ch->fighting) {
	state = BSTAT_HUNT_PRINCE;
	return false;
    }

    return true;
}
    
bool KidnapBandit::spec_sleep( ) 
{
    if (IS_AWAKE( ch )) {
	if (ch->fighting)
	    state = BSTAT_FIGHT;
	else
	    state = BSTAT_HUNT_PRINCE;

	return false;
    }

    return true;
}

bool KidnapBandit::spec( ) 
{
    if (!getQuest( ))
	return false;

    while (true)
	switch (state.getValue( )) {
	case BSTAT_HUNT_PRINCE: 
	    if (spec_hunt_prince( ))
		return true;
	    
	    break;
	case BSTAT_KIDNAP:      
	    if (spec_kidnap( ))
		return true;
	    
	    break;
	case BSTAT_SLEEP:       
	    if (spec_sleep( ))
		return true;
	    
	    break;
	case BSTAT_FIGHT:	
	    if (spec_fight( ))
		return true;

	    break;
	}

    return true;
}

/*
 * mobprogs
 */
void KidnapBandit::fight( Character *victim ) 
{
    debug( "Я сражаюсь." );

    if (state == BSTAT_KIDNAP)
	princeDetach( );

    state = BSTAT_FIGHT;

    KidnapMobile::fight( victim );
}

void KidnapBandit::greet( Character *victim ) 
{
    switch (state.getValue( )) {
    case BSTAT_HUNT_PRINCE: 
	if (ourHero( victim ) && number_percent( ) < 50) /* little surprise */
	    interpret_fmt( ch, "blackjack %s", victim->getNameP( ) );
	
	break;
    default:
	break;
    }
}

bool KidnapBandit::extractNotify( Character *ch, bool fTotal, bool fCount )
{
    if (ch == prince)
	prince = NULL;

    return KidnapMobile::extractNotify( ch, fTotal, fCount );
}

/*
 * routines for prince 
 */
bool KidnapBandit::ourPrince( Character *mob )
{
    return getQuest( ) && quest->check<KidnapPrince>( mob );
}

NPCharacter * KidnapBandit::getPrince( )
{
    if (!prince) 
	prince = getPrinceWorld( );
    
    return prince;
}

NPCharacter * KidnapBandit::getPrinceWorld( )
{
    if (getQuest( ))
	return quest->getMobileWorld<KidnapPrince>( );
    else
	return NULL;
}

bool KidnapBandit::princeHere( )
{
    return getPrince( ) && (prince->in_room == ch->in_room);
}

void KidnapBandit::princeAttach( ) 
{
    KidnapPrince::Pointer behavior;
    
    behavior = prince->behavior.getDynamicPointer<KidnapPrince>( );
    behavior->state = STAT_KIDNAPPED;

    if (prince->master)
	prince->stop_follower( );

    prince->add_follower( ch );

    state = BSTAT_KIDNAP;
    
    if (getQuest( ))
	quest->getScenario( ).actBeginKidnap( ch, prince );
}

void KidnapBandit::princeDetach( ) 
{
    KidnapPrince::Pointer behavior;
    
    if (!getPrince( ))
	return;
    
    behavior = prince->behavior.getDynamicPointer<KidnapPrince>( );
    behavior->state = STAT_LOST;

    if (prince->master)
	prince->stop_follower( );
}

/*
 *
 */
void KidnapBandit::config( PCharacter *hero )
{
    int ave;
    int level = hero->getModifyLevel( );

    ch->setLevel( level );
    ch->hitroll = level * 2;
    ch->damroll = number_fuzzy( level / 2 );
    ch->max_mana = ch->mana = level * 10;
    ch->max_move = ch->move = 1000;
    ch->max_hit = (1 + level / 30) * level * 10;
    ch->hit = ch->max_hit;
    ch->armor[0] = -level * 5;
    ch->armor[1] = -number_fuzzy(level) * 6;
    ch->armor[2] = -number_fuzzy(level) * 6;
    ch->armor[3] = -number_fuzzy(level) * 6;
    ch->saving_throw = -level / 2;
    
    ave = number_range( level / 3, level / 2 );
    ch->damage[DICE_NUMBER] = (int) ::sqrt( 2 * ave );
    ch->damage[DICE_TYPE]   = ch->damage[DICE_NUMBER];
}

/*
 * routines for hero
 */
bool KidnapBandit::heroAttack( PCharacter *hero )
{
    if (!hero) {
	if (!( hero = getHeroRoom( ) ))
	    return false;

	if (!IS_AWAKE( hero ))
	    return false;
    }
    
    debug( "Я вижу неспящего героя - ща как дам в дыню!" );
    multi_hit( ch, hero );
    
    if (!ch->fighting) {
	debug( "Все это очень хорошо, но сражение не началось!" );
	return false;
    }
    
    state = BSTAT_FIGHT;
    return true;
}

