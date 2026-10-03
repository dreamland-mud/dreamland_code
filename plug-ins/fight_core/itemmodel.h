#ifndef ITEMMODEL_H
#define ITEMMODEL_H

/*
 * One item model (docs/plans/one-item-model.md): the base value of anything an item
 * carries, in gear-score points, plus the fit clauses that adjust it for one
 * character. The random item generators buy affixes with base points / one M; the
 * gear sage scores base points x fit. Weights and tables come from
 * config/fight/item_value.json (itemvalue.h); a missing key keeps today's constant.
 */
#include "dlstring.h"
#include "bitstring.h"

namespace Json { class Value; }
class Character;

/** Per-point weights of one profile. The char-dependent fields (spellFactor,
 *  heldFlags, curWeaponSwing, learn*) are the sage's to fill. */
struct ItemWeights {
    double hp = 0, mana = 0, manaGain = 0, healGain = 0, dr = 0, hr = 0, saves = 0;
    double weaponWeight = 0;   // per average weapon damage, split from dr so a caster
                               // can value a weapon's melee output below its damroll
    double stat[6] = {};       // str, int, wis, dex, con, cha
    bool   caster = false;
    double ac = 0, slevel = 0, level = 0, skillLevel = 0, skillLevelSkill = 0, move = 0, beats = 0;
    double spellFactor = 1.0;  // slevel discount for a char with few/no spells (0..1)
    double learnSkill = 0, learnGroup = 0, learnAll = 0;   // APPLY_LEARNED %, by scope
    // affect_flags the char already has for free (perma affects + worn gear OUTSIDE the
    // candidate's slot): a candidate re-granting one scores it 0. Sage-only.
    bitstring_t heldFlags = 0;
    // One swing of the char's wielded weapon in points, for a non-weapon item's
    // combathits. -1 = no usable weapon. Sage-only.
    double curWeaponSwing = -1.0;
};

/** Fill the profile weights. acLevel is the level the AC weight is taken at:
 *  the asking char's level in the sage, the item's level for a base price. */
void item_weights(ItemWeights &w, bool caster, int acLevel);

/** Measure rolls at this level: level / slot factor, at least 1. */
int item_rolls(int level, const DLString &slot = DLString::emptyString);

/** One M in points: one roll set (dr + hr + 10 hp + 10 mana) x rolls (decision 2). */
double item_one_m(int level, bool caster, const DLString &slot = DLString::emptyString);

/** Tier budget multiplier by item level, item_value.json measure.level_curve
 *  {"level": mult, ...} interpolated; 1.0 when absent (decision 3). */
double item_level_curve(int level);

/** An affix's min_level / max_level window; absent = no limit (decisions 3, 4). */
bool item_level_window_ok(const Json::Value &affix, int level);

/** Base points of a plain numeric apply that needs no char (hp, dr, saves, ac,
 *  slevel, unscoped +level, move, beats...). Primary stats are flat weight x mod
 *  here; the sage resolves them through item_fit_stat. Scoped applies (skill or
 *  group +level / learned) and APPLY_NONE return 0: they only have a fit value. */
double item_apply_points(int location, int modifier, const ItemWeights &w);

/** Base points of one affect_flags bit, no fit (one profile column). */
double item_flag_base(bitstring_t flag, bool caster);

/** affect_flags bitvector with the sage's fit: a positive bit already held is 0,
 *  a positive bit the char can self-cast is 10%. target 0 / heldFlags 0 = base. */
double item_flag_points(bitstring_t bits, bool caster, Character *target, bitstring_t heldFlags);

/** res/imm/vuln bitvector, kind 0 res / 1 imm / 2 vuln, no fit. */
double item_res_points(bitstring_t bits, int kind);

/** Same with the "already have it" clause (decision 6), combined the way
 *  immunity.cpp resolves: immune wins, a second resist adds nothing, an immunity
 *  over an owned resist is worth the difference. ownedImm/ownedRes are the char's
 *  flags WITHOUT the item being replaced. Vulns keep their base unless immune. */
double item_res_points_fit(bitstring_t bits, int kind, bitstring_t ownedImm, bitstring_t ownedRes);

/*
 * Fit clauses.
 */
/** Stat points actually gained: base + delta clamped to [MIN_STAT, cap]. */
double item_fit_stat(int base, int delta, int cap);

/** The char knows this spell/skill, so a gear copy of its buff is worth 10%. */
bool item_fit_self_cast(Character *ch, const char *skillName);

/** Landed share of weapon dice at the char's skill: (20 + skill%) / 100. */
double item_fit_weapon_skill(Character *ch, int weaponSn);

/** A material the char may not wear (druid + metal) is worth nothing: 0 or 1. */
double item_fit_material(Character *ch, const DLString &material);

/** Weapon flag alignment clause (decision 8): holy needs a good owner, vorpal is
 *  worth x2 to a good one, vampiric only to an evil one. Multiplier. */
double item_fit_alignment(Character *ch, const DLString &weaponFlag);

#endif
