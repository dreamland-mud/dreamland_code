/* $Id: questweapon.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 *
 * The STR/DEX/HIT/MANA affect numbers moved to Fenia (.tmp.questreward, family
 * "weapon"); the hitroll/damroll/value generator stays here in C++ because its
 * fine-grained tier configuration has no Fenia binding. equip() is overridden
 * (not the shared PersonalQuestReward path) because the weapon hangs its
 * affects conditionally by alignment and then runs the generator.
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
#include "l10n.h"

void QuestWeapon::wear(Character *ch)
{
    ch->pecho(_("{CТвое оружие ярко вспыхивает.{x"));
}

DLString QuestWeapon::questFamily( ) const
{
    return "weapon";
}

void QuestWeapon::equip(Character *ch)
{
    static const DLString fam = "weapon";
    bool evil = IS_EVIL(ch);
    bool good = IS_GOOD(ch);
    bool neutral = IS_NEUTRAL(ch);

    short level = ch->getModifyLevel();
    int tier = questTier(obj);

    obj->level = level;

    if (!obj->affected.empty()) {
        // Re-scale existing affects. STR is skipped for good and DEX for evil,
        // matching the old addAffect early-returns, so an alignment flip does
        // not zero an affect the current alignment would not grant.
        for (auto &paf : obj->affected) {
            int loc = paf->location;

            if (loc == APPLY_STR && good)
                continue;
            if (loc == APPLY_DEX && evil)
                continue;
            if (loc != APPLY_STR && loc != APPLY_DEX && loc != APPLY_HIT && loc != APPLY_MANA)
                continue;

            int mod = 0;
            if (feniaModifier(ch, fam, loc, tier, mod)) {
                paf->level = level;
                paf->modifier = mod;
            }
        }
    } else {
        // First wear: hang the affects the current alignment grants.
        Affect af;
        af.type = -1;
        af.duration = -1;
        af.level = level;

        int mod = 0;

        if (!good && feniaModifier(ch, fam, APPLY_STR, tier, mod)) {
            af.location = APPLY_STR;
            af.modifier = mod;
            affect_to_obj(obj, &af);
        }

        if (!evil && feniaModifier(ch, fam, APPLY_DEX, tier, mod)) {
            af.location = APPLY_DEX;
            af.modifier = mod;
            affect_to_obj(obj, &af);
        }

        if (feniaModifier(ch, fam, APPLY_HIT, tier, mod)) {
            af.location = APPLY_HIT;
            af.modifier = mod;
            affect_to_obj(obj, &af);
        }

        if (feniaModifier(ch, fam, APPLY_MANA, tier, mod)) {
            af.location = APPLY_MANA;
            af.modifier = mod;
            affect_to_obj(obj, &af);
        }
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
