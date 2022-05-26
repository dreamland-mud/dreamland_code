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

    /* TODO: remove the following code once global onAttackAI is implemented. */     
    const char *cmd = 0;
    static const SpellChance spellTable [] = {
        { gsn_blindness,     50 },
        { gsn_faerie_fire,   50 }, 
        { gsn_dispel_affects, 100 },
        { gsn_curse,        100 },
        { gsn_none,          -1 }
    };

    if (gsn_backstab->usable( ch ) && get_eq_char( ch, wear_wield ) && chance( 80 ))
        cmd = "backstab";
    else 
    if (gsn_smash->usable( ch ) && chance( 80 )) 
        cmd = "smash";
    else 
    if (gsn_dirt_kicking->usable( ch )) 
        cmd = "dirt";
    else 
    if (gsn_bash->usable( ch ) && get_eq_char( ch, wear_shield )) 
        cmd = "bash";

    if (cmd) 
        interpret_raw( ch, cmd, victim->getNameC() );
    else 
        SpellChanceTable( spellTable, ch, victim ).castSpell( );
        
    if (!ch->fighting)
        multi_hit( ch, victim , "murder" );
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

    
    /* TODO: remove the following block once implemented in global onFightAI */
    gprog("onFightAI", "C", ch);

    const char *cmd = 0;

    switch (number_range(0, 7)) {
    case (0) :
        if (IS_SET(ch->off_flags, OFF_BASH))
            cmd = "smash";
        break;

    case (1) :
        if (!IS_AFFECTED(ch,AFF_BERSERK)) {
            if (gsn_tiger_power->usable( ch ))
                cmd = "tiger";
            else if (IS_SET(ch->off_flags,OFF_BERSERK))
                cmd = "berserk";
        }
        break;

    case (2) :
        if (gsn_disarm->usable( ch ) && !(get_eq_char( victim, wear_wield ) == 0))
            cmd = "disarm";       
        break;

    case (3) :
        if (IS_SET(ch->off_flags,OFF_KICK))
            cmd = "kick";
        break;

    case (4) :
        if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
            cmd = "dirt";
        break;

    case (5) :
        if (IS_SET(ch->off_flags,OFF_TAIL))
            cmd = "tail";
        break;

    case (6) :
        if (IS_SET(ch->off_flags,OFF_TRIP))
            cmd = "trip";
        break;
    case (7) :
        if (IS_SET(ch->off_flags,OFF_CRUSH))
            cmd = "crush";
        break;
    }

    if (cmd)
        interpret_raw( ch, cmd );
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
