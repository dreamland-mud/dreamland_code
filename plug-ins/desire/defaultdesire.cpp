/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultdesire.h"
#include "pcharacter.h"
#include "liquid.h"
#include "room.h"

#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

PROF(vampire);

/*-------------------------------------------------------------------
 * DefaultDesire
 *------------------------------------------------------------------*/
DefaultDesire::DefaultDesire( )
                 : drinkCoef( 1 )
{
}

DefaultDesire::~DefaultDesire( )
{
}

void DefaultDesire::loaded( )
{
    desireManager->registrate( Pointer( this ) );
}

void DefaultDesire::unloaded( )
{
    desireManager->unregistrate( Pointer( this ) );
}

/*-------------------------------------------------------------------
 * actions
 *------------------------------------------------------------------*/
bool DefaultDesire::canDrink( PCharacter *ch )
{
    return true;
}

bool DefaultDesire::canEat( PCharacter *ch )
{
    return true;
}

void DefaultDesire::report( PCharacter *ch, ostringstream &buf )
{
    if (isActive( ch ) && !msgReport.empty( ))
	buf << fmt( NULL, msgReport.c_str( ), ch );
}

bool DefaultDesire::isActive( PCharacter *ch )
{
    return applicable( ch ) && ch->desires[getIndex( )] <= activeLimit;
}

void DefaultDesire::drink( PCharacter *ch, int amount, Liquid *liq )
{
    if (applicable( ch )) 
	gain( ch, amount * liq->getDesires( )[getIndex( )] / drinkCoef );
}

void DefaultDesire::eat( PCharacter *ch, int amount )
{
    if (applicable( ch ))
	gain( ch, amount );
}


void DefaultDesire::vomit( PCharacter *ch )
{
    if (applicable( ch ))
	ch->desires[getIndex( )] = vomitAmount;
}

void DefaultDesire::update( PCharacter *ch )
{
    if (applicable( ch ))
	gain( ch, getUpdateAmount( ch ) );
}

void DefaultDesire::gain( PCharacter *ch, int value )
{
    int oldDesire, desire;
    bool wasActive;
    
    if (!applicable( ch ) || ch->is_immortal( )) {
	reset( ch );
	return;
    }

    if (value == 0)
	return;

    if (IS_GHOST( ch ))
	return;

    if (IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY) || ch->getRealLevel( ) <= PK_MIN_LEVEL) {
	desire = maxValue;
        ch->desires[getIndex( )] = desire;
	return;
    }
    
    wasActive = isActive( ch );
    oldDesire = ch->desires[getIndex( )];
    desire = URANGE( (int)minValue, oldDesire + value, (int)maxValue );
    ch->desires[getIndex( )] = desire;
    
    if (desire == damageLimit && ch->getRealLevel( ) >= PK_MIN_LEVEL) {
	if (!msgDamageSelf.empty( ))
	    ch->pecho( msgDamageSelf.c_str( ) );
	if (!msgDamageRoom.empty( ))
	    ch->recho( msgDamageRoom.c_str( ), ch );
	damage( ch );
	return;
    }

    if (wasActive != isActive( ch )) {
	if (wasActive) {
	    if (!msgStop.empty( ))
		ch->println( msgStop );
	} else if (!msgStart.empty( ))
	    ch->println( msgStart );
	return;
    }

    if (isActive( ch )) {
	if (!msgActive.empty( ))
	    ch->println( msgActive );
	return;
    }
}

void DefaultDesire::damage( PCharacter * )
{
}

bool DefaultDesire::isVampire( PCharacter *ch )
{
    return ch->getProfession( ) == prof_vampire && ch->getRealLevel( ) >= 10;
}

bool DefaultDesire::applicable( PCharacter * )
{
    return true;
}

void DefaultDesire::reset( PCharacter *ch )
{
    ch->desires[getIndex( )] = resetAmount; 
}

int DefaultDesire::getUpdateAmount( PCharacter * )
{
    return 0;
}

bool DefaultDesire::isOverflow( PCharacter * )
{
    return false;
}

