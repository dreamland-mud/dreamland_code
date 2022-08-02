
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

/*
 * 'bash door' skill command
 */

SKILL_RUNP(bashdoor)
{
    char arg[MAX_INPUT_LENGTH];
    Character *gch;
    int chance = 0;
    EXTRA_EXIT_DATA *peexit = 0;
    int damage_bash, door = 0;
    Room *room = ch->in_room;

    one_argument(argument, arg);

    if (MOUNTED(ch)) {
        ch->pecho("Только не верхом!");
        return;
    }

    if (RIDDEN(ch)) {
        ch->pecho("Ты не можешь выбить дверь, когда оседлан{Sfа{Sx.");
        return;
    }

    if (arg[0] == '\0') {
        ch->pecho("Выбить дверь в каком направлении?");
        return;
    }

    if (ch->fighting) {
        ch->pecho("Сначала закончи сражение.");
        return;
    }

    peexit = room->extra_exits.find(arg);
    if ((!peexit || !ch->can_see(peexit)) && (door = find_exit(ch, arg, FEX_NO_INVIS | FEX_DOOR | FEX_NO_EMPTY)) < 0) {
        ch->pecho("Но тут нечего выбивать!");
        return;
    }

    int slevel = skill_level(*gsn_bash_door, ch);

    /* look for guards */
    for (gch = room->people; gch; gch = gch->next_in_room) {
        if (gch->is_npc() && IS_AWAKE(gch) && slevel + 5 < gch->getModifyLevel()) {
            oldact("$C1 стоит слишком близко к двери.", ch, 0, gch, TO_CHAR);
            return;
        }
    }

    // 'bash door'
    EXIT_DATA *pexit = 0;
    EXIT_DATA *pexit_rev = 0;
    int exit_info;

    if (peexit != 0) {
        door = DIR_SOMEWHERE;
        exit_info = peexit->exit_info;
    } else {
        pexit = room->exit[door];
        exit_info = pexit->exit_info;
    }

    if (!IS_SET(exit_info, EX_CLOSED)) {
        ch->pecho("Здесь уже открыто.");
        return;
    }

    if (!IS_SET(exit_info, EX_LOCKED)) {
        ch->pecho("Просто попробуй открыть.");
        return;
    }

    if (IS_SET(exit_info, EX_NOPASS) && !IS_SET(exit_info, EX_BASH_ONLY)) {
        ch->pecho("Эту дверь невозможно вышибить.");
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->getCarryWeight() / 100;

    chance += (ch->size - 2) * 20;

    /* stats */
    chance += ch->getCurrStat(STAT_STR);

    if (is_flying(ch))
        chance -= 10;

    /* level
        chance += ch->getModifyLevel() / 10;
        */

    chance += (gsn_bash_door->getEffective(ch) - 90 + skill_level_bonus(*gsn_bash_door, ch));
    const char *doorname = peexit ? peexit->short_desc_from : direction_doorname(pexit);
    oldact("Ты бьешь в $N4, пытаясь выбить!", ch, 0, doorname, TO_CHAR);
    oldact("$c1 бьет в $N4, пытаясь выбить!", ch, 0, doorname, TO_ROOM);

    if (room->isDark() && !IS_AFFECTED(ch, AFF_INFRARED))
        chance /= 2;

    chance = URANGE(3, chance, 98);

    /* now the attack */
    if (number_percent() < chance) {
        gsn_bash_door->improve(ch, true);

        if (peexit != 0) {
            REMOVE_BIT(peexit->exit_info, EX_LOCKED);
            REMOVE_BIT(peexit->exit_info, EX_CLOSED);
            oldact("$c1 выбивает дверь.", ch, 0, 0, TO_ROOM);
            oldact("Ты выбиваешь дверь!", ch, 0, 0, TO_CHAR);
        } else {
            REMOVE_BIT(pexit->exit_info, EX_LOCKED);
            REMOVE_BIT(pexit->exit_info, EX_CLOSED);
            oldact("$c1 выбивает дверь.", ch, 0, 0, TO_ROOM);
            oldact("Ты выбиваешь дверь!", ch, 0, 0, TO_CHAR);

            /* open the other side */
            if ((pexit_rev = direction_reverse(room, door))) {
                REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
                REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
                direction_target(room, door)->echo(POS_RESTING, "%^N1 с грохотом вылетает.", doorname);
            }
        }

    } else {
        oldact("Обессилев, ты падаешь лицом вниз!", ch, 0, 0, TO_CHAR);
        oldact("Обессилев, $c1 упа$gло|л|ла лицом вниз.", ch, 0, 0, TO_ROOM);
        gsn_bash_door->improve(ch, false);
        ch->position = POS_RESTING;
        damage_bash = ch->damroll + number_range(4, 4 + 4 * ch->size + chance / 5);
        damage(ch, ch, damage_bash, gsn_bash_door, DAM_BASH, true, DAMF_WEAPON);
        if (IS_CHARMED(ch) && ch->master->getPC()) {
            DLString petName = Syntax::noun(ch->toNoun()->getFullForm());
            ch->master->pecho(fmt(0, "%1$^C1 упа%1$Gло|л|ла и не может ходить и выполнять некоторые команды. Напиши {y{hc{lRприказать %2$N3 встать{lEorder %2$N1 stand{x, если хочешь продолжить выбивать %1$Gим|им|ей двери.", ch, petName.c_str()));
        }
    }

    return;
}



/*
 * 'kick' skill command
 */
SKILL_RUNP(kick)
{
    Character *victim;
    int chance;
    char arg[MAX_INPUT_LENGTH];
    bool FightingCheck;

    if (MOUNTED(ch)) {
        ch->pecho("Ты не можешь ударить ногой, если ты верхом!");
        return;
    }

    if (ch->fighting != 0)
        FightingCheck = true;
    else
        FightingCheck = false;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
        victim = ch->fighting;
        if (victim == 0) {
            ch->pecho("Сейчас ты не сражаешься!");
            return;
        }
    } else if ((victim = get_char_room(ch, arg)) == 0) {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (victim == ch) {
        ch->pecho("Ударить себя ногой? Довольно тяжело...");
        return;
    }

    if (is_safe(ch, victim)) {
        return;
    }

    if (IS_CHARMED(ch) && ch->master == victim) {
        oldact("Но $C1 твой друг!!!", ch, 0, victim, TO_CHAR);
        return;
    }

    if (SHADOW(ch)) {
        ch->pecho("Твоя нога вязнет в твоей тени...");
        oldact_p("$c1 выделывает балетные па перед своей тенью.",
                 ch, 0, 0, TO_ROOM, POS_RESTING);
        return;
    }

    chance = number_percent();

    if (is_flying(ch))
        chance = (int)(chance * 1.1);

    if (chance < gsn_kick->getEffective(ch)) {
        gsn_kick->improve(ch, true, victim);

        Object *on_feet;
        int dam = number_range(1, ch->getModifyLevel());

        if ((ch->getProfession() == prof_samurai) && IS_SET(ch->parts, PART_FEET) && ((on_feet = get_eq_char(ch, wear_feet)) == 0 || (on_feet != 0 && !material_is_typed(on_feet, MAT_METAL)))) {
            dam *= 2;
        }

        dam += ch->damroll / 2;
        damapply_class(ch, dam);

        //10% extra damage for every skill level
        dam += dam * skill_level_bonus(*gsn_kick, ch) / 10;

        if (IS_SET(ch->parts, PART_TWO_HOOVES))
            dam = 3 * dam / 2;
        else if (IS_SET(ch->parts, PART_FOUR_HOOVES))
            dam *= 2;

        try {
            damage_nocatch(ch, victim, dam, gsn_kick, DAM_BASH, true, DAMF_WEAPON);
        } catch (const VictimDeathException &) {
            return;
        }

        if (number_percent() < (gsn_double_kick->getEffective(ch) * 8) / 10) {
            gsn_double_kick->improve(ch, true, victim);

            Object *on_feet;
            int dam = number_range(1, ch->getModifyLevel());

            if ((ch->getProfession() == prof_samurai) && IS_SET(ch->parts, PART_FEET) && ((on_feet = get_eq_char(ch, wear_feet)) == 0 || (on_feet != 0 && !material_is_typed(on_feet, MAT_METAL)))) {
                dam *= 2;
            }

            dam += ch->damroll / 2;
            damapply_class(ch, dam);

            //10% extra damage for every skill level
            dam += dam * skill_level_bonus(*gsn_double_kick, ch) / 10;

            if (IS_SET(ch->parts, PART_TWO_HOOVES))
                dam = 3 * dam / 2;
            else if (IS_SET(ch->parts, PART_FOUR_HOOVES))
                dam *= 2;

            try {
                damage_nocatch(ch, victim, dam, gsn_double_kick, DAM_BASH, true, DAMF_WEAPON);
            } catch (const VictimDeathException &) {
                return;
            }
        }

    } else {
        damage(ch, victim, 0, gsn_kick, DAM_BASH, true, DAMF_WEAPON);
        gsn_kick->improve(ch, false, victim);
    }

    if (!FightingCheck) {
        if (IS_SET(ch->parts, PART_TWO_HOOVES | PART_FOUR_HOOVES))
            yell_panic(ch, victim,
                       "Помогите! Кто-то ударил меня копытом!",
                       "Помогите! %1$^C1 удари%1$Gло|л|ла меня копытом!");
        else
            yell_panic(ch, victim,
                       "Помогите! Кто-то ударил меня ногой!",
                       "Помогите! %1$^C1 удари%1$Gло|л|ла меня ногой!");
    }
}




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
        // TODO: add a method to output messages with CONFIG_FIGHTSPAM check to Fenia skill commands
        victim->pecho( "Ты пытаешься парировать атаку, но путаешься в своей тени." );
        ch->pecho( "%2$^C1 постоянно путается в своей тени.", ch, victim );
        ch->recho( victim, "%2$^C1 постоянно путается в своей тени.", ch, victim );
        return false;
    }
    
    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && (!defending_weapon 
            || !IS_WEAPON_STAT(defending_weapon, WEAPON_HOLY))) 
    {
        victim->pecho( "%3$^O1 проходит насквозь через твою попытку спарировать!", ch, victim, wield );
        
        if (defending_weapon) {
            ch->pecho( "%3$^O1 проходит сквозь оружие %2$C2!", ch, victim, wield );
            ch->recho( victim, "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!", ch, victim, wield );
        }
        else {
            ch->pecho( "%3$^O1 проходит сквозь руки %2$C2!", ch, victim, wield );
            ch->recho( victim, "%3$^O1 %1$C2 проходит сквозь руки %2$C2!", ch, victim, wield );
        }

        return false;
    }

    victim->pecho( "Ты парируешь атаку %1$C2.", ch, victim );
    ch->pecho( "%2$^C1 парирует твою атаку.", ch, victim );
    ch->recho( victim, "%2$^C1 парирует атаку %1$C2.", ch, victim );

    // TODO: damage_to_obj is called here with a chance
    // destroyWeapon( );

    if ( number_percent() >  gsn_parry->getEffective( victim ) )
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
        victim->pecho( "Тень пинает тебя." );
        ch->pecho( "Тень и %2$C1 играют в кунг-фу.", ch, victim );
        ch->recho( victim, "Тень и %2$C1 играют в кунг-фу.", ch, victim );
        return false;
    }

    victim->pecho( "Ты отражаешь руками атаку %1$C2.", ch, victim );
    ch->pecho( "%2$^C1 отражает руками твою атаку.", ch, victim );
    ch->recho( victim, "%2$^C1 отражает руками атаку %1$C2.", ch, victim );

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
        victim->pecho( "Стая летучих мышей вокруг тебя сбита с толку твоей тенью." );
        ch->pecho( "Стая летучих мышей вокруг %2$C2 сбита с толку тенью.", ch, victim );
        ch->recho( victim, "Стая летучих мышей вокруг %2$C2 сбита с толку тенью.", ch, victim );
        return false;
    }
    
    ch->pecho( "Ты не смо%1$Gгло|г|гла пробиться сквозь стаю летучих мышей, кружащихся вокруг %2$C2.", ch, victim);
    victim->pecho( "Стая летучих мышей не позволяет %1$C3 повредить тебе.", ch, victim );
    ch->recho( victim, "%1$^C1 пытается разогнать стаю летучих мышей вокруг %2$C2.", ch, victim );

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
        victim->pecho( "Ты впустую машешь щитом перед твоей тенью." );
        victim->recho( "%2$^C1 впустую размахивает щитом перед своей тенью.", ch, victim);
        return false;
    }

    victim->pecho( "Ты отражаешь щитом атаку %1$C2.", ch, victim );
    ch->pecho( "%2$^C1 отражает твою атаку %2$P2 щитом.", ch, victim );
    ch->recho( victim, "%2$^C1 отражает атаку %1$C2 своим щитом.", ch, victim );

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
        victim->pecho( "Тень запутывает тебя." );
        victim->recho( "Тень %2$C2 лезет со своими советам.", ch, victim );
        return false;
    }

    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && !IS_WEAPON_STAT(def1, WEAPON_HOLY) 
        && !IS_WEAPON_STAT(def2, WEAPON_HOLY)) 
    {
        victim->pecho( "%3$^O1 проходит насквозь через твою попытку кросс-блокировать!", ch, victim, wield );
        ch->pecho( "%3$^O1 проходит сквозь оружие %2$C2!", ch, victim, wield );
        ch->recho( victim, "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!", ch, victim, wield );

        return false;
    }
    
    victim->pecho( "Ты кросс-блокируешь атаку %1$C2.", ch, victim );
    ch->pecho( "%2$^C1 кросс-блокирует твою атаку.", ch, victim );
    ch->recho( victim, "%2$^C1 кросс-блокирует атаку %1$C2.", ch, victim );

    // TODO call damage_to_obj for the wield
    // destroyWeapon( );

    if ( number_percent() >  gsn_cross_block->getEffective( victim ) )
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
SKILL_APPLY(dodge)
{
    int chance, prof;
    Object *wield;
    bool secondary = !!level;

    if ( !IS_AWAKE(victim) )
        return false;

    if ( MOUNTED(victim) )
        return false;
    
    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;

    wield = get_wield(ch, secondary);
    chance  = gsn_dodge->getEffective( victim ) / 2;

    /* chance for high dex. */
    chance += 2 * (victim->getCurrStat(STAT_DEX) - 20);
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += chance / 5;
    else if (prof == prof_thief || prof == prof_ninja)
        chance += chance / 10;
    else if (prof == prof_anti_paladin)
        chance -= chance / 10;

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
        && (gsn_forest_fighting->getEffective( victim ) > number_percent( ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }

    if (wield && (wield->value0() == WEAPON_FLAIL || wield->value0() == WEAPON_WHIP))
        chance = ( int )( chance * 1.2 );
        
    if (number_percent( ) >= chance + ( skill_level(*gsn_dodge, victim) - ch->getModifyLevel() ) / 2
        || ( !victim->is_npc() && !victim->move ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        victim->pecho( "Ты скачешь вокруг своей тени, пытаясь от нее увернуться." );
        victim->recho( "%2$^C1 забавно прыгает вокруг своей тени.", ch, victim );
        return false;
    }

    victim->pecho( "Ты уворачиваешься от атаки %1$C2.", ch, victim );
    ch->pecho( "%2$^C1 уворачивается от твоей атаки.", ch, victim );
    ch->recho( victim, "%2$^C1 уворачивается от атаки %1$C2.", ch, victim );

    if ( number_percent() < (gsn_dodge->getEffective( victim ) / 20 )
        && !(is_flying( ch ) || ch->position < POS_FIGHTING) )
    {
        /* size */
        if (victim->size < ch->size)
            chance += (victim->size - ch->size) * 10;  /* bigger = harder to trip */

        /* dex */
        chance += victim->getCurrStat(STAT_DEX);
        chance -= ch->getCurrStat(STAT_DEX) * 3 / 2;

        if (is_flying( victim ) )
            chance -= 10;

        /* speed */
        if (IS_QUICK(victim))
            chance += 10;

        if (IS_QUICK(ch))
            chance -= 20;

        /* level */
            chance += ( victim->getModifyLevel() - ch->getModifyLevel() ) * 2;

        /* now the attack */
        if ( number_percent() < (chance / 20) )
        {
            oldact("$c1 теряет равновесие и падает вниз!", ch,0,victim,TO_VICT);
            oldact("$C1 уворачивается от твоей атаки, ты теряешь равновесие, и падаешь вниз!", ch,0,victim,TO_CHAR);
            oldact("$C1 уворачивается от атаки $c2, $c1 теряет равновесие и падает вниз.", ch,0,victim,TO_NOTVICT);

            ch->setWait(gsn_trip->getBeats(ch));
            ch->position = POS_RESTING;
        }
    }

    gsn_dodge->improve( victim, true, ch );
    return true;
}