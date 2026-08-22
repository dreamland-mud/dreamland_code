/* $Id$
 *
 * ruffina, 2004
 */
#include "misc_desires.h"
#include "desire_damages.h"

#include "pcharacter.h"
#include "room.h"
#include "affect.h"
#include "loadsave.h"
#include "skillreference.h"

#include "interp.h"
#include "fight_safe.h"
#include "wearloc_utils.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"

RELIG(karmina);
GSN(hunger);
GSN(thirst);

/*
 * Starvation / dehydration no longer deal HP damage (hunger/thirst rework). Instead
 * each hangs an undispellable weakening affect -- reduced strength and slowed -- while
 * the desire sits at its floor; it clears on its own once the character eats or drinks.
 *
 * Refreshed by resetting the duration on the existing affect rather than affect_join,
 * which ADDS the modifier every call (affects.cpp) and would spiral strength down each
 * tick. A short positive duration (not a permanent -1) is deliberate: char_update_affects
 * only ticks POSITIVE-duration affects, so -1 would silence the Fenia onUpdateChar flavor
 * hook; 2 ticks survives the per-tick decrement while starving and decays off within a
 * tick or two of feeding, firing the affect's removeChar message.
 */
static void refresh_desire_affect( PCharacter *ch, int sn, int strPenalty )
{
    if (ch->isAffected( sn )) {
        for (auto &paf: ch->affected.findAll( sn ))
            paf->duration = 2;
        return;
    }

    Affect af;
    af.type     = sn;
    af.level    = ch->getModifyLevel( );
    af.duration = 2;
    af.location.setTable( &apply_flags );
    af.location = APPLY_STR;
    af.modifier = strPenalty;
    af.bitvector.setTable( &affect_flags );
    af.bitvector.setValue( AFF_SLOW );
    affect_to_char( ch, &af );
}

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
                ch->pecho(_("{rКармина{x утоляет твою жажду, предотвращая безумие."));
                ch->recho(_("%^O1 на челе %C2 вспыхивает {rбагряным{x."), tattoo, ch);
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
                interpret_raw( ch, "murder",  "%s", vch->getNameC());
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
        ch->pecho( _("Ты проносишь мимо рта... *ИК*") );
        return false;
    }

    return true;
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
    refresh_desire_affect( ch, gsn_thirst, -2 );
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
    refresh_desire_affect( ch, gsn_hunger, -2 );
}

bool HungerDesire::applicable( PCharacter *ch )
{
    return !isVampire( ch );
}

