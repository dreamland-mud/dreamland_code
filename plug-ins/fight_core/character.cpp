/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
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
void Character::setWait( int pulses )
{
    if (is_immortal( ))
	wait = 1;
    else
	wait = max( wait, pulses );
}

void Character::setWaitViolence( int bticks )
{
    setWait( bticks * dreamland->getPulseViolence( ) );
}

void Character::setDaze( int pulses )
{
    daze = max( daze, pulses );
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
}

void Character::unsetLastFightTime( )
{
    last_fight_time = -1;
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


/*
 * player pk counter 
 */
void PCharacter::check_hit_newbie( Character *victim )
{
	// памятка: "как попасть в аутсайдеры"
#warning надо б добавить код дуэлей к Character::check_hit_newbie()

	if ( getClan() != clan_none ) // клановые не считаются
		return;

	if ( victim->is_npc() ) // на всякий случай
		return;

	if ( victim->getClan() != clan_none ) // нападения на клановых - прямой путь в out-s :)
	{
		interpret_raw(this, "clan", "petition outsider");
		return;
	}

	newbie_hit_counter.setValue( newbie_hit_counter.getValue() + 1 );
	
	if ( newbie_hit_counter.getValue() > 10 ) // на неклановых можно нападать.. но недолго
	{
		interpret_raw(this, "clan", "petition outsider");
		return;
	}
}


