/* $Id$
 *
 * ruffina, 2004
 */
#include "sleepaffecthandler.h"
#include "skill.h"
#include "npcharacter.h"
#include "merc.h"

void SleepAffectHandler::remove( Character *victim )
{
    DefaultAffectHandler::remove( victim );

    if(victim->is_npc() && victim->position == POS_SLEEPING)
	victim->position = victim->getNPC()->default_pos;
}

