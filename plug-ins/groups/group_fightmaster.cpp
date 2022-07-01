
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
#include "vnum.h"

GSN(area_attack);
GSN(bash);
GSN(bash_door);
GSN(cavalry);
GSN(crush);
GSN(double_kick);
GSN(kick);
GSN(protective_shield);
GSN(smash);

CLAN(shalafi);
PROF(anti_paladin);
PROF(samurai);

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
            DLString petName = Syntax::noun(ch->getNameP('1'));
            ch->master->pecho(fmt(0, "%1$^C1 упа%1$Gло|л|ла и не может ходить и выполнять некоторые команды. Напиши {y{hc{lRприказать %2$s встать{lEorder %2$s stand{x, если хочешь продолжить выбивать %1$Gим|им|ей двери.", ch, petName.c_str()));
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
