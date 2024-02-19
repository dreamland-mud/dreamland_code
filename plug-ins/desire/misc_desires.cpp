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
#include "wearloc_utils.h"
#include "merc.h"

#include "def.h"

RELIG(karmina);

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

    if (ch->in_room->people && !ch->fighting && !IS_AFFECTED(ch, AFF_SLEEP)) {
        Character *vch, *vch_next;

        if (!IS_AWAKE(ch))
            interpret_raw( ch, "stand" );

        if (ch->getReligion() == god_karmina && chance(50)) {
            Object *tattoo = get_eq_char(ch, wear_tattoo);
            if (tattoo) {
                ch->pecho("{rКармина{x утоляет твою жажду, предотвращая безумие.");
                ch->recho("%^O1 на челе %C2 вспыхивает {rбагряным{x.", tattoo, ch);
                ch->desires[getIndex( )] = 40;
                return;
            }
        }

        for (vch = ch->in_room->people; vch != 0 && ch->fighting == 0; vch = vch_next) {
            vch_next = vch->next_in_room;
            
            if (ch != vch 
                && ch->can_see(vch) 
                && !IS_BLOODLESS(vch)
                && !is_safe_nomessage(ch, vch))
            {
                interpret_raw( ch, "yell", "КРОВИ! Я ЖАЖДУ КРОВИ!");
                interpret_raw( ch, "murder",  vch->getNameC());
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
    return !isVampire(ch);
}

bool DrunkDesire::isActive( PCharacter *ch )
{
    return applicable( ch ) && ch->desires[getIndex( )] > activeLimit;
}

bool DrunkDesire::canDrink( PCharacter *ch )
{
    if (isActive( ch )) {
        ch->pecho( "Ты проносишь мимо рта.. *ИК*" );
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
        ch->pecho( "Твой желудок полон, ты больше не можешь выпить ни капли." );
        return false;
    }

    return true;
}

bool FullDesire::canEat( PCharacter *ch )
{
    if (isOverflow( ch )) {
        ch->pecho( "Твой желудок полон, ты больше не можешь съесть ни кусочка." );
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
    if (ch->in_room->getSectorType() == SECT_DESERT)
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

