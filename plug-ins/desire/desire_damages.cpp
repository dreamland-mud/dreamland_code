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
}

void HungerDamage::message( )
{
    msgRoom( "От голода %2$C1\6себя", dam, ch );
    msgChar( "От голода ты\5себя", dam );
}

ThirstDamage::ThirstDamage( Character *ch, int dam )
    : SelfDamage( ch, DAM_NONE, dam )
{
}

void ThirstDamage::message( )
{
    msgRoom( "От жажды %2$C1\6себя", dam, ch );
    msgChar( "От жажды ты\5себя", dam );
}


