/* $Id: fight.cpp,v 1.1.2.12 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko            {NoFate, Demogorgon}                           *
 *    Koval Nazar            {Nazar, Redrum}                                    *
 *    Doropey Vladimir            {Reorx}                                           *
 *    Kulgeyko Denis            {Burzum}                                           *
 *    Andreyanov Aleksandr  {Manwe}                                           *
 *    и все остальные, кто советовал и играл в этот MUD                           *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium             *
 *     Serdar BULUT {Chronos}        bulut@rorqual.cc.metu.edu.tr               *
 *     Ibrahim Canpunar  {Asena}    canpunar@rorqual.cc.metu.edu.tr        *    
 *     Murat BICER  {KIO}        mbicer@rorqual.cc.metu.edu.tr              *    
 *     D.Baris ACAR {Powerman}    dbacar@rorqual.cc.metu.edu.tr            *    
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence                   *    
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                           *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                                   *
 *                                                                           *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                                   *
 *                                                                           *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                   *
 ***************************************************************************/
/***************************************************************************
*    ROM 2.4 is copyright 1993-1995 Russ Taylor                                   *
*    ROM has been brought to you by the ROM consortium                           *
*        Russ Taylor (rtaylor@pacinfo.com)                                   *
*        Gabrielle Taylor (gtaylor@pacinfo.com)                                   *
*        Brian Moore (rom@rom.efn.org)                                           *
*    By using this code, you have agreed to follow the terms of the        *
*    ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/


#include <algorithm>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <math.h>

#include "logstream.h"
#include "skill.h"
#include "skillcommand.h"
#include "commonattributes.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "dreamland.h"
#include "affect.h"
#include "pcharactermanager.h"
#include "room.h"

#include "pcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "clanreference.h"
#include "gsn_plugin.h"
#include "race.h"
#include "npcharacter.h"
#include "core/object.h"
#include "effects.h"
#include "wiznet.h"
#include "../anatolia/handler.h"
#include "act_move.h"
#include "magic.h"
#include "vnum.h"
#include "wearloc_utils.h"
#include "skill_utils.h"

#include "onehit_undef.h"
#include "damage_impl.h"
#include "fight.h"
#include "material.h"
#include "def.h"


GSN(none);
GSN(throw_stone);
GSN(exotic);

PROF(none);
PROF(warrior);
PROF(paladin);
PROF(anti_paladin);
PROF(ninja);
PROF(ranger);
PROF(samurai);

static bool mprog_fight( Character *ch, Character *victim )
{
    FENIA_CALL( ch, "Fight", "C", victim );
    FENIA_NDX_CALL( ch->getNPC( ), "Fight", "CC", ch, victim );
    BEHAVIOR_VOID_CALL( ch->getNPC( ), fight, victim );
    return false;
}

static bool oprog_fight( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Fight", "C", ch );
    FENIA_NDX_CALL( obj, "Fight", "OC", obj, ch );
    BEHAVIOR_VOID_CALL( obj, fight, ch );
    return false;
}

static bool oprog_fight_carry( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "FightCarry", "C", ch );
    FENIA_NDX_CALL( obj, "FightCarry", "OC", obj, ch );
    return false;
}

static void wlprog_fight( Object *obj, Character *ch)
{
    obj->wear_loc->onFight(ch, obj);
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( )
{
    Character *ch;
    Character *victim;
    Object *obj, *obj_next;

    for ( ch = char_list; ch != 0; ch = ch->next )
    {
        if ( ( victim = ch->fighting ) == 0 || ch->in_room == 0 ){
            if ( IS_AFFECTED(ch,AFF_STUN) && !ch->isAffected(gsn_power_word_stun))
            {
                ch->pecho("Оглушение постепенно проходит.");
                REMOVE_BIT(ch->affected_by,AFF_STUN);        
                SET_BIT(ch->affected_by,AFF_WEAK_STUN);        
            }
            else if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
            {
                ch->pecho("Гул в твоей голове затихает.");
                REMOVE_BIT(ch->affected_by,AFF_WEAK_STUN);
            }
            continue;
        }

        if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
        {
            FENIA_VOID_CALL( ch->in_room, "Dive", "Ci", ch,  MOVETYPE_DANGEROUS );

            if ( ch->fighting != 0 )
                multi_hit( ch, victim );
            else
                stop_fighting( ch, false );
        }
        else
        {
            stop_fighting( ch, false );
        }

        if( ( victim = ch->fighting ) == 0 )
        {
            continue;
        }

        // If ch is fighting with a victim, ensure that the victim fights back.
        if (victim->fighting == 0) {
            set_fighting(victim, ch);

            if (victim->fighting == ch) {
                victim->pecho("Ты вступаешь в битву с %C5.", ch);
                ch->pecho("%^C1 вступает с тобой в битву!", victim);
                ch->recho(victim, "%^C1 вступает в битву с %C5.", victim, ch);
            }
        }

        ch->last_fought = victim;

        ch->setLastFightTime( );
        UNSET_DEATH_TIME(ch);

        // Item fight progs (in behaviors) will throw exception if victim is killed.
        try {
            for (obj = ch->carrying; obj; obj = obj_next) {
                obj_next = obj->next_content;

                // onFight is only called for items in meaningful wearlocations.
                // Use onFightCarry to describe fighting behavior in inventory, hair, tail etc.
                if( ch->fighting) {
                    if (obj_is_worn(obj))
                        oprog_fight( obj, ch );
                    else
                        oprog_fight_carry(obj, ch);

                    if (obj_is_worn(obj)) 
                        wlprog_fight(obj, ch);
                }
            }
        } catch (const VictimDeathException &vde) {
            continue;
        }

        /*
         * Fun for the whole family!
         */
        check_assist(ch,victim);
    }
}

struct second_weapon_t {
    int prof;
    int percents[WEAPON_MAX + 1];
};

const struct second_weapon_t second_weapon_table [] = {
/*                  HAND EXOT SWRD DAG STF MACE AXE FLA WHI POLE BOW ARR LAN */
{ prof_warrior,     { 60, 45,  45, 90, 18, 18,  90, 18, 18, 18,  18, 18, 18 } },
{ prof_paladin,     { 60, 45,  45, 90, 18, 60,  18, 90, 18, 60,  18, 18, 18 } },
{ prof_anti_paladin,{ 45, 45,  60, 60, 18, 18,  18, 45, 18, 18,  18, 18, 18 } },
{ prof_ninja,       { 90, 45,  45, 90, 60, 18,  18, 18, 18, 18,  18, 18, 18 } }, 
{ prof_ranger,      { 60, 45,  40, 90, 90, 18,  18, 18, 60, 18,  18, 60, 18 } },
{ prof_samurai,     { 60, 45,  90, 90, 18, 18,  18, 18, 18, 18,  18, 18, 18 } },
{ prof_none,        { 0,                                                    } },
};

void second_weapon_hit( Character *ch, Character *victim, int chance )
{
    int index = 0; /* hand to hand */
    Object *weapon;

    weapon = get_eq_char(ch, wear_wield);
    if (weapon != 0 
        && IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)
        && ch->getRace( )->getSize( ) < SIZE_HUGE)
        return;

    if (get_eq_char(ch, wear_shield) || get_eq_char(ch, wear_hold))
        return;
    
    weapon = get_eq_char(ch, wear_second_wield);
    if (weapon && weapon->item_type == ITEM_WEAPON)
        index = weapon->value0() + 1;
  
    int chance_modifier = 18;
 
    for (int i = 0; second_weapon_table[i].prof != prof_none; i++) {
        if (ch->getProfession( ) == second_weapon_table[i].prof) {
            chance_modifier = second_weapon_table[i].percents[index];
            break;
        }
    }
    
    chance = chance * chance_modifier / 100;

    if (number_percent( ) < gsn_second_weapon->getEffective( ch ) * chance / 100) {
        one_hit_nocatch( ch, victim, true );
        gsn_second_weapon->improve( ch, true, victim );
    }
}

bool next_attack( Character *ch, Character *victim, Skill &skill, int coef )
{
    int chance = skill.getEffective( ch ) / coef;

    if (IS_SLOW(ch))
        chance = chance * 3 / 4;
    
    chance+= skill_level_bonus(skill, ch);

    if (number_percent( ) < chance) {
        one_hit_nocatch( ch, victim );
        skill.improve( ch, true, victim );
        
        if (ch->fighting != victim)
            return false;
            
        second_weapon_hit( ch, victim, chance );
        
        if (ch->fighting != victim)
            return false;
    }

    return true;
}

bool forest_attack( Character *ch, Character *victim )
{
    if (gsn_forest_fighting->getCommand( )->run(ch, FOREST_ATTACK)) {
        int chance = gsn_forest_fighting->getEffective( ch );
        
        while (number_percent() < chance) {
            one_hit_nocatch( ch, victim );
            gsn_forest_fighting->improve( ch, true, victim );
            
            if (ch->fighting != victim)
                return false;
                
            second_weapon_hit( ch, victim, chance );
            
            if (ch->fighting != victim)
                return false;
                
            chance /= 3;
        }
    }

    return true;
}

/*
 * Do one group of attacks.
 */
void multi_hit( Character *ch, Character *victim )
{
    try {
        multi_hit_nocatch( ch, victim );
    }
    catch (const VictimDeathException &) {
    }
}
   
void multi_hit_nocatch( Character *ch, Character *victim )
{
    /* no attacks for stunnies -- just a check */
    if ( ch->position < POS_RESTING )
        return;

    /* no attacks on ghosts or attacks by ghosts */
    if ( ( !victim->is_npc() && IS_GHOST( victim ) )
        || ( !ch->is_npc() && IS_GHOST( ch ) ) )
        return;

    if (check_stun( ch, victim ))
        return;
    
    mprog_fight( ch, victim );

    if (ch->is_npc( ))
        return;
   
    one_hit_nocatch( ch, victim );

    if (ch->fighting != victim)
        return;
    
    gsn_area_attack->getCommand( )->run( ch, victim );

    if (IS_QUICK(ch))
        one_hit_nocatch( ch, victim );

    if (ch->fighting != victim )
        return;
    
    second_weapon_hit(ch,victim,100);

    ch->fighting == victim 
        && next_attack( ch, victim, *gsn_second_attack, 2 )
        && next_attack( ch, victim, *gsn_third_attack, 3 )
        && next_attack( ch, victim, *gsn_fourth_attack, 3 )
        && next_attack( ch, victim, *gsn_fifth_attack, 3 )
        && forest_attack( ch, victim );
}

void one_hit_nocatch( Character *ch, Character *victim, bool secondary )
{
    UndefinedOneHit( ch, victim, secondary ).hit( );
}

void one_hit( Character *ch, Character *victim, bool secondary )
{
    try {
        one_hit_nocatch( ch, victim, secondary );
    }
    catch (const VictimDeathException& e) {                                     
    }
}

bool 
damage_nocatch( Character *ch, Character *victim, 
                int dam, int sn, int dam_type, bool show, bitstring_t dam_flag ) 
{
    return SkillDamage( ch, victim, sn, dam_type, dam, dam_flag ).hit( show );
}

bool damage( Character *ch, Character *victim, 
             int dam, int sn, int dam_type, bool show, bitstring_t dam_flag ) 
{
    try {
        return damage_nocatch( ch, victim, dam, sn, dam_type, show, dam_flag );
    }
    catch (const VictimDeathException& e) {                                     
    }

    return true;
}

bool 
damage_nocatch( Affect *paf, Character *victim, 
                int dam, int sn, int dam_type, bool show, bitstring_t dam_flag ) 
{
    return SkillDamage( paf, victim, sn, dam_type, dam, dam_flag ).hit( show );
}

bool damage( Affect *paf, Character *victim, 
             int dam, int sn, int dam_type, bool show, bitstring_t dam_flag ) 
{
    try {
        return damage_nocatch( paf, victim, dam, sn, dam_type, show, dam_flag );
    }
    catch (const VictimDeathException& e) {                                     
    }

    return true;
}

void rawdamage_nocatch( Character *ch, Character *victim, int dam_type, int dam, bool show )
{
    RawDamage( ch, victim, dam_type, dam ).hit( show );
}

void rawdamage( Character *ch, Character *victim, int dam_type, int dam, bool show )
{
    try {
        rawdamage_nocatch( ch, victim, dam_type, dam, show );
    } catch (const VictimDeathException &) {
    }
}

static inline bool must_not_yell( Character *ch, Character *victim, int flags )
{
    /* sanity checks */
    if (!ch || !victim || ch == victim)
        return true;
    
    /* sleeping victims yell only with FYP_SLEEP flag */
    if (!IS_AWAKE(victim) && !IS_SET(flags, FYP_SLEEP))
        return true;
    
    /* players yell always */
    if (!victim->is_npc( ))
        return false;
    
    /* in some cases (like murder or shooting) mobs yell always too */
    if (IS_SET(flags, FYP_VICT_ANY))
        return false;
    
    /* by default mobs are silent */
    return true;
}

void yell_panic( Character *ch, Character *victim, const char *msgBlind, const char *msg, int flags )
{
    static const char *defaultMsgBlind = "Помогите! Кто-то напал на меня!";
    static const char *defaultMsg = "Помогите! %1$^C1 напа%1$Gло|л|ла на меня!";

    if (must_not_yell( ch, victim, flags ))
        return;

    if (!victim->can_see( ch )) 
        interpret_raw( victim, "yell", 
                        msgBlind ? msgBlind : defaultMsgBlind );
    else
        interpret_raw( victim, "yell", 
                       fmt( victim, msg ? msg : defaultMsg, ch, victim ).c_str( ) );
}

void damage_to_obj( Character *ch, Object *wield, Object *worn, int damage ) 
{
    if (damage == 0)
        return;

    if (material_is_flagged( worn, MAT_INDESTR ))
        return;

    if (IS_SET( worn->extra_flags, ITEM_NOPURGE ))
        return;

    worn->condition -= damage;

    oldact("$o1 наносит повреждения $O3.", ch, wield, worn, TO_ROOM );

    if (worn->condition < 1) {
        oldact("$O1 разлетается на мелкие части.", ch, wield, worn, TO_ROOM);
        extract_obj( worn );
        return;
    }
}

int move_dec( Character *ch ) 
{
    if (ch->is_npc( ))
        return 0;

    return min( max( ch->getModifyLevel( ) / 20, 1 ), (int)ch->move );
}
