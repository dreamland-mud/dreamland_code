/* $Id$
 *
 * ruffina, 2004
 */
#include "objects.h"

#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"


/*---------------------------------------------------------------------
 * HorseHarness
 *--------------------------------------------------------------------*/
HorseHarness::HorseHarness( )
{
}

Rideable::Pointer HorseHarness::findHorse( Character *ch, const DLString &arg )
{
    Rideable::Pointer null;
    Character *rch;

    if (!( rch = get_char_room( ch, arg ) ))
	return null;

    if (!rch->is_npc( ))
	return null;

    if (!rch->getNPC( )->behavior)
	return null;

    return rch->getNPC( )->behavior.getDynamicPointer<Rideable>( );
}

bool HorseHarness::canDress( Character *ch, Character *victim )
{
    return checkHorse( victim );
}

/*---------------------------------------------------------------------
 * HorseBridle
 *--------------------------------------------------------------------*/
HorseBridle::HorseBridle( )
          : tethered( false )
{
}


/*---------------------------------------------------------------------
 * HorseSaddle
 *--------------------------------------------------------------------*/
HorseSaddle::HorseSaddle( )
{
}

