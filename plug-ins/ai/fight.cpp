/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"

#include "skillcommand.h"
#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "damage.h"
#include "fight.h"
#include "fenia_utils.h"
#include "interp.h"
#include "handler.h"
#include "magic.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(none);
GSN(blindness);
GSN(faerie_fire);
GSN(dispel_affects);
GSN(curse);
GSN(second_attack);
GSN(third_attack);
GSN(fourth_attack);
GSN(guard);
GSN(dirt_kicking);
GSN(smash);
GSN(backstab);
GSN(tiger_power);
GSN(disarm);
GSN(bash);

/*
 * Hit victim for the first time 
 */
void BasicMobileBehavior::attack( Character *victim )
{
    if (gsn_guard->getCommand( )->apply( victim, ch ))
        victim = victim->getPC( )->guarded_by;
    
    if (isAfterCharm( ))
        attackDumb( victim );
    else
        attackSmart( victim );
}

void BasicMobileBehavior::attackDumb( Character *victim )
{
    multi_hit( ch, victim , "murder" );
}

void BasicMobileBehavior::attackSmart( Character *victim )
{   
    gprog("onAttackAI", "CC", ch, victim);
}


/*
 * Battle-round. Called from multi_hit and every 4 seconds from violence_update
 */
void BasicMobileBehavior::fight( Character *victim )
{
    if (doWimpy( ))
        return;

    /* no attacks on ghosts */
    if (IS_GHOST( victim )) 
        return;

    if (ch->is_mirror( ))
        return;

    setLastFought( victim );

    one_hit( ch, victim );

    if (ch->fighting != victim)
        return;

    /* Area attack -- BALLS nasty! */
    if (IS_SET(ch->off_flags, OFF_AREA_ATTACK)) {
        Character *vch, *vch_next;

        for (vch = ch->in_room->people; vch != 0; vch = vch_next) {
            vch_next = vch->next_in_room;

            if ((vch != victim && vch->fighting == ch))
                one_hit( ch, vch );
        }
    }

    if (IS_QUICK(ch))
        one_hit( ch, victim );
    
    try {
        ch->fighting == victim 
            && next_attack( ch, victim, *gsn_second_attack, 2 )
            && next_attack( ch, victim, *gsn_third_attack, 4 )
            && next_attack( ch, victim, *gsn_fourth_attack, 6 );
    }
    catch (const VictimDeathException &e) {
        return;
    }

    if (ch->fighting != victim)
        return;

    if (ch->wait > 0)
        return;

    gprog("onFightAI", "C", ch);
}


/*
 * Wimp out
 */
bool BasicMobileBehavior::mustFlee( )
{
    if (IS_SET(ch->act, ACT_WIMPY) 
            && number_bits( 2 ) == 0
            && ch->hit < ch->max_hit / 5)
    {
        return true;
    }
    
    if (IS_CHARMED(ch) 
            && ch->master->in_room != ch->in_room)
    {
        return true;
    }

    if (CAN_DETECT(ch,ADET_FEAR) && !IS_SET(ch->act,ACT_NOTRACK)) 
        return true;

    return false;
}

bool BasicMobileBehavior::doWimpy( )
{
    if (ch->wait >= dreamland->getPulseViolence( ) / 2)
        return false;
    
    if (!mustFlee( ))
        return false;
    
    interpret_raw( ch, "flee" );
    clearLastFought( );

    return true;
}
