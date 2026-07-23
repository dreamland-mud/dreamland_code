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
#include "profiler.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "core/behavior/behavior_utils.h"
#include "dreamland.h"
#include "affect.h"
#include "affecthandler.h"
#include "pcharactermanager.h"
#include "room.h"
#include "roomutils.h"

#include "pcharacter.h"
#include "merc.h"

#include "act.h"
#include "interp.h"
#include "clanreference.h"

#include "race.h"
#include "npcharacter.h"
#include "core/object.h"
#include "effects.h"
#include "wiznet.h"
#include "loadsave.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "magic.h"
#include "vnum.h"
#include "wearloc_utils.h"
#include "profflags.h"
#include "skill_utils.h"
#include "fenia_utils.h"
#include "../loadsave/behavior_utils.h"
#include "onehit_undef.h"
#include "damage_impl.h"
#include "fight.h"
#include "material.h"
#include "def.h"
#include "l10n.h"
#include "descriptor.h"
#include "comm.h"

GSN(area_attack);
GSN(fifth_attack);
GSN(forest_fighting);
GSN(fourth_attack);
GSN(paralysis);
GSN(second_attack);
GSN(second_weapon);
GSN(third_attack);

PROF(none);
PROF(warrior);
PROF(paladin);
PROF(anti_paladin);
PROF(ninja);
PROF(ranger);
PROF(samurai);

bool rprog_dive(Character *wch, int danger);

static void afprog_fight(Character *ch, Character *victim)
{
    SpellTarget::Pointer target(NEW, ch);

    for (auto &paf: ch->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect()->onFight(target, paf, victim);
}

static bool mprog_fight( Character *ch, Character *victim, string command )
{
    if (behavior_trigger(ch, "Fight", "CC", ch, victim))
        return true;

    FENIA_CALL( ch, "Fight", "C", victim );
    FENIA_NDX_CALL( ch->getNPC( ), "Fight", "CC", ch, victim );
    BEHAVIOR_VOID_CALL( ch->getNPC( ), fight, victim, command );
    return false;
}

static bool oprog_fight( Object *obj, Character *ch )
{
    if (behavior_trigger(obj, "Fight", "OC", obj, ch))
        return true;
    
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
// Colour for the fightspam-OFF round-summary number, by % of victim's max hp
// (mirrors the newdamage intensity progression r->M->G->B->R, percent-keyed).
static char round_damage_colour( int perc )
{
    if (perc <= 10) return 'r';
    if (perc <= 25) return 'M';
    if (perc <= 50) return 'G';
    if (perc <= 75) return 'B';
    return 'R';
}

// After a combat round, fightspam-OFF players get one aggregated line per
// participant in their room ("<attacker> => <victim> [N]"), shown only for real
// damage (>0) against a still-living victim -- a kill already printed its line.
// Pointer-safe: runs over the LIVE char_list after the hit loop (dead chars are
// already extracted; the victim is read live as att->fighting).
static void fightspam_round_summary( )
{
    for (Character *ch = char_list; ch != 0; ch = ch->next) {
        if (ch->getPC( ) == 0 || ch->extracted || ch->in_room == 0)
            continue;
        if (ch->fighting == 0) {
            ch->fightEmptyStreak = 0; // reset the indicator between fights
            continue;
        }
        if (!IS_AWAKE(ch) || IS_SET(ch->getPC( )->config, CONFIG_FIGHTSPAM))
            continue;

        bool anyLine = false;
        for (Character *att = ch->in_room->people; att != 0; att = att->next_in_room) {
            if (att->extracted || att->roundDamage <= 0)
                continue;

            Character *vic = att->fighting;
            if (vic == 0 || vic->extracted || vic->position == POS_DEAD || vic->max_hit <= 0)
                continue;
            if (!ch->can_sense( att ) || !ch->can_sense( vic ))
                continue;

            int perc = att->roundDamage * 100 / vic->max_hit;
            char num[64];
            snprintf( num, sizeof( num ), "{D[{x{%c%d{x{D]{x",
                      round_damage_colour( perc ), att->roundDamage );

            // Args in reference order (1=attacker, 2=victim, 3=number string) so
            // the act formatter reads each va_arg with the right type.
            ch->pecho( _("%1$^C1 {C=>{x %2$C1 %3$s"), att, vic, num );
            anyLine = true;
        }

        if (anyLine) {
            ch->fightEmptyStreak = 0;
            continue;
        }

        // Empty round (no damage summary): show one ">" per empty round on the
        // SAME line ("You're fighting! >>>>"), WITHOUT a prompt -- so the player
        // sees combat is still ticking. The label prints once at the streak start.
        // Only when the output buffer is idle, so we never race real pending output.
        Descriptor *d = ch->desc;
        if (d == 0 || d->outtop != 0)
            continue;

        DLString chunk;
        if (ch->fightEmptyStreak == 0)
            chunk << "{R" << _("Ты сражаешься!").getMessage( ch ).c_str( ) << " {r>{x";
        else
            chunk << "{r>{x";
        ch->fightEmptyStreak++;

        // fcommand=true skips the buffer's leading "\n\r" so ">" appends to the
        // current line; process_output(d,false) flushes it without a prompt.
        bool savedFcommand = d->fcommand;
        d->fcommand = true;
        d->send( chunk.c_str( ) );
        process_output( d, false );
        d->fcommand = savedFcommand;
    }
}

void violence_update()
{
    ProfilerBlock profiler("violence_update", 100);
    Character *ch;
    Character *victim;
    Object *obj, *obj_next;

    // Reset per-round damage accumulators before the hit loop; summarized below.
    for (ch = char_list; ch != 0; ch = ch->next)
        ch->roundDamage = 0;

    for (ch = char_list; ch != 0; ch = ch->next)
    {
        try
        {
            if ((victim = ch->fighting) == 0 || ch->in_room == 0)
            {
                if (IS_AFFECTED(ch, AFF_STUN) && !ch->isAffected(gsn_paralysis))
                {
                    ch->pecho(_("Оглушение постепенно проходит."));
                    REMOVE_BIT(ch->affected_by, AFF_STUN);
                    SET_BIT(ch->affected_by, AFF_WEAK_STUN);
                }
                else if (IS_AFFECTED(ch, AFF_WEAK_STUN))
                {
                    ch->pecho(_("Гул в твоей голове затихает."));
                    REMOVE_BIT(ch->affected_by, AFF_WEAK_STUN);
                }

                if (ch->adrenaline_pending && !ch->is_adrenalined())
                {
                    ch->pecho(_("Адреналин в твоей крови улегся, и ты успокаиваешься."));
                    ch->adrenaline_pending = false;
                }
                continue;
            }

            if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
            {
                rprog_dive(ch, MOVETYPE_DANGEROUS);

                if (ch->fighting != 0)
                    multi_hit_nocatch(ch, victim);
                else
                    stop_fighting(ch, false);
            }
            else
            {
                stop_fighting(ch, false);
            }

            if ((victim = ch->fighting) == 0)
            {
                continue;
            }

            // If ch is fighting with a victim, ensure that the victim fights back.
            if (victim->fighting == 0)
            {
                set_fighting(victim, ch);

                if (victim->fighting == ch)
                {
                    victim->pecho(_("Ты вступаешь в битву с %C5."), ch);
                    ch->pecho(_("%^C1 вступает с тобой в битву!"), victim);
                    ch->recho(victim, _("%^C1 вступает в битву с %C5."), victim, ch);
                }
            }

            ch->last_fought = victim;

            ch->setLastFightTime();
            UNSET_DEATH_TIME(ch);

            // Affect and item fight progs (in behaviors) will throw exception if victim is killed.
            afprog_fight(ch, victim);

            for (obj = ch->carrying; obj; obj = obj_next)
            {
                obj_next = obj->next_content;

                // onFight is only called for items in meaningful wearlocations.
                // Use onFightCarry to describe fighting behavior in inventory, hair, tail etc.
                if (ch->fighting)
                {
                    if (obj_is_worn(obj))
                        oprog_fight(obj, ch);
                    else
                        oprog_fight_carry(obj, ch);

                    if (obj_is_worn(obj))
                        wlprog_fight(obj, ch);
                }
            }


            /*
            * Fun for the whole family!
            */
            check_assist(ch, victim);

        }
        catch (const VictimDeathException &vde)
        {
            continue;
        }
    }

    fightspam_round_summary( );
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

int second_weapon_chance(Profession *prof, Object *weapon)
{
    int chance_modifier = 18;
    int index = 0; /* hand to hand */

    if (weapon && weapon->item_type == ITEM_WEAPON)
        index = weapon->value0() + 1;

    for (int i = 0; second_weapon_table[i].prof != prof_none; i++) {
        if (prof->getIndex() == second_weapon_table[i].prof) {
            chance_modifier = second_weapon_table[i].percents[index];
            break;
        }
    }

    return chance_modifier;
}

void second_weapon_hit( Character *ch, Character *victim, int chance )
{
    Object *weapon;

    // can't dual wield without either of hands/wrists/arms
    const GlobalBitvector &loc = ch->getWearloc( );    
    if (!loc.isSet( wear_hands )
        || !loc.isSet( wear_wrist_l )
        || !loc.isSet( wear_wrist_r ))        
        return; 
    
    weapon = get_eq_char(ch, wear_wield);
    if (weapon != 0 
        && IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)
        && ch->getRace( )->getSize( ) < SIZE_HUGE)
        return;

    if (get_eq_char(ch, wear_shield) || get_eq_char(ch, wear_hold))
        return;
    
    weapon = get_eq_char(ch, wear_second_wield);
 
    int chance_modifier = second_weapon_chance(ch->getProfession().getElement(), weapon);
     
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
    if (!RoomUtils::isNature(ch->in_room))
        return true;

    if (!gsn_forest_fighting->usable(ch))
        return true;

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

    return true;
}

/*
 * Do one group of attacks.
 */
void multi_hit( Character *ch, Character *victim, string command )
{
    try {
        multi_hit_nocatch( ch, victim, command );
    }
    catch (const VictimDeathException &) {
    }
}
   
void multi_hit_nocatch( Character *ch, Character *victim, string command )
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
    
    mprog_fight( ch, victim, command );

    if (ch->is_npc( ))
        return;
   
    one_hit_nocatch( ch, victim, false, command );

    if (ch->fighting != victim)
        return;
    
    if (gsn_area_attack->usable(ch))
        gsn_area_attack->getCommand( )->apply( ch, victim );

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

void one_hit_nocatch( Character *ch, Character *victim, bool secondary, string command )
{
    UndefinedOneHit( ch, victim, secondary, command ).hit( );
}

void one_hit( Character *ch, Character *victim, bool secondary, string command )
{
    try {
        one_hit_nocatch( ch, victim, secondary, command );
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

void rawdamage_nocatch( Character *ch, Character *victim, int dam_type, int dam, bool show, const DLString &deathReason )
{
    RawDamage( ch, victim, dam_type, dam, deathReason ).hit( show );
}

void rawdamage( Character *ch, Character *victim, int dam_type, int dam, bool show, const DLString &deathReason )
{
    try {
        RawDamage( ch, victim, dam_type, dam, deathReason ).hit( show );
    } catch (const VictimDeathException &) {
    }
}

void yell_panic( Character *ch, Character *victim, const char *msgBlind, const char *msg, const char *label )
{
    gprog("onPanicYell", "CCsss", ch, victim, msgBlind, msg, label ? label : "attack");
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

    oldact(_("$o1 наносит повреждения $O3."), ch, wield, worn, TO_ROOM );

    if (worn->condition < 1) {
        oldact(_("$O1 разлетается на мелкие части."), ch, wield, worn, TO_ROOM);
        obj_dump_content(worn);
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

// Apply damange bonus by character class:
// melee (+1-100%), agile (+0.9-90%), hybrid (+0.5-77%), caster (0)
void damapply_class(Character *ch, int &dam)
{
    if (ch->is_npc())
        return;

    int div;        
   
    if (ch->getProfession( )->getFlags( ).isSet(PROF_HYBRID))
        div = 130; 
    else if (ch->getProfession( )->getFlags( ).isSet(PROF_AGILE))
        div = 110;
    else if (!ch->getProfession( )->getFlags( ).isSet(PROF_CASTER))
        div = 100;   
    else 
        return;
 
     dam += dam * number_percent()/div;
}
