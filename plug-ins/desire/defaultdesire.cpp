/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultdesire.h"
#include "pcharacter.h"
#include "liquid.h"
#include "room.h"

#include "act.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define HUNGER_MIN_LEVEL (PK_MIN_LEVEL + 5)

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

// Helper function to show non-empty messages to the character.
static void ptc( PCharacter *ch, const DLString &msg )
{
    if (!msg.empty( ))
        ch->println( msg );
}

void DefaultDesire::gain( PCharacter *ch, int value )
{
    int oldDesire, desire;
    bool wasActive;
    bool isNewbie = ch->getRealLevel( ) <  HUNGER_MIN_LEVEL;
    
    if (!applicable( ch ) || ch->is_immortal( )) {
        reset( ch );
        return;
    }

    if (value == 0)
        return;

    if (IS_GHOST( ch ))
        return;

    wasActive = isActive( ch );
    oldDesire = ch->desires[getIndex( )];
    desire = URANGE( (int)minValue, oldDesire + value, (int)maxValue );

    // Newbies don't become too hungry.
    if (isNewbie)
        desire = std::max( 10, desire );

    ch->desires[getIndex( )] = desire;

    // Too hungry or thirsty, inflict some damage.
    if (desire == damageLimit && !isNewbie) {
        if (!msgDamageSelf.empty( ))
            ch->pecho( msgDamageSelf.c_str( ) );
        if (!msgDamageRoom.empty( ))
            ch->recho( msgDamageRoom.c_str( ), ch );
        damage( ch );
        return;
    }

    // Was hungry but now satisfied, print stop message.
    if (wasActive != isActive( ch ) && wasActive) {
        ptc( ch, msgStop );
        return;
    }
    
    // Feel hunger for the first time, print start message.
    if (wasActive != isActive( ch )) {
        ptc( ch, msgStart );
        return;
    }

    // Still feeling hungry, report about it.
    if (isActive( ch )) {
        ptc( ch, msgActive );
        return;
    }
}

void DefaultDesire::damage( PCharacter * )
{
}

bool DefaultDesire::isVampire( PCharacter *ch )
{
    return ch->getSkillData(gsn_vampire).learned >= 100;
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

