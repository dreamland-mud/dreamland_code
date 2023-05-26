/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          character_scheduler.cpp  -  description
                             -------------------
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "logstream.h"

#include "dlscheduler.h"
#include "update_params.h"

#include "skillreference.h"
#include "skill.h"
#include "spelltarget.h"
#include "affecthandler.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "desire.h"
#include "race.h"

#include "room.h"

#include "dreamland.h"
#include "damage_impl.h"
#include "damageflags.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"
#include "skill_utils.h"

GSN(bandage);
GSN(magic_concentrate);
GSN(curl);
GSN(corruption);

DESIRE(thirst);
DESIRE(hunger);
DESIRE(bloodlust);

void CharacterParamsUpdateTask::before( )
{
}

void CharacterParamsUpdateTask::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( Pointer( this ) );
}

void CharacterParamsUpdateTask::run( Character *ch )
{
    if (ch->in_room == 0)
        return;

    if (ch->isDead( ))
        return;

    if (ch->is_npc() && IS_SET(ch->getNPC()->act, ACT_NOUPDATE))
        return;

    DLString name = ch->getNameC();

    try {
        gainHitPoint( ch );
        gainMana( ch );
        gainMove( ch );
    }
    catch (const VictimDeathException &) {
        LogStream::sendNotice() << "Victim killed in update params task: " << name << endl;
    }
}

static bool afprog_updatehit(Character *ch)
{
    bool rc = false;
    SpellTarget::Pointer spellTarget(NEW, ch);

    for (auto &paf: ch->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect( )->onUpdateHit(spellTarget, paf))
            rc = true;

    return rc;
}

/*------------------------------------------------------------------------
 * Hit point update
 *------------------------------------------------------------------------*/
void CharacterParamsUpdateTask::gainHitPoint( Character *ch )
{
    int gain = 0;
    int number, rate;
    
    if (ch->hit > ch->max_hit) 
        ch->hit = std::max( ch->hit - (ch->hit - ch->max_hit) / 4 - 1, (int)ch->max_hit);

    if (afprog_updatehit(ch))
        return;
    
    if (ch->hit >= ch->max_hit)
        return;

    gain = std::max(3, 2 * ch->getCurrStat(STAT_CON) + (7 * ch->getRealLevel( )) / 4);
    rate = ch->getProfession( )->getHpRate( );
    gain = (gain * rate) / 100;
    number = number_percent();
    gain += number * gain / 100;

    int k = std::max(1, ch->getCurrStat(STAT_CON) - 10);

    gain = gain * k / 10;

    switch ( ch->position.getValue( ) ) {
    default:
        gain /= 4;
        break;
    case POS_SLEEPING:
        if (gsn_curl->usable( ch )) {
            if (number_percent( ) < gsn_curl->getEffective( ch )) {
                gain += (gain * skill_level_bonus(*gsn_curl, ch) )/ 20 ;
                gsn_curl->improve( ch, true );
            }
        }
        
        break;
    case POS_RESTING:
        gain /= 2;
        break;
    case POS_FIGHTING:
        gain /= 8;
        break;
    }

    // Don't restore normal health when hungry or thirsty, but allow to recover from negative hp (stunned, incap position).
    if (!ch->is_npc( ) && ch->hit > 0)
        if (desire_hunger->isActive( ch->getPC( ) )
            || desire_thirst->isActive( ch->getPC( ) )
            || desire_bloodlust->isActive( ch->getPC( ) ))
        {
             gain = 0;
        }
    
    gain = gain * ch->in_room->getHealRate() / 100;

    if ( ch->on != 0 && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * ch->on->value3() / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
        gain /= 4;

    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
        gain /= 8;

    if ( IS_AFFECTED(ch,AFF_HASTE) )
        gain /= 2 ;

    if (IS_AFFECTED(ch, AFF_SLOW))
        gain *= 2 ;

    if ( IS_HARA_KIRI(ch) )
        gain *= 3;

    if (IS_AFFECTED(ch, AFF_REGENERATION)) {
        // extra gain for "native" regen, e.g. trolls/mawgs -- but not mobs
        if (ch->getRace()->getAff().isSet(AFF_REGENERATION) && !ch->is_npc())
            gain *= 4;
        else if (!ch->is_npc())
            gain *= 2;
        else
            gain = gain * 3 / 2;
    }

    gain += gain * ch->heal_gain / 100;

    gain = std::max ( (int)(number_percent( ) < gain * 100 / 24), gain / 24 );
    
    ch->hit = std::min( (ch->is_npc()?gain*7:gain) + ch->hit, (int)ch->max_hit);
}



/*------------------------------------------------------------------------
 * Mana point update
 *------------------------------------------------------------------------*/
void CharacterParamsUpdateTask::gainMana( Character *ch )
{
    int gain = 0;
    int number, rate;

    if ( ch->mana == ch->max_mana )
        return;

    gain = ch->getCurrStat(STAT_WIS) + (2 * ch->getCurrStat(STAT_INT)) + ch->getRealLevel( );
    rate = ch->getProfession( )->getManaRate( );
    gain = ( gain * rate ) / 100;

    number = number_percent();
    gain += number * gain / 100;

    if ( ch->getProfession()->getManaRate() > 70 )
        gain += number * gain / 100;


    if (rate < 70)
        gain /= 2;

    switch ( ch->position.getValue( ) ) {
    default:
        gain /= 4;
        break;
    case POS_SLEEPING:
        break;
    case POS_RESTING:
        gain /= 2;
        break;
    case POS_FIGHTING:
        gain /= 10;
        break;
    }

    if (!ch->is_npc( ) && ch->mana > 0)
        if (desire_hunger->isActive( ch->getPC( ) )
            || desire_thirst->isActive( ch->getPC( ) )
            || desire_bloodlust->isActive( ch->getPC( ) ))
        {
             gain = 0;
        }

    gain = gain * min(400, ch->in_room->getManaRate()) / 100;

    if ( ch->on != 0 && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * min(400, ch->on->value4()) / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain /= 4;

    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
        gain /= 8;

    if ( IS_AFFECTED(ch,AFF_HASTE) )
        gain /= 2 ;
    
    if (IS_AFFECTED(ch, AFF_SLOW))
        gain *= 2 ;

    if (ch->isAffected( gsn_magic_concentrate ))
        gain /= 2;

    if ( ch->getCurrStat(STAT_INT) > 20 )
        gain = ( gain * 13) / 10;

    if ( ch->getCurrStat(STAT_WIS) > 20 )
        gain = ( gain * 11) / 10;

    if ( IS_HARA_KIRI(ch) )
        gain *= 3;

    gain += gain * ch->mana_gain / 100;

    gain = std::max ( (int)(number_percent( ) < gain * 100 / 16), gain / 16 );
    ch->mana = std::min( ch->mana + gain, (int)ch->max_mana );
}

/*-------------------------------------------------------------------------
 * Move point update
 *------------------------------------------------------------------------*/
void CharacterParamsUpdateTask::gainMove( Character *ch )
{
    int gain = 0;

    if ( ch->move == ch->max_move )
        return;

    if ( ch->is_npc() ) {
        gain = ch->getRealLevel( );
    } else {
        gain = std::max( 15, 2 * ch->getRealLevel( ) );

        switch ( ch->position.getValue( ) ) {
        case POS_SLEEPING:
            gain += 2 * (ch->getCurrStat(STAT_DEX));
            break;
        case POS_RESTING:
            gain += ch->getCurrStat(STAT_DEX);
            break;
        }

        if (desire_hunger->isActive( ch->getPC( ) )
            || desire_thirst->isActive( ch->getPC( ) )
            || desire_bloodlust->isActive( ch->getPC( ) ))
        {
             gain = 3;
        }
    }

    gain = gain * ch->in_room->getHealRate() / 100;

    if ( ch->on != 0 && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * ch->on->value3() / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
        gain /= 4;

    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE)) 
        gain /=2 ;

    if (IS_AFFECTED(ch, AFF_SLOW))
        gain *= 2;
    
    if ( ch->getCurrStat(STAT_DEX) > 20 )
        gain *= (14 /10);

    if ( IS_HARA_KIRI(ch) )
        gain *= 3;

    if( ch->isAffected(gsn_bandage ) )
        gain += ch->getRealLevel( ) / 20;

    gain += gain * ch->heal_gain / 100;

    gain = std::max ( (int)(number_percent( ) < gain * 100 / 16), gain / 16 );
    ch->move = std::min( ch->move + gain, (int)ch->max_move );
}

