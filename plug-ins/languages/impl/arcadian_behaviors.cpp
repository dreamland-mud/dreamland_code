/* $Id$
 *
 * ruffina, 2009
 */
#include "arcadian_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "race.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "act.h"
#include "loadsave.h"
#include "effects.h"
#include "fight_extract.h"
#include "vnum.h"
#include "def.h"

GSN(arcadian);
GSN(gas_breath);

/*
 * ArcadianDrinkBehavior
 */
void ArcadianDrinkBehavior::setEffectName( const WordEffect::Pointer effect )
{
    Language::Pointer lang;

    if (!( lang = languageManager->findLanguage( gsn_arcadian->getName( ) ) ))
        return;

    effectName = lang->getEffectName( effect );
}

void ArcadianDrinkBehavior::setQuality( PCharacter *ch )
{
    quality = gsn_arcadian->getEffective( ch );
}

DrinkContainerWEBase::Pointer ArcadianDrinkBehavior::findDrinkEffect( ) const
{
    Language::Pointer lang;
    WordEffect::Pointer effect;
    DrinkContainerWEBase::Pointer drinkEffect;

    if (!( lang = languageManager->findLanguage( gsn_arcadian->getName( ) ) ))
        return drinkEffect;

    if (!( effect = lang->findEffect( effectName.getValue( ) ) ))
        return drinkEffect;

    drinkEffect = effect.getDynamicPointer<DrinkContainerWEBase>( );
    return drinkEffect;
}

bool ArcadianDrinkBehavior::hasSameEffect( Object *source )
{
    ArcadianDrinkBehavior::Pointer bhv;
    
    if (source->behavior)
        if (( bhv = source->behavior.getDynamicPointer<ArcadianDrinkBehavior>( ) ))
            if (bhv->effectName.getValue( ) == effectName.getValue( ))
                return true;

    return false;
}

void ArcadianDrinkBehavior::pourOut( Character *ch, int amount )
{
    DrinkContainerWEBase::Pointer drinkEffect = findDrinkEffect( );
    
    if (!drinkEffect || !isActive( )) 
        return;
    
    drinkEffect->onPourOut( this, ch, amount );
    cleanup( );
}

void ArcadianDrinkBehavior::pour( Character *ch, Object *destination, int amount )
{
    if (obj->value1() == 0)
        cleanup( );
}

void ArcadianDrinkBehavior::fill( Character *ch, Object *source, int amount )
{
    if (!source || !hasSameEffect( source ))
        cleanup( );
}

void ArcadianDrinkBehavior::pourOut( Character *ch, Character *victim, int amount )
{
    DrinkContainerWEBase::Pointer drinkEffect = findDrinkEffect( );
    
    if (!drinkEffect || !isActive( )) 
        return;
    
    drinkEffect->onPourOut( this, ch, victim, amount );
    cleanup( );
}

void ArcadianDrinkBehavior::drink( Character *ch, int amount )
{
    DrinkContainerWEBase::Pointer drinkEffect = findDrinkEffect( );
    
    if (!drinkEffect || !isActive( ))
        return;
    
    drinkEffect->onDrink( this, ch, amount );

    if (obj->value1() == 0)
        cleanup( );
}

void ArcadianDrinkBehavior::cleanup( )
{
    effectName = "";
}




/*
 * BeerElementalBehavior
 */
bool BeerElementalBehavior::specFight( ) 
{
    int dam, level;
    Character *victim = ch->fighting;
    
    if (number_bits( 3 ))
        return BasicMobileDestiny::specFight( );
    
    level = ch->getRealLevel( );
    dam = max( 16, (int)ch->hit );
    dam = number_range( dam / 15 + 1, dam / 8 );
    dam = max( dam      + dice( level, 12 ) / 10, 
               dam / 10 + dice( level, 12 ) );
    
    oldact("$c1 дышит на тебя перегаром!", ch, 0, victim, TO_VICT);
    oldact("$c1 дышит перегаром на $C4!", ch, 0, victim, TO_NOTVICT);
    
    try {
        damage_nocatch( ch, victim, dam, gsn_gas_breath, DAM_POISON, true, DAMF_SPELL);
        poison_effect( victim, ch, level, dam, TARGET_CHAR );
    } catch (const VictimDeathException &) {
    }

    return true;
}

bool BeerElementalBehavior::area( )
{                               
    ch->max_hit -= ch->max_hit / 100;

    if (ch->max_hit < ch->getModifyLevel( )) {
        oldact("$c1 окончательно усыхает и исчезает.", ch, 0, 0, TO_ROOM );
        extract_char( ch, true );
        return true;
    }

    return BasicMobileDestiny::area( );
}

