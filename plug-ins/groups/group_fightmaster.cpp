
/* $Id: group_fightmaster.cpp,v 1.1.2.24.6.20 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "logstream.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "act_move.h"
#include "affect.h"
#include "commonattributes.h"

#include "mercdb.h"
#include "npcharacter.h"
#include "object.h"
#include "pcharacter.h"
#include "race.h"
#include "room.h"

#include "act.h"
#include "clanreference.h"
#include "damage.h"
#include "def.h"
#include "fight.h"
#include "handler.h"
#include "interp.h"
#include "magic.h"
#include "material.h"
#include "merc.h"
#include "mercdb.h"
#include "morphology.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "charutils.h"
#include "skill_utils.h"
#include "roomutils.h"
#include "vnum.h"

GSN(area_attack);
GSN(bash);
GSN(bash_door);
GSN(blind_fighting);
GSN(cavalry);
GSN(crush);
GSN(double_kick);
GSN(forest_fighting);
GSN(kick);
GSN(protective_shield);
GSN(smash);
GSN(parry);
GSN(hand_block);
GSN(bat_swarm);
GSN(shield_block);
GSN(cross_block);
GSN(dodge);
GSN(trip);
CLAN(shalafi);
PROF(anti_paladin);
PROF(samurai);
PROF(warrior);
PROF(paladin);
PROF(ninja);
PROF(thief);

static auto needsSpam = [](Character *target) {
    return target->getPC() 
            && IS_SET(target->getPC()->config, CONFIG_FIGHTSPAM);
};


/*
 * 'area attack' skill command
 */
SKILL_DECL(areaattack);
SKILL_APPLY(areaattack)
{
    int count = 0, max_count;
    Character *vch, *vch_next;

    if (number_percent() >= gsn_area_attack->getEffective(ch))
        return false;

    gsn_area_attack->improve(ch, true, victim);

    int slevel = skill_level(*gsn_area_attack, ch);

    if (slevel < 70)
        max_count = 1;
    else if (slevel < 80)
        max_count = 2;
    else if (slevel < 90)
        max_count = 3;
    else
        max_count = 4;

    for (vch = ch->in_room->people; vch != 0; vch = vch_next) {
        vch_next = vch->next_in_room;
        if (vch != victim && vch->fighting == ch) {
            one_hit_nocatch(ch, vch);
            count++;
        }
        if (count == max_count)
            break;
    }

    return true;
}

/*
 * 'parry' skill command
 */
SKILL_DECL(parry);
SKILL_APPLY(parry)
{
    int chance, prof;
    Object *defending_weapon;
    Object *wield;
    // TODO: enhance arg list with custom arg structures?
    bool secondary = !!level;

    if (!IS_AWAKE(victim))
        return false;

    if (IS_AFFECTED(victim,AFF_STUN))
        return false;

    wield = get_wield(ch, secondary);
    defending_weapon = get_eq_char( victim, wear_wield );

    if (!victim->is_npc( ) && defending_weapon == 0)
        return false;

    chance    = gsn_parry->getEffective( victim ) / 2;
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += chance / 5;
    else if (prof == prof_anti_paladin && victim->getClan( ) == clan_shalafi) 
            chance /= 2;

    if (wield && (wield->value0() == WEAPON_FLAIL || wield->value0() == WEAPON_WHIP ))
        return false;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ) )
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }
    
    if (RoomUtils::isNature(victim->in_room) 
        && gsn_forest_fighting->usable(victim)
        &&  (number_percent( ) < gsn_forest_fighting->getEffective( victim ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }


    if (number_percent( ) >= chance + skill_level(*gsn_parry, victim) - ch->getModifyLevel())
        return false;

    if(SHADOW(victim))
    {
        echo_char(victim, needsSpam, "Ты пытаешься парировать атаку, но путаешься в своей тени.");
        echo_char(ch, needsSpam, "%2$^C1 постоянно путается в своей тени.", ch, victim );
        echo_notvict( ch, victim, needsSpam, "%2$^C1 постоянно путается в своей тени.", ch, victim );
        return false;
    }
    
    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && (!defending_weapon 
            || !IS_WEAPON_STAT(defending_weapon, WEAPON_HOLY))) 
    {
        echo_char(victim, needsSpam, "%3$^O1 проходит насквозь через твою попытку спарировать!", ch, victim, wield );
        
        if (defending_weapon) {
            echo_char(ch, needsSpam, "%3$^O1 проходит сквозь оружие %2$C2!", ch, victim, wield );
            echo_notvict( ch, victim, needsSpam, "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!", ch, victim, wield );
        }
        else if (victim->getWearloc().isSet(wear_hands)) {
            echo_char(ch, needsSpam, "%3$^O1 проходит сквозь руки %2$C2!", ch, victim, wield );
            echo_notvict( ch, victim, needsSpam, "%3$^O1 %1$C2 проходит сквозь руки %2$C2!", ch, victim, wield );
        } else {
            echo_char(ch, needsSpam, "%3$^O1 проходит сквозь %2$C4!", ch, victim, wield );
            echo_notvict( ch, victim, needsSpam, "%3$^O1 %1$C2 проходит сквозь %2$C4!", ch, victim, wield );
        }

        return false;
    }

    echo_char(victim, needsSpam, "Ты парируешь атаку %1$C2.", ch, victim);
    echo_char(ch, needsSpam, "%2$^C1 парирует твою атаку.", ch, victim);
    echo_notvict(ch, victim, needsSpam, "%2$^C1 парирует атаку %1$C2.", ch, victim);

    // TODO: damage_to_obj is called here with a chance
    // destroyWeapon( );

    if ( number_percent() >  gsn_parry->getEffective( victim ) && CharUtils::hasLegs(victim))
    {
        /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 20;

        if (ch->size < victim->size)
            chance += (ch->size - victim->size) * 25;
        else
            chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 4/3;

        if ( is_flying( ch ) )
            chance -= 10;

        /* speed */
        if (IS_QUICK(ch))
            chance += 10;

        if (IS_QUICK(victim))
            chance -= 20;

        /* level */
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;

        /* now the attack */
        if (number_percent() < ( chance / 20  ))
        {
            oldact("Ты не можешь устоять на ногах!",ch,0,victim,TO_VICT);
            oldact("Ты падаешь вниз!",ch,0,victim,TO_VICT);
            oldact("$C1 не может устоять на ногах и падает вниз!", ch,0,victim,TO_CHAR);
            oldact("$C1 пытается парировать мощный удар $c1, но не может устоять на ногах.", ch,0,victim,TO_NOTVICT);

            victim->setWait(gsn_bash->getBeats(victim));
            victim->position = POS_RESTING;
        }
    }

    gsn_parry->improve( victim, true, ch );
    return true;
}

/*
 * 'hand block' skill command
 */
SKILL_DECL(handblock);
SKILL_APPLY(handblock)
{
   int chance;

    if ( !IS_AWAKE(victim) )
        return false;
    
    if (!check_bare_hands( victim ))
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;
    
    if (!victim->getWearloc( ).isSet( wear_hands ))
        return false;

    if ( victim->is_npc() || !gsn_hand_block->usable( victim ))
        return false;
        
    if ((chance = gsn_hand_block->getEffective( victim )) <= 1)
        return false;

    if ( victim->getProfession( ) == prof_ninja) {
        chance /= 2;
    }
    else
        chance /= 3;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if ( number_percent( ) >= chance +( skill_level(*gsn_hand_block, victim) - ch->getModifyLevel() ) ) 
        return false;
    
    victim->move -= move_dec( victim );

    if(SHADOW(victim))
    {
        echo_char(victim, needsSpam, "Тень пинает тебя." );
        echo_char(ch, needsSpam, "Тень и %2$C1 играют в кунг-фу.", ch, victim );
        echo_notvict( ch, victim, needsSpam, "Тень и %2$C1 играют в кунг-фу.", ch, victim );
        return false;
    }

    echo_char(victim, needsSpam, "Ты отражаешь руками атаку %1$C2.", ch, victim);
    echo_char(ch, needsSpam, "%2$^C1 отражает руками твою атаку.", ch, victim);
    echo_notvict(ch, victim, needsSpam, "%2$^C1 отражает руками атаку %1$C2.", ch, victim);

    gsn_hand_block->improve( victim, true, ch );
    return true;}

/*
 * 'bat swarm' skill command
 */
SKILL_DECL(batswarm);
SKILL_APPLY(batswarm)
{
    int chance;

    if (!victim->isAffected(gsn_bat_swarm))
        return false;

    chance = 50 + (victim->getModifyLevel( ) - ch->getModifyLevel( ));

    if (number_percent( ) > chance)
        return false;

    if (SHADOW(victim)) {
        echo_char(victim, needsSpam, "Стая летучих мышей вокруг тебя сбита с толку твоей тенью." );
        echo_char(ch, needsSpam, "Стая летучих мышей вокруг %2$C2 сбита с толку тенью.", ch, victim );
        echo_notvict( ch, victim, needsSpam, "Стая летучих мышей вокруг %2$C2 сбита с толку тенью.", ch, victim );
        return false;
    }
    
    echo_char(ch, needsSpam, "Ты не смо%1$Gгло|г|гла пробиться сквозь стаю летучих мышей, кружащихся вокруг %2$C2.", ch, victim);
    echo_char(victim, needsSpam, "Стая летучих мышей не позволяет %1$C3 повредить тебе.", ch, victim );
    echo_notvict( ch, victim, needsSpam, "%1$^C1 пытается разогнать стаю летучих мышей вокруг %2$C2.", ch, victim );

    return true;
}

/*
 * 'shield block' skill command
 */
SKILL_DECL(shieldblock);
SKILL_APPLY(shieldblock)
{
    int chance, prof;
    Object *wield;
    bool secondary = !!level;

    if ( !IS_AWAKE(victim) )
        return false;
    
    if ( get_eq_char( victim, wear_shield ) == 0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;
        
    wield = get_wield(ch, secondary);
    chance = gsn_shield_block->getEffective( victim );
    
    if (chance <= 1)
        return false;

    chance = chance / 2 - 10;
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += 10;

    if (wield) { 
        if (wield->value0() == WEAPON_FLAIL)
            chance /= 2;
        if (wield->value0() == WEAPON_WHIP)
            return false;
    }

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }
    
    if (RoomUtils::isNature(victim->in_room) 
        && gsn_forest_fighting->usable(victim)
        && (number_percent() < gsn_forest_fighting->getEffective( victim ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }
    
    if ( number_percent( ) >= chance + skill_level(*gsn_shield_block, victim) - ch->getModifyLevel()
        || ( !victim->is_npc() && !victim->move ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        echo_char(victim, needsSpam, "Ты впустую машешь щитом перед твоей тенью." );
        echo_room(victim, needsSpam, "%2$^C1 впустую размахивает щитом перед своей тенью.", ch, victim);
        return false;
    }

    echo_char(victim, needsSpam, "Ты отражаешь щитом атаку %1$C2.", ch, victim );
    echo_char(ch, needsSpam, "%2$^C1 отражает твою атаку %2$P2 щитом.", ch, victim );
    echo_notvict( ch, victim, needsSpam, "%2$^C1 отражает атаку %1$C2 своим щитом.", ch, victim );

    // TODO damage_to_obj called here for the shield
    // destroyShield( );

    gsn_shield_block->improve( victim, true, ch );
    return true;

}

/*
 * 'cross block' skill command
 */
SKILL_DECL(crossblock);
SKILL_APPLY(crossblock)
{
    int chance;
    Object *def1, *def2;
    Object *wield;
    bool secondary = !!level;

    if ( !IS_AWAKE(victim) )
        return false;
    
    def1 = get_eq_char( victim, wear_wield );
    def2 = get_eq_char( victim, wear_second_wield );
    wield = get_wield(ch, secondary);

    if ( def1 == 0 || def2 == 0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;

    if ( victim->is_npc() )
    {
        chance    = min( 35, (int)victim->getRealLevel( ) );
    }
    else
    {
        int prof;

        if ( gsn_cross_block->getEffective( victim ) <= 1 )
            return false;

        chance    = gsn_cross_block->getEffective( victim ) / 3;
        prof = victim->getProfession( );

        if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
            chance += chance / 2;
        
    }

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( victim->is_npc() && victim->move <=0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if ( number_percent( ) >= chance + ( skill_level(*gsn_cross_block, victim) - ch->getModifyLevel() ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        echo_char(victim, needsSpam, "Тень запутывает тебя." );
        echo_room(victim, needsSpam, "Тень %2$C2 лезет со своими советам.", ch, victim );
        return false;
    }

    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && !IS_WEAPON_STAT(def1, WEAPON_HOLY) 
        && !IS_WEAPON_STAT(def2, WEAPON_HOLY)) 
    {
        echo_char(victim, needsSpam,"%3$^O1 проходит насквозь через твою попытку кросс-блокировать!", ch, victim, wield );
        echo_char(ch, needsSpam, "%3$^O1 проходит сквозь оружие %2$C2!", ch, victim, wield );
        echo_notvict(ch, victim, needsSpam, "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!", ch, victim, wield );

        return false;
    }
    
    echo_char(victim, needsSpam, "Ты кросс-блокируешь атаку %1$C2.", ch, victim );
    echo_char(ch, needsSpam, "%2$^C1 кросс-блокирует твою атаку.", ch, victim );
    echo_notvict(ch, victim, needsSpam, "%2$^C1 кросс-блокирует атаку %1$C2.", ch, victim );

    // TODO call damage_to_obj for the wield
    // destroyWeapon( );

    if ( number_percent() >  gsn_cross_block->getEffective( victim ) && CharUtils::hasLegs(victim))
    {
        /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 10;

        if (ch->size < victim->size)
            chance += (ch->size - victim->size) * 25;
        else
            chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 5/3;

        if (is_flying( ch ))
            chance -= 20;

        /* speed */
        if (IS_QUICK(ch))
            chance += 10;

        if (IS_QUICK(victim))
            chance -= 20;

        /* level */
        chance += (ch->getRealLevel( ) - victim->getRealLevel( )) * 2;

        /* now the attack */
        if (number_percent() < ( chance / 20  ))
        {
            oldact("Тебе не удается удержать равновесие!\nТы падаешь!", ch, 0, victim, TO_VICT);
            oldact("$C1 не может сдержать твою атаку и падает!", ch, 0, victim, TO_CHAR);
            oldact("$C1 не может сдержать ошеломляющую атаку $c2 и падает.", ch, 0, victim, TO_NOTVICT);

            victim->setWait(gsn_bash->getBeats(victim));
            victim->position = POS_RESTING;
        }
    }
    gsn_cross_block->improve( victim, true, ch );
    return true;
}

/*
 * 'dodge' skill command
 */
SKILL_DECL(dodge);
