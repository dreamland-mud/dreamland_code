/* $Id$
 *
 * ruffina, 2004
 */
#include "npcharacter.h"
#include "pcharacter.h"
#include "wearlocation.h"
#include "char_weight.h"

#include "stats_apply.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * Retrieve a character's carry capacity.
 *-------------------------------------------------------------------------*/
int Char::canCarryNumber(Character *ch) 
{
    if (ch->is_immortal( ) && ch->getRealLevel( ) >= LEVEL_HERO)
        return 1000;
    
    if (ch->master && !ch->master->is_npc( ) &&ch-> master->getPC( )->pet == ch)
        return 0;

    return wearlocationManager->size( ) + ch->getCurrStat(STAT_DEX) - 10 + ch->size;
}

int Char::canCarryWeight(Character *ch) 
{
    if (ch->is_immortal( ) && ch->getRealLevel( ) >= LEVEL_HERO)
        return 10000000;

    if (ch->master && !ch->master->is_npc( ) && ch->master->getPC( )->pet == ch)
        return 0;

    return get_str_app(ch).carry * 10 + ch->getRealLevel( ) * 25;
}

int Char::getCarryWeight(Character *ch)
{
    return ch->carry_weight + ch->silver / 12 + ch->gold * 2 / 5;
}

