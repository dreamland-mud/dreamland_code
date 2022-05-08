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
    msgRoom( "От голода %2$C1\6себя", dam, ch );
    msgChar( "От голода ты\5себя", dam );
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



