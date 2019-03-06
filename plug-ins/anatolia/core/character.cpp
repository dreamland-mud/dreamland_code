/* $Id$
 *
 * ruffina, 2004
 */
#include "npcharacter.h"
#include "pcharacter.h"
#include "wearlocation.h"

#include "stats_apply.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * Retrieve a character's carry capacity.
 *-------------------------------------------------------------------------*/
int Character::canCarryNumber( ) 
{
    if (is_immortal( ) && getRealLevel( ) >= LEVEL_HERO)
        return 1000;
    
    if (master && !master->is_npc( ) && master->getPC( )->pet == this)
        return 0;

    return wearlocationManager->size( ) + getCurrStat(STAT_DEX) - 10 + size;
}

int Character::canCarryWeight( ) 
{
    if (is_immortal( ) && getRealLevel( ) >= LEVEL_HERO)
        return 10000000;

    if (master && !master->is_npc( ) && master->getPC( )->pet == this)
        return 0;

    return get_str_app(this).carry * 10 + getRealLevel( ) * 25;
}

int Character::getCarryWeight( ) const
{
    return carry_weight + silver / 12 + gold * 2 / 5;
}

