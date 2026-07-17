#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "commandtemplate.h"
#include "skillreference.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "fight.h"
#include "arg_utils.h"
#include "raceflags.h"
#include "interp.h"
#include "occupations.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

GSN(gratitude);
PROF(anti_paladin);

bool omprog_give( Object *obj, Character *ch, Character *victim );

CMDRUNP(request)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Character* victim;
    Object* obj;

    if (ch->isAffected(gsn_gratitude))
    {
        ch->pecho(_("Подожди немного."));
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->is_npc())
        return;

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        ch->pecho(_("Что и у кого ты хочешь попросить?"));
        return;
    }

    if ((victim = get_char_room(ch, arg2)) == 0)
    {
        ch->pecho(_("Здесь таких нет."));
        return;
    }

    if (!victim->is_npc())
    {
        ch->pecho(_("На игроков такие штучки не пройдут. Просто поговори с ними!"));
        return;
    }

    if (victim->position <= POS_SLEEPING)
    {
        oldact(_("$C1 не в состоянии выполнить твою просьбу."), ch, 0, victim, TO_CHAR);
        return;
    }

    if (ch->move < (50 + ch->getRealLevel()))
    {
        do_say(victim, "Ты выглядишь устало, может, отдохнешь сначала?");
        return;
    }

    Flags att = victim->getRace()->getAttitude(*ch->getRace());

    /* Donating races (e.g. centaurs) donate regardless of alignment.
     * Otherwise good mobs would donate to good players.
     */
    if (!att.isSet(RACE_DONATES))
    {
        if (!IS_GOOD(ch) || !IS_GOOD(victim))
        {
            if (IS_EVIL(ch) && !IS_EVIL(victim))
            {
                do_say(victim, "У тебя нечистая душа, я ничего тебе не дам!");
            }
            else
            {
                do_say(victim, "Я не дам тебе ничего!");
            }
            if (ch->getModifyLevel() > 30 && number_percent() > 75)
            {
                interpret_raw(victim, "murder", ch->getNameC());
            }
            return;
        }
    }

    ch->setWaitViolence(1);
    ch->move -= 10;
    ch->move = max((int)ch->move, 0);

    if (victim->getModifyLevel() >= ch->getModifyLevel() + 10 || victim->getModifyLevel() >= ch->getModifyLevel() * 2)
    {
        say_fmt(_("Всему свое время, малыш%2$G||ка."), victim, ch);
        return;
    }

    if (((obj = get_obj_carry(victim, arg1)) == 0
        && (obj = get_obj_wear(victim, arg1)) == 0))
    {
        do_say(victim, "Извини, у меня нет этого.");
        return;
    }

    if (IS_SET(obj->extra_flags, ITEM_INVENTORY)
        && victim->getNPC()->behavior
        && IS_SET(victim->getNPC()->behavior->getOccupation(), (1 << OCC_SHOPPER)))
    {
        say_fmt(_("Если тебе нравится %3$O1, ты можешь у меня %3$P2 купить."), victim, ch, obj);
        return;
    }

    if (!Item::canDrop(ch, obj)
        || (obj_is_worn(obj) && IS_OBJ_STAT(obj, ITEM_NOREMOVE)))
    {
        do_say(victim, "Извини, но эта вещь проклята, и я не могу избавиться от нее.");
        return;
    }

    if (ch->carry_number + obj->getNumber() > Char::canCarryNumber(ch))
    {
        ch->pecho(_("Твои руки полны."));
        return;
    }

    if (Char::getCarryWeight(ch) + obj->getWeight() > Char::canCarryWeight(ch))
    {
        ch->pecho(_("Ты не можешь нести такой вес."));
        return;
    }

    if (!ch->can_see(obj))
    {
        ch->pecho(_("Ты не видишь этого."));
        return;
    }
    if (!victim->can_see(ch))
    {
        do_say(victim, "Извини, я не вижу тебя.");
        return;
    }

    if (!victim->can_see(obj))
    {
        do_say(victim, "Извини, я не вижу этой вещи.");
        return;
    }



    if (obj->pIndexData->vnum == 520) // Knight's key
    {
        ch->pecho(_("Извини, он не отдаст тебе это."));
        return;
    }

    if (is_safe(ch, victim))
    {
        return;
    }

    if (obj->pIndexData->limit >= 0 && obj->isAntiAligned(ch))
    {
        ch->pecho(_("%2$^s не позволяют тебе завладеть %1$O5."),
            obj,
            IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
        return;
    }


    obj_from_char(obj);
    obj_to_char(obj, ch);
    oldact(_("$c1 просит $o4 у $C2."), ch, obj, victim, TO_NOTVICT);
    oldact(_("Ты просишь $o4 у $C2."), ch, obj, victim, TO_CHAR);
    oldact(_("$c1 просит $o4 у тебя."), ch, obj, victim, TO_VICT);

    omprog_give(obj, victim, ch);

    ch->move -= (50 + ch->getModifyLevel());
    ch->move = max((int)ch->move, 0);
    ch->hit -= 3 * (ch->getModifyLevel() / 2);
    ch->hit = max((int)ch->hit, 0);

    oldact(_("Ты чувствуешь благодарность за доверие $C2."), ch, 0, victim, TO_CHAR);
    postaffect_to_char(ch, gsn_gratitude, ch->getModifyLevel() / 10);
}



CMDRUNP(demand)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Character* victim;
    Object* obj;
    int chance;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->is_npc())
        return;

    if (ch->getProfession() != prof_anti_paladin)
    {
        ch->pecho(_("Ты никого не запугаешь своим видом."));
        return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        ch->pecho(_("Потребовать что и у кого?"));
        return;
    }

    if ((victim = get_char_room(ch, arg2)) == 0)
    {
        ch->pecho(_("Таких тут нет."));
        return;
    }

    if (!victim->is_npc()) {
        ch->pecho(_("Просто убей и отбери."));
        return;
    }

    if (IS_SET(victim->act, ACT_NODEMAND)) {
        oldact(_("$C1 не подчинится твоему требованию."), ch, 0, victim, TO_CHAR);
        return;
    }

    if (victim->position <= POS_SLEEPING)
    {
        oldact(_("$C1 не в состоянии исполнить твой приказ."), ch, 0, victim, TO_CHAR);
        return;
    }

    if (victim->getNPC()->behavior
        && IS_SET(victim->getNPC()->behavior->getOccupation(), (1 << OCC_SHOPPER)))
    {
        ch->pecho(_("Хочешь -- купи!"));
        return;
    }

    ch->setWaitViolence(1);

    chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
    chance += (ch->getCurrStat(STAT_CHA) - 15) * 10;
    chance += ch->getModifyLevel() - victim->getModifyLevel();

    if (victim->getModifyLevel() >= ch->getModifyLevel() + 10 || victim->getModifyLevel() >= ch->getModifyLevel() * 2)
        chance = 0;

    if (number_percent() > chance) {
        do_say(victim, "Я не собираюсь ничего отдавать тебе!");
        interpret_raw(victim, "murder", ch->getNameC());
        return;
    }

    if (((obj = get_obj_carry(victim, arg1)) == 0
        && (obj = get_obj_wear(victim, arg1)) == 0)
        || IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
        do_say(victim, "Извини, у меня нет этого.");
        return;
    }

    if (!Item::canDrop(ch, obj)
        || (obj_is_worn(obj) && IS_OBJ_STAT(obj, ITEM_NOREMOVE)))
    {
        do_say(victim,
            "Эта вещь проклята, и я не могу избавиться от нее.");
        return;
    }


    if (ch->carry_number + obj->getNumber() > Char::canCarryNumber(ch))
    {
        ch->pecho(_("Твои руки полны."));
        return;
    }

    if (Char::getCarryWeight(ch) + obj->getWeight() > Char::canCarryWeight(ch))
    {
        ch->pecho(_("Ты не сможешь нести такую тяжесть."));
        return;
    }

    if (!ch->can_see(obj))
    {
        oldact(_("Ты не видишь этого."), ch, 0, victim, TO_CHAR);
        return;
    }

    if (!victim->can_see(ch))
    {
        do_say(victim, "Извини, я не вижу тебя.");
        return;
    }

    if (!victim->can_see(obj))
    {
        do_say(victim, "Извини, я не вижу этой вещи.");
        return;
    }

    if (is_safe(ch, victim))
    {
        return;
    }

    if (obj->pIndexData->limit >= 0 && obj->isAntiAligned(ch))
    {
        ch->pecho(_("%2$^s не позволяют тебе завладеть %1$O5."),
            obj,
            IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
        return;
    }

    oldact(_("$c1 требует $o4 у $C2."), ch, obj, victim, TO_NOTVICT);
    oldact(_("Ты требуешь $o4 у $C2."), ch, obj, victim, TO_CHAR);
    oldact(_("$c1 требует у тебя $o4."), ch, obj, victim, TO_VICT);

    obj_from_char(obj);
    obj_to_char(obj, ch);

    omprog_give(obj, victim, ch);

    ch->pecho(_("Твое могущество повергает всех в трепет."));
}

