/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
#include "controlresist.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "clanreference.h"

#include "dreamland.h"
#include "interp.h"
#include "merc.h"
#include "def.h"

CLAN(none);

/*---------------------------------------------------------------------------
 * wait and daze state, fight delay time 
 *--------------------------------------------------------------------------*/
int control_resist( Character *ch, int current, int pulses )
{
    if (pulses <= current || !ch->is_npc( ) || ch == interpret_actor( ))
        return max( current, pulses );

    int divisor;
    switch (ch->getNPC( )->getTier( )) {
    case 1:  divisor = 4; break;
    case 2:  divisor = 3; break;
    case 3:  divisor = 2; break;
    default: divisor = 1; break;
    }

    // Only the added part shrinks, so "wait + n" and "max(wait, n)" agree.
    return current + (pulses - current) / divisor;
}

void Character::setWait( int pulses )
{
    if (is_immortal( ))
        wait = 1;
    else
        wait = control_resist( this, wait, pulses );
}

void Character::setWaitViolence( int bticks )
{
    setWait( bticks * dreamland->getPulseViolence( ) );
}

void Character::setDaze( int pulses )
{
    daze = control_resist( this, daze, pulses );
}

void Character::setDazeViolence( int bticks )
{
    setDaze( bticks * dreamland->getPulseViolence( ) );
}

time_t Character::getLastFightDelay( ) const
{
    return dreamland->getCurrentTime( ) - last_fight_time;
}

void Character::setLastFightTime( )
{
    last_fight_time = dreamland->getCurrentTime( );
    adrenaline_pending = true;
}

void Character::unsetLastFightTime( )
{
    last_fight_time = -1;
    adrenaline_pending = false; // death/reset: suppress the "calmed down" line
}

time_t Character::getLastFightTime( ) const
{
    return last_fight_time;
}

bool Character::is_adrenalined( ) const
{
    if (last_fight_time == -1)
        return false;

    if (is_immortal( ))
        return false;
        
    return getLastFightDelay( ) < FIGHT_DELAY_TIME;
}


