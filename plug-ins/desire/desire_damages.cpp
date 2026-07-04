/* $Id$
 *
 * ruffina, 2004
 */
#include "desire_damages.h"
#include "character.h"

#include "damageflags.h"
#include "merc.h"
#include "def.h"

HungerDamage::HungerDamage( Character *ch, int dam )
    : SelfDamage( ch, DAM_NONE, dam )
{
    deathReason = "hunger";
}

void HungerDamage::message( )
{
    // No \5/\6 damage-verb markers here: those expand to combat verbs from the
    // attack table (СОКРУШАЕШЬ/РАСЧЛЕНЯЕШЬ at high dam), which read absurdly for
    // starvation. Hunger just weakens -- and canDamage() never lets it kill.
    msgRoom( "%2$^C1 слабеет от голода", dam, ch );
    msgChar( "Ты слабеешь от голода", dam );
}

bool HungerDamage::canDamage()
{    
    // Don't die from hunger. 
    return HEALTH(ch) > 2 && SelfDamage::canDamage();
}

ThirstDamage::ThirstDamage( Character *ch, int dam )
    : SelfDamage( ch, DAM_NONE, dam )
{
    deathReason = "thirst";
}

void ThirstDamage::message( )
{
    msgRoom( "От жажды %2$C1\6себя", dam, ch );
    msgChar( "От жажды ты\5себя", dam );
}

bool ThirstDamage::canDamage()
{
    // Don't die from thirst. 
    return HEALTH(ch) > 2 && SelfDamage::canDamage();
}



