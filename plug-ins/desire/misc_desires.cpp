/* $Id$
 *
 * ruffina, 2004
 */
#include "misc_desires.h"
#include "desire_damages.h"

#include "pcharacter.h"
#include "room.h"

#include "interp.h"
#include "fight_safe.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*
 * bloodlust
 */
int BloodlustDesire::getUpdateAmount( PCharacter *ch )
{
    return -1;
}

void BloodlustDesire::damage( PCharacter *ch )
{
    int dam;

    if (ch->in_room->people && !ch->fighting) {
	Character *vch, *vch_next;

	if (!IS_AWAKE(ch))
	    interpret_raw( ch, "stand" );

	for (vch = ch->in_room->people; vch != 0 && ch->fighting == 0; vch = vch_next) {
	    vch_next = vch->next_in_room;
	    
	    if (ch != vch 
		&& ch->can_see(vch) 
		&& !is_safe_nomessage(ch, vch))
	    {
		interpret_raw( ch, "yell", "КРОВИ! Я ЖАЖДУ КРОВИ!");
		interpret_raw( ch, "murder",  vch->getNameP( ));
		return;
	    }
	}
    }

    dam = ch->max_hit * number_range(2, 4) / 100;
    dam = max( dam, 1 );

    ThirstDamage( ch, dam ).hit( true );
}


bool BloodlustDesire::applicable( PCharacter *ch )
{
    return isVampire( ch );
}


/*
 * drunk
 */
int DrunkDesire::getUpdateAmount( PCharacter *ch )
{
    return -1;
}

bool DrunkDesire::applicable( PCharacter *ch )
{
#ifdef DRUNK_SUPPORT    
    return !isVampire( ch );
#else
    return false;
#endif
}

bool DrunkDesire::isActive( PCharacter *ch )
{
    return applicable( ch ) && ch->desires[getIndex( )] > activeLimit;
}

bool DrunkDesire::canDrink( PCharacter *ch )
{
    if (isActive( ch )) {
	ch->println( "Ты проносишь мимо рта.. *ИК*" );
	return false;
    }

    return true;
}

/*
 * full
 */
int FullDesire::getUpdateAmount( PCharacter *ch )
{
    return ch->size > SIZE_MEDIUM ? -4 : -2;
}

bool FullDesire::applicable( PCharacter *ch )
{
    return !isVampire( ch );
}

bool FullDesire::canDrink( PCharacter *ch )
{
    if (isOverflow( ch )) {
	ch->println( "Ты больше не можешь выпить ни капли." );
	return false;
    }

    return true;
}

bool FullDesire::canEat( PCharacter *ch )
{
    if (isOverflow( ch )) {
	ch->println( "Ты больше не можешь съесть ни кусочка." );
	return false;
    }

    return true;
}

bool FullDesire::isOverflow( PCharacter *ch )
{
    return applicable( ch ) && ch->desires[getIndex( )] > overflowLimit;
}

/*
 * thirst
 */
int ThirstDesire::getUpdateAmount( PCharacter *ch )
{
    if (ch->in_room->sector_type == SECT_DESERT)
	return -3;
    else
	return -1;
}

void ThirstDesire::damage( PCharacter *ch )
{
    int dam;
    
    dam = ch->max_hit * number_range(2, 4) / 100;
    dam = max( dam, 1 );

    ThirstDamage( ch, dam ).hit( true );
}

bool ThirstDesire::applicable( PCharacter *ch )
{
    return !isVampire( ch );
}

/*
 * hunger
 */
int HungerDesire::getUpdateAmount( PCharacter *ch )
{
    return ch->size > SIZE_MEDIUM ? -2 : -1;
}


void HungerDesire::damage( PCharacter *ch )
{
    int dam;
    
    dam = ch->max_hit * number_range(2, 4) / 100;
    dam = max( dam, 1 );

    HungerDamage( ch, dam ).hit( true );
}

bool HungerDesire::applicable( PCharacter *ch )
{
    return !isVampire( ch );
}

