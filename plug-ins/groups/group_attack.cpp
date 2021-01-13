/* $Id: group_attack.cpp,v 1.1.2.19.6.12 2010-09-01 21:20:44 rufina Exp $
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
#include "skillmanager.h"
#include "spelltemplate.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "material.h"
#include "fight.h"
#include "damage.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "vnum.h"
#include "act.h"
#include "def.h"

PROF(cleric);
PROF(paladin);
PROF(anti_paladin);

SPELL_DECL(BladeBarrier);
VOID_SPELL(BladeBarrier)::run(Character *ch, Character *victim, int sn, int level)
{
    // TO-DO: refactor this with unified damcalc function with multiple projectiles
    int dam;

    act("Множество острых клинков возникает вокруг $c2, поражая $C4.", ch, 0, victim, TO_NOTVICT);
    act("Вокруг тебя возникает множество острых клинков, поражая $C4.", ch, 0, victim, TO_CHAR);
    act("Множество острых клинков возникает вокруг $c2, поражая тебя!", ch, 0, victim, TO_VICT);
    dam = dice(level, 6);
    if (saves_spell(level, victim, DAM_PIERCE, ch, DAMF_SPELL))
        dam /= 2;
    damage_nocatch(ch, victim, dam, sn, DAM_PIERCE, true, DAMF_SPELL);

    act("Клинки со звоном ударяют в $c4!", victim, 0, 0, TO_ROOM);
    act("Острые клинки ударяют в тебя!", victim, 0, 0, TO_CHAR);
    dam = dice(level, 5);
    if (saves_spell(level, victim, DAM_PIERCE, ch, DAMF_SPELL))
        dam /= 2;
    damage_nocatch(ch, victim, dam, sn, DAM_PIERCE, true, DAMF_SPELL);

    if (number_percent() <= 55)
        return;

    act("Клинки со звоном ударяют в $c4!", victim, 0, 0, TO_ROOM);
    act("Острые клинки со звоном ударяют в тебя!", victim, 0, 0, TO_CHAR);
    dam = dice(level, 7);
    if (saves_spell(level, victim, DAM_PIERCE, ch, DAMF_SPELL))
        dam /= 2;
    damage_nocatch(ch, victim, dam, sn, DAM_PIERCE, true, DAMF_SPELL);

    if (number_percent() <= 50)
        return;

    act("Клинки со звоном ударяют в $c4!", victim, 0, 0, TO_ROOM);
    act("Острые клинки ударяют в тебя!", victim, 0, 0, TO_CHAR);
    dam = dice(level, 6);
    if (saves_spell(level, victim, DAM_PIERCE, ch, DAMF_SPELL))
        dam /= 3;
    damage_nocatch(ch, victim, dam, sn, DAM_PIERCE, true, DAMF_SPELL);

    if (victim->fighting != 0) {
        victim->setWaitViolence(number_bits(2) + 1);
        victim->position = POS_RESTING;
    }
}

SPELL_DECL(Bluefire);
VOID_SPELL(Bluefire)::run(Character *ch, Character *victim, int sn, int level)
{

    int dam;

    if (!ch->is_npc() && !IS_NEUTRAL(ch)) {
        victim = ch;
        ch->send_to("Твой {CГолубой огонь{x оборачивается против тебя!\n\r");
    }

    // Tier 2 damage: spells with regular or no special effects
    if (level <= 20)
        dam = dice(level, 8);
    else if (level <= 40)
        dam = dice(level, 12);
    else if (level <= 70)
        dam = dice(level, 15);
    else
        dam = dice(level, 18);

    if (number_percent() > 50) {
        if (saves_spell(level, victim, DAM_FIRE, ch, DAMF_SPELL))
            dam /= 2;
        if (victim != ch) {
            act_p("Голубой огонь $c2 {Rобжигает{x $C2!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
            act_p("Голубой огонь $c2 {Rобжигает{x тебя!", ch, 0, 0, TO_VICT, POS_RESTING);
            act_p("Твой Голубой огонь {Rобжигает{x $C2!", 0, 0, victim, TO_CHAR, POS_RESTING);
        }
        damage_nocatch(ch, victim, dam, sn, DAM_FIRE, true, DAMF_SPELL);
        fire_effect(victim, level, dam, TARGET_CHAR, DAMF_SPELL);
    } else {
        if (saves_spell(level, victim, DAM_COLD, ch, DAMF_SPELL))
            dam /= 2;
        if (victim != ch) {
            act_p("Голубой огонь $c2 {Cобмораживает{x $C2!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
            act_p("Голубой огонь $c2 {Cобмораживает{x тебя!", ch, 0, 0, TO_VICT, POS_RESTING);
            act_p("Твой Голубой огонь {Cобмораживает{x $C2!", 0, 0, victim, TO_CHAR, POS_RESTING);
        }
        damage_nocatch(ch, victim, dam, sn, DAM_COLD, true, DAMF_SPELL);
        cold_effect(victim, level, dam, TARGET_CHAR, DAMF_SPELL);
    }
}

SPELL_DECL(Demonfire);
VOID_SPELL(Demonfire)::run(Character *ch, Character *victim, int sn, int level)
{

    int dam;

    if (!ch->is_npc() && !IS_EVIL(ch)) {
        victim = ch;
        ch->send_to("Силы {RДемонов Ада{x оборачиваются против тебя!\n\r");
    }

    // Tier 2 damage: spells with regular or no special effects
    if (level <= 20)
        dam = dice(level, 8);
    else if (level <= 40)
        dam = dice(level, 12);
    else if (level <= 70)
        dam = dice(level, 15);
    else
        dam = dice(level, 18);

    if (number_percent() > 50) {
        if (saves_spell(level, victim, DAM_FIRE, ch, DAMF_SPELL))
            dam /= 2;
        if (victim != ch) {
            act_p("Демонический огонь $c2 {Rобжигает{x $C2!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
            act_p("Демонический огонь $c2 {Rобжигает{x тебя!", ch, 0, 0, TO_VICT, POS_RESTING);
            act_p("Твой Демонический огонь {Rобжигает{x $C2!", 0, 0, victim, TO_CHAR, POS_RESTING);
        }
        damage_nocatch(ch, victim, dam, sn, DAM_FIRE, true, DAMF_SPELL);
        fire_effect(victim, level, dam, TARGET_CHAR, DAMF_SPELL);
    } else {
        if (saves_spell(level, victim, DAM_NEGATIVE, ch, DAMF_SPELL))
            dam /= 2;
        if (victim != ch) {
            act_p("Демоны Ада, призванные $c5, {rтерзают{x $C2!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
            act_p("Демоны Ада, призванные $c5, {rтерзают{x тебя!", ch, 0, 0, TO_VICT, POS_RESTING);
            act_p("Демоны Ада, призванные тобой, {rтерзают{x $C2!", 0, 0, victim, TO_CHAR, POS_RESTING);
        }
        damage_nocatch(ch, victim, dam, sn, DAM_NEGATIVE, true, DAMF_SPELL);
        if (!IS_AFFECTED(victim, AFF_CURSE))
            spell(gsn_curse, level, ch, victim);
    }
}

SPELL_DECL(DispelEvil);
VOID_SPELL(DispelEvil)::run(Character *ch, Character *victim, int sn, int level)
{

    // TO-DO: combine with Dispel Good into Dispel Align
    int dam;

    if (!ch->is_npc() && IS_EVIL(ch)) {
        victim = ch;
        ch->send_to("Сила Добра оборачивается против тебя!\n\r");
    }

    if (IS_GOOD(victim)) {
        act_p("Силы Добра защищают $c4.", victim, 0, 0, TO_ROOM, POS_RESTING);
        act_p("Силы Добра тебя.", victim, 0, 0, TO_CHAR, POS_RESTING);
        return;
    }

    if (IS_NEUTRAL(victim)) {
        act_p("$C1 не чувствует этого.", ch, 0, victim, TO_CHAR, POS_RESTING);
        return;
    }

    if (ch->getProfession() == prof_cleric ||
        ch->getProfession() == prof_paladin ||
        ch->getProfession() == prof_anti_paladin) {
        // Tier 1 damage: only spells with severe limitations
        if (level <= 20)
            dam = dice(level, 10);
        else if (level <= 40)
            dam = dice(level, 13);
        else if (level <= 70)
            dam = dice(level, 16);
        else
            dam = dice(level, 20);
    } else {
        // Tier 3 damage: spells with powerful effects
        if (level <= 20)
            dam = dice(level, 7);
        else if (level <= 40)
            dam = dice(level, 10);
        else if (level <= 70)
            dam = dice(level, 13);
        else
            dam = dice(level, 16);
    }

    // works better on healthier victims
    if ((int)(victim->hit / victim->max_hit) * 100 < (int)(level / 2))
        dam /= 2;

    if (saves_spell(level, victim, DAM_HOLY, ch, DAMF_PRAYER))
        dam /= 2;

    damage_nocatch(ch, victim, dam, sn, DAM_HOLY, true, DAMF_PRAYER);
}

SPELL_DECL(DispelGood);
VOID_SPELL(DispelGood)::run(Character *ch, Character *victim, int sn, int level)
{
    // TO-DO: combine with Dispel Good into Dispel Align
    int dam;

    if (!ch->is_npc() && IS_GOOD(ch)) {
        victim = ch;
        ch->send_to("Сила Зла оборачивается против тебя!\n\r");
    }

    if (IS_EVIL(victim)) {
        act_p("Силы Зла защищают $c4.", victim, 0, 0, TO_ROOM, POS_RESTING);
        act_p("Силы Зла тебя.", victim, 0, 0, TO_CHAR, POS_RESTING);
        return;
    }

    if (IS_NEUTRAL(victim)) {
        act_p("$C1 не чувствует этого.", ch, 0, victim, TO_CHAR, POS_RESTING);
        return;
    }

    if (ch->getProfession() == prof_cleric ||
        ch->getProfession() == prof_paladin ||
        ch->getProfession() == prof_anti_paladin) {
        // Tier 1 damage: only spells with severe limitations
        if (level <= 20)
            dam = dice(level, 10);
        else if (level <= 40)
            dam = dice(level, 13);
        else if (level <= 70)
            dam = dice(level, 16);
        else
            dam = dice(level, 20);
    } else {
        // Tier 3 damage: spells with powerful effects
        if (level <= 20)
            dam = dice(level, 7);
        else if (level <= 40)
            dam = dice(level, 10);
        else if (level <= 70)
            dam = dice(level, 13);
        else
            dam = dice(level, 16);
    }

    // works better on healthier victims
    if ((int)(victim->hit / victim->max_hit) * 100 < (int)(level / 2))
        dam /= 2;

    if (saves_spell(level, victim, DAM_NEGATIVE, ch, DAMF_PRAYER))
        dam /= 2;

    damage_nocatch(ch, victim, dam, sn, DAM_NEGATIVE, true, DAMF_PRAYER);
}

SPELL_DECL(Earthquake);
VOID_SPELL(Earthquake)::run(Character *ch, Room *room, int sn, int level)
{

    int dam;

    ch->send_to("Земля дрожит под твоими ногами!\n\r");
    act("$c1 вызывает ураган и землетрясение.", ch, 0, 0, TO_ROOM);

    area_message(ch, "Земля слегка дрожит под твоими ногами.", true);

    for (auto &vch : room->getPeople()) {
        if (!vch->isDead() && vch->in_room == room) {

            if (DIGGED(vch) && vch->was_in_room->area == room->area)
                if (!is_safe_nomessage(ch, vch) && number_percent() < ch->getSkill(sn) / 2)
                    undig_earthquake(vch);

            if (ch == vch)
                continue;

            if (is_safe_spell(ch, vch, true))
                continue;

            if (vch->is_mirror() && number_percent() < 50)
                continue;

            if (is_flying(vch))
                continue;

            // Tier 3 damage: spells with powerful effects
            if (level <= 20)
                dam = dice(level, 7);
            else if (level <= 40)
                dam = dice(level, 10);
            else if (level <= 70)
                dam = dice(level, 13);
            else
                dam = dice(level, 16);

            switch (room->sector_type) {
            case SECT_MOUNTAIN:
                dam *= 4;
                break;
            case SECT_CITY:
                dam *= 3;
                break;
            case SECT_INSIDE:
                dam *= 2;
                break;
            }

            try {
                damage_nocatch(ch, vch, dam, sn, DAM_BASH, true, DAMF_SPELL);
                if (number_percent() < ch->getSkill(sn) / 2) {
                    vch->setWaitViolence(2);
                    vch->position = POS_RESTING;
                }
            } catch (const VictimDeathException &) {
                continue;
            }
        }
    }
}

SPELL_DECL(Web);
VOID_SPELL(Web)::run(Character *ch, Character *victim, int sn, int level)
{

    Affect af;

    if (victim->isAffected(sn)) {
        if (victim == ch)
            ch->send_to("Ты и так в паутине.\n\r");
        else
            act_p("Густая паутина уже сковала движения $C2.", ch, 0, victim, TO_CHAR, POS_RESTING);
        return;
    }

    act_p("$c1 пытается опутать $C4 энергетической паутиной!",
          ch, 0, victim, TO_NOTVICT, POS_RESTING);
    act_p("Ты пытаешься опутать $C4 энергетической паутиной!",
          ch, 0, victim, TO_CHAR, POS_RESTING);
    act_p("$c1 пытается опутать тебя энергетической паутиной!",
          ch, 0, victim, TO_VICT, POS_RESTING);

    if (saves_spell(level, victim, DAM_ENERGY, ch, DAMF_SPELL)) {
        act_p("$C1 с легкостью уворачивается от сетей.", ch, 0, victim, TO_CHAR, POS_RESTING);
        act_p("$C1 с легкостью уворачивается от сетей.", ch, 0, victim, TO_NONVICT, POS_RESTING);
        victim->send_to("Ты с легкостью уворачиваешься от сетей.\n\r");
        return;
    }

    af.type = sn;
    af.level = level;
    af.duration = 1 + level / 30;
    af.location = APPLY_HITROLL;
    af.modifier = -1 * (level / 6);
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = -1 * (level / 6);
    affect_to_char(victim, &af);

    af.location = APPLY_DEX;
    af.modifier = -1 - level / 40;
    af.bitvector.setTable(&detect_flags);
    af.bitvector.setValue(ADET_WEB);
    affect_to_char(victim, &af);

    if (ch != victim)
        act_p("Ты сковываешь движения $C4 густой сетью!", ch, 0, victim, TO_CHAR, POS_RESTING);

    act_p("$C1 сковывает движения $C4 густой сетью!", ch, 0, victim, TO_NONVICT, POS_RESTING);
    victim->send_to("Густая сеть сковывает твои движения!\n\r");
}

SPELL_DECL(HeatMetal);
VOID_SPELL(HeatMetal)::run(Character *ch, Character *victim, int sn, int level)
{

    Object *obj_lose, *obj_next;
    int dam = 0;
    bool fail = true;

    if (!saves_spell(level - 2, victim, DAM_FIRE, ch, DAMF_PRAYER)) {
        for (obj_lose = victim->carrying;
             obj_lose != 0;
             obj_lose = obj_next) {
            obj_next = obj_lose->next_content;
            if (number_range(1, 2 * level) > obj_lose->level && !saves_spell(level, victim, DAM_FIRE, ch, DAMF_PRAYER) && material_is_typed(obj_lose, MAT_METAL) && !IS_OBJ_STAT(obj_lose, ITEM_BURN_PROOF)) {
                switch (obj_lose->item_type) {
                case ITEM_ARMOR:
                    if (obj_lose->wear_loc != wear_none) /* remove the item */
                    {
                        if (can_drop_obj(victim, obj_lose) && (obj_lose->weight / 10) > number_range(1, 2 * victim->getCurrStat(STAT_DEX)) && obj_lose->wear_loc->remove(obj_lose, 0)) {
                            act_p("$c1 кричит от боли и бросает $o4 на землю!",
                                  victim, obj_lose, 0, TO_ROOM, POS_RESTING);
                            act_p("Ты кричишь от боли и бросаешь $o4 на землю!",
                                  victim, obj_lose, 0, TO_CHAR, POS_RESTING);
                            dam += (number_range(1, obj_lose->level) / 2);
                            obj_from_char(obj_lose);
                            obj_to_room(obj_lose, victim->in_room);
                            fail = false;
                        } else /* stuck on the body! ouch! */
                        {
                            act_p("$o1 обжигает твою кожу!",
                                  victim, obj_lose, 0, TO_CHAR, POS_RESTING);
                            dam += (number_range(1, obj_lose->level));
                            fail = false;
                        }

                    } else /* drop it if we can */
                    {
                        if (can_drop_obj(victim, obj_lose)) {
                            act_p("$c1 кричит от боли и бросает $o4 на землю!",
                                  victim, obj_lose, 0, TO_ROOM, POS_RESTING);
                            act_p("Ты кричишь от боли и бросаешь $o4 на землю!",
                                  victim, obj_lose, 0, TO_CHAR, POS_RESTING);
                            dam += (number_range(1, obj_lose->level) / 2);
                            obj_from_char(obj_lose);
                            obj_to_room(obj_lose, victim->in_room);
                            fail = false;
                        } else /* cannot drop */
                        {
                            act_p("$o1 обжигает твою кожу!",
                                  victim, obj_lose, 0, TO_CHAR, POS_RESTING);
                            dam += (number_range(1, obj_lose->level) / 2);
                            fail = false;
                        }
                    }
                    break;
                case ITEM_WEAPON:
                    if (obj_lose->wear_loc != wear_none) /* try to drop it */
                    {
                        if (IS_WEAPON_STAT(obj_lose, WEAPON_FLAMING))
                            continue;

                        if (can_drop_obj(victim, obj_lose) && obj_lose->wear_loc->remove(obj_lose, 0)) {
                            act_p("$o1 выпадает из обожженных рук $c2.",
                                  victim, obj_lose, 0, TO_ROOM, POS_RESTING);
                            victim->send_to("Оружие выпадает из твоих обожженных рук!\n\r");
                            dam += 1;
                            obj_from_char(obj_lose);
                            obj_to_room(obj_lose, victim->in_room);
                            fail = false;
                        } else /* YOWCH! */
                        {
                            victim->send_to("Раскаленное оружие обжигает твои руки!\n\r");
                            dam += number_range(1, obj_lose->level);
                            fail = false;
                        }
                    } else /* drop it if we can */
                    {
                        if (can_drop_obj(victim, obj_lose)) {
                            victim->pecho("%1$^O1 раскаляется, и ты бросаешь %1$P2 на землю.", obj_lose);
                            victim->recho("%1$^O1 раскаляется, и %2$C1 бросает %1$P2 на землю.", obj_lose, victim);
                            dam += (number_range(1, obj_lose->level) / 2);
                            obj_from_char(obj_lose);
                            obj_to_room(obj_lose, victim->in_room);
                            fail = false;
                        } else /* cannot drop */
                        {
                            act_p("$o1 обжигает тебя!",
                                  victim, obj_lose, 0, TO_CHAR, POS_RESTING);
                            dam += (number_range(1, obj_lose->level) / 2);
                            fail = false;
                        }
                    }
                    break;
                }
            }
        }
    }
    if (fail) {
        ch->send_to("Твоя попытка нагревания закончилась неудачей.\n\r");
        victim->send_to("Ты чувствуешь легкое прикосновение тепла.\n\r");
    } else /* damage! */
    {
        dam = 2 * dam / 3;
        damage_nocatch(ch, victim, dam, sn, DAM_FIRE, true, DAMF_PRAYER);
    }
}

SPELL_DECL(Holycross);
VOID_SPELL(Holycross)::run(Character *ch, Object *grave, int sn, int level)
{
    int dam;
    PCMemoryInterface *pcm;
    PCharacter *victim;

    if ((ch->getProfession() != prof_cleric && ch->getProfession() != prof_paladin) || IS_EVIL(ch)) {
        ch->send_to("Ты не владеешь этой силой.\r\n");
        return;
    }

    if (grave->pIndexData->vnum != OBJ_VNUM_GRAVE) {
        ch->send_to("Сюда не воткнется, это не могила.\r\n");
        return;
    }

    pcm = PCharacterManager::find(DLString(grave->getOwner()));

    if (!pcm || (victim = dynamic_cast<PCharacter *>(pcm)) == 0 || !DIGGED(victim)) {
        ch->send_to("В этой могиле никого нет.\r\n");
        LogStream::sendError() << "Unexistent grave owner: " << grave->getOwner() << endl;
        return;
    }

    if (number_percent() > ch->getSkill(sn)) {
        act_p("$c1 втыкает в могилу крест, но он падает набок.", ch, 0, 0, TO_ROOM, POS_RESTING);
        act_p("Ты втыкаешь в могилу крест, но он падает набок.", ch, 0, 0, TO_CHAR, POS_RESTING);
        return;
    }

    act_p("$c1 втыкает в могилу священный крест!", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p("Ты втыкаешь в могилу священный крест!", ch, 0, 0, TO_CHAR, POS_RESTING);
    act_p("Из-под земли раздается раздирающий душу вопль!", ch, 0, 0, TO_ALL, POS_RESTING);

    undig(victim);

    // Tier 1 damage: only spells with severe limitations
    if (level <= 20)
        dam = dice(level, 10);
    else if (level <= 40)
        dam = dice(level, 13);
    else if (level <= 70)
        dam = dice(level, 16);
    else
        dam = dice(level, 20);

    damage_nocatch(ch, victim, dam, sn, DAM_HOLY, true, DAMF_PRAYER);
}
