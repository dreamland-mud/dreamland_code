/* $Id: questweapon.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "questweapon.h"
#include "class.h"
#include "affect.h"
#include "character.h"
#include "object.h"
#include "profflags.h"
#include "act.h"
#include "weapongenerator.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

void QuestWeapon::wear(Character *ch)
{
    ch->send_to("{CТвое оружие ярко вспыхивает.{x\r\n");
}

void QuestWeapon::equip(Character *ch)
{
    bool evil = IS_EVIL(ch);
    bool good = IS_GOOD(ch);
    bool neutral = IS_NEUTRAL(ch);

    obj->level = ch->getModifyLevel();

    if (!obj->affected.empty()) {
        for (auto &paf : obj->affected)
            addAffect(ch, paf);
    } else {
        Affect af;

        af.type = -1;
        af.duration = -1;

        if (!good) {
            af.location = APPLY_STR;
            addAffect(ch, &af);
            affect_to_obj(obj, &af);
        }

        if (!evil) {
            af.location = APPLY_DEX;
            addAffect(ch, &af);
            affect_to_obj(obj, &af);
        }

        af.location = APPLY_HIT;
        addAffect(ch, &af);
        affect_to_obj(obj, &af);

        af.location = APPLY_MANA;
        addAffect(ch, &af);
        affect_to_obj(obj, &af);
    }

    WeaponGenerator()
        .item(obj)
        .valueTier(2)
        .hitrollTier(evil ? 3 : neutral ? 2 : 1)
        .hitrollIndexBonus(good ? 2 : 0)
        .damrollTier(evil ? 1 : neutral ? 2 : 3)
        .damrollIndexBonus(evil ? 2 : 0)
        .assignValues()
        .assignHitroll()
        .assignDamroll();
}

void QuestWeapon::addAffect(Character *ch, Affect *paf)
{
    short level = ch->getModifyLevel();

    switch (paf->location) {
    case APPLY_STR:
        if (IS_GOOD(ch))
            return;
        paf->level = level;
        paf->modifier = IS_EVIL(ch) ? 2 : 1;
        return;
    case APPLY_DEX:
        if (IS_EVIL(ch))
            return;
        paf->level = level;
        paf->modifier = IS_GOOD(ch) ? 2 : 1;
        return;
    case APPLY_HIT:
        paf->level = level;
        paf->modifier = level * 2;
        return;
    case APPLY_MANA:
        paf->level = level;
        paf->modifier = level * 2;
        if (ch->getProfession()->getFlags().isSet(PROF_CASTER))
            paf->modifier += paf->modifier * 3 / 2;

        return;
    }
}

