/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultdesire.h"
#include "pcharacter.h"
#include "liquid.h"
#include "room.h"
#include "skillreference.h"

#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define HUNGER_MIN_LEVEL (PK_MIN_LEVEL + 5)

GSN(vampire);

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

/** Display current desire status as 0..10 coloured dots. */
DLString DefaultDesire::showDots(PCharacter *ch) const
{
    ostringstream dots;
    int maxDots = 10;
    int current = ch->desires[getIndex( )];
    int progress = current * maxDots / maxValue;
    progress = URANGE(1, progress, maxDots);

    char color = progress <= 4 ? 'R' : progress <= 7 ? 'Y' : 'G';
    dots << "{" << color;

    int i;
    for (i = 1; i <= progress; i++)
        dots << "*";
    for (; i <= maxDots; i++)
        dots << " ";

    dots << "{x";

    return dots.str();
}

/** Display current desire status as X%. */
DLString DefaultDesire::showPercent(PCharacter *ch) const
{
    ostringstream buf;
    int current = ch->desires[getIndex( )];
    int progress = current * 100 / maxValue;
    progress = URANGE(1, progress, 100);

    char color = progress <= 40 ? 'R' : progress <= 70 ? 'Y' : 'G';

    buf << "{" << color << progress << "%{w";
    return buf.str();
}

void DefaultDesire::report( PCharacter *ch, ostringstream &buf )
{
    if (ch->getRealLevel( ) <  HUNGER_MIN_LEVEL)
        return;

    if (ch->is_immortal())
        return;

    if (!applicable(ch))
        return;
        
    if (msgReport.empty())
        return;

    // Show simple text message when the desire is activated, e.g. "You're hungry."
    if (isActive(ch)) {
        buf << fmt( NULL, msgReport.c_str( ), ch );
        return;
    }

    if (what.empty())
        return;

    // Draw player desire status as progress bar or percents.
    buf << what << " ";

    if (IS_SET(ch->config, CONFIG_SCREENREADER))
        buf << showPercent(ch);
    else
        buf << "[" << showDots(ch) << "]";

    buf << ".";
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
        ch->pecho( msg );
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

    if (!ch->desc)
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

