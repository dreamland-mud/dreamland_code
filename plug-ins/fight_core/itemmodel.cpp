#include <algorithm>
#include <map>
#include <jsoncpp/json/json.h>

#include "itemmodel.h"
#include "itemvalue.h"
#include "material.h"
#include "material-table.h"
#include "skill.h"
#include "skillmanager.h"
#include "dl_math.h"
#include "loadsave.h"
#include "damageflags.h"
#include "character.h"
#include "merc.h"
#include "def.h"
#include "configurable.h"
#include "armorgenerator.h"
#include "damage.h"

using std::max;

static Json::Value pcBaseline;

CONFIGURABLE_LOADED(fight, pc_baseline)
{
    pcBaseline = value;
}

/** bands[*][melee|caster][field] interpolated by band centre, clamped at the ends. */
static double pc_baseline_field(int level, bool caster, const char *field)
{
    if (!pcBaseline.isObject() || !pcBaseline.isMember("bands"))
        return 0;

    const Json::Value &bands = static_cast<const Json::Value &>(pcBaseline)["bands"];
    if (!bands.isObject())
        return 0;
    std::map<int, double> points;
    for (auto const &key: bands.getMemberNames()) {
        const Json::Value &b = bands[key][caster ? "caster" : "melee"];
        if (DLString(key).isNumber() && b.isObject() && b[field].isNumeric())
            points[DLString(key).toInt()] = b[field].asDouble();
    }

    if (points.empty())
        return 0;
    if (level <= points.begin()->first)
        return points.begin()->second;
    if (level >= points.rbegin()->first)
        return points.rbegin()->second;

    auto hi = points.upper_bound(level);
    auto lo = std::prev(hi);
    return lo->second + (hi->second - lo->second) * (level - lo->first) / (hi->first - lo->first);
}

double item_pc_dmg(int level, bool caster)   { return pc_baseline_field(level, caster, "dmg"); }
double item_pc_hp(int level, bool caster)    { return pc_baseline_field(level, caster, "hp"); }
double item_pc_round(int level, bool caster)
{
    double round = pc_baseline_field(level, caster, "round");
    if (caster)
        round += pc_baseline_field(level, true, "spell_round");
    return round;
}

double item_combat_points(double dmgPerRound, double controlShare, int level, bool caster)
{
    ItemWeights w;
    item_weights(w, caster, level);

    double points = 0;
    double round = item_pc_round(level, caster);
    if (round > 0)
        points += dmgPerRound / round * item_pc_dmg(level, caster) * w.dr;

    double f = std::min(0.95, controlShare);
    if (f > 0)
        points += w.hp * item_pc_hp(level, caster) * f / (1 - f);

    return points;
}

void item_weights(ItemWeights &w, bool caster, int acLevel)
{
    const char *pf = caster ? "caster" : "melee";
    bool c = caster;

    w.caster       = caster;
    w.hp           = item_value(pf, "hp",           1.0);
    w.mana         = item_value(pf, "mana",         c ? 0.5 : 0.1);
    w.manaGain     = item_value(pf, "mana_gain",    c ? 0.6 : 0.05);
    w.healGain     = item_value(pf, "heal_gain",    c ? 0.15 : 0.3);
    w.dr           = item_value(pf, "damroll",      c ? 6.0 : 12.0);
    w.hr           = item_value(pf, "hitroll",      c ? 2.0 : 6.0);
    w.saves        = item_value(pf, "saves",        c ? 5.0 : 3.0);
    w.weaponWeight = item_value(pf, "weapon_weight", c ? 4.0 : 12.0);
    w.stat[0]      = item_value(pf, "str", c ? 1 : 3);
    w.stat[1]      = item_value(pf, "int", c ? 2 : 1);
    w.stat[2]      = item_value(pf, "wis", c ? 3 : 1);
    w.stat[3]      = item_value(pf, "dex", c ? 1 : 2);
    w.stat[4]      = item_value(pf, "con", 3);
    w.stat[5]      = item_value(pf, "cha", 0);

    // AC is near-useless except at low level: ac_base at level 1, linearly 0 by ac_zero_level.
    double acZero     = item_value("shared", "ac_zero_level", 40);
    w.ac              = item_value("shared", "ac_base", 0.5) * max(0.0, acZero - acLevel) / max(1.0, acZero - 1);
    // One item model: a caster still values AC past ac_zero_level (no parry, hit by everything).
    if (item_model_enabled())
        w.ac = max(w.ac, item_value(pf, "ac_floor", 0));
    w.slevel          = item_value(pf, "slevel", c ? 25.0 : 4.0);
    w.level           = item_value(pf, "level", c ? 40.0 : 14.0);
    w.skillLevel      = item_value(pf, "skill_level_group", c ? 18.0 : 8.0);
    w.skillLevelSkill = item_value(pf, "skill_level_skill", c ? 9.0 : 4.0);
    w.move            = item_value("shared", "move", 0.05);
    // One item model: moves are a melee fighter's mana (Kit 2026-10-04).
    if (item_model_enabled())
        w.move = item_value(pf, "move", w.move);
    // beats = percent cut of skill lag: central for casters (every action is a
    // lag-gated cast), a token for melee whose damage is the violence round.
    w.beats           = item_value(pf, "beats", c ? 6.0 : 1.0);
    w.learnSkill      = item_value("shared", "learn_skill", 2.0);
    w.learnGroup      = item_value("shared", "learn_group", 3.0);
    w.learnAll        = item_value("shared", "learn_all", 4.0);
}

int item_rolls(int level, const DLString &slot)
{
    double factor = item_value("measure", "default_factor", 11);
    if (!slot.empty())
        factor = item_value_sub("measure", "slot_factor", slot.c_str(), factor);
    if (factor < 1)
        factor = 11;

    return max(1, (int)(level / factor));
}

double item_one_m(int level, bool caster, const DLString &slot)
{
    ItemWeights w;
    item_weights(w, caster, level);
    return item_rolls(level, slot) * (w.dr + w.hr + 10 * w.hp + 10 * w.mana);
}

double item_level_curve(int level)
{
    std::map<int, double> points;
    const Json::Value &curve = item_value_object("measure", "level_curve");
    if (!curve.isObject())
        return 1.0;

    for (auto const &key: curve.getMemberNames()) {
        const Json::Value &v = curve[key];
        // A key that is not a level would throw from toInt() on a reset or death path.
        if (v.isNumeric() && DLString(key).isNumber())
            points[DLString(key).toInt()] = v.asDouble();
    }

    if (points.empty())
        return 1.0;
    if (level <= points.begin()->first)
        return points.begin()->second;
    if (level >= points.rbegin()->first)
        return points.rbegin()->second;

    auto hi = points.upper_bound(level);
    auto lo = std::prev(hi);
    return lo->second + (hi->second - lo->second) * (level - lo->first) / (hi->first - lo->first);
}

static int itemModelOverride = -1;

bool item_model_enabled()
{
    if (itemModelOverride >= 0)
        return itemModelOverride > 0;
    return item_value("measure", "item_model", 0) != 0;
}

int item_model_override(int mode)
{
    int old = itemModelOverride;
    itemModelOverride = mode < 0 ? -1 : (mode > 0 ? 1 : 0);
    return old;
}

int item_model_cost(int measureCm, int level)
{
    int cost = max(0, (int)(item_value("measure", "cost_k", 5.5) * measureCm / 100.0 * level));
    int cap = (int)item_value("measure", "cost_cap", 0);
    return cap > 0 ? min(cost, cap) : cost;
}

double item_points_by_level(const Json::Value &table, int level, int col)
{
    if (!table.isObject())
        return 0;

    std::map<int, double> points;
    for (auto const &key: table.getMemberNames()) {
        const Json::Value &v = table[key];
        const Json::Value &x = (col >= 0 && v.isArray()) ? v[(Json::ArrayIndex)col] : v;
        if (DLString(key).isNumber() && x.isNumeric())
            points[DLString(key).toInt()] = x.asDouble();
    }

    if (points.empty())
        return 0;
    if (level <= points.begin()->first)
        return points.begin()->second;
    if (level >= points.rbegin()->first)
        return points.rbegin()->second;

    auto hi = points.upper_bound(level);
    auto lo = std::prev(hi);
    return lo->second + (hi->second - lo->second) * (level - lo->first) / (hi->first - lo->first);
}

bool item_level_window_ok(const Json::Value &affix, int level)
{
    if (affix.isMember("min_level") && level < affix["min_level"].asInt())
        return false;
    if (affix.isMember("max_level") && level > affix["max_level"].asInt())
        return false;
    return true;
}

double item_apply_points(int location, int m, const ItemWeights &w)
{
    switch (location) {
    case APPLY_HIT:         return w.hp * m;
    case APPLY_MANA:        return w.mana * m;
    case APPLY_MANA_GAIN:   return w.manaGain * m;
    case APPLY_HEAL_GAIN:   return w.healGain * m;
    case APPLY_DAMROLL:     return w.dr * m;
    case APPLY_HITROLL:     return w.hr * m;
    case APPLY_SAVES:
    case APPLY_SAVING_ROD:
    case APPLY_SAVING_PETRI:
    case APPLY_SAVING_BREATH:
    case APPLY_SAVING_SPELL: return w.saves * (-m);   // lower save = better
    case APPLY_STR:         return w.stat[0] * m;
    case APPLY_INT:         return w.stat[1] * m;
    case APPLY_WIS:         return w.stat[2] * m;
    case APPLY_DEX:         return w.stat[3] * m;
    case APPLY_CON:         return w.stat[4] * m;
    case APPLY_CHA:         return w.stat[5] * m;
    case APPLY_AC:          return w.ac * (-m);       // negative ac = better
    case APPLY_SPELL_LEVEL: return w.slevel * m * w.spellFactor;
    case APPLY_MOVE:        return w.move * m;
    case APPLY_BEATS:       return w.beats * (-m);    // shorter skill lag = better
    case APPLY_LEVEL:       return w.level * m;       // unscoped; scoped is a fit value
    }
    return 0;
}

/*
 * affect_flags values ([melee, caster] in item_value.json "flags"). spell = the
 * buff a char may cast itself (10% clause); 0 for gear-only bits and curses.
 */
struct FlagValue {
    bitstring_t flag;
    const char *key;
    double melee, caster;
    const char *spell;
};

static const FlagValue flag_values[] = {
    { AFF_SANCTUARY,    "sanctuary",    300,  300,  "sanctuary" },
    { AFF_HASTE,        "haste",        110,  40,   "haste" },
    { AFF_SLOW,         "slow",         -55,  -10,  0 },
    { AFF_PROTECT_EVIL, "protect_evil", 60,   60,   "protection evil" },
    { AFF_PROTECT_GOOD, "protect_good", 60,   60,   "protection good" },
    { AFF_REGENERATION, "regeneration", 30,   15,   0 },
    { AFF_FLYING,       "flying",       15,   15,   "fly" },
    { AFF_PASS_DOOR,    "pass_door",    15,   15,   "pass door" },
    { AFF_INVISIBLE,    "invisible",    15,   15,   "invis" },
    { AFF_IMP_INVIS,    "imp_invis",    100,  100,  "improved invis" },
    { AFF_CAMOUFLAGE,   "camouflage",   100,  100,  "camouflage" },
    { AFF_FADE,         "fade",         100,  100,  "fade" },
    { AFF_SNEAK,        "sneak",        12,   8,    "sneak" },
    { AFF_HIDE,         "hide",         6,    6,    "hide" },
    { AFF_INFRARED,     "infrared",     8,    8,    "infravision" },
    { AFF_SWIM,         "swim",         4,    4,    0 },
    // Cursed-item penalties (never discounted).
    { AFF_STUN,         "stun",         -100, -100, 0 },
    { AFF_WEAK_STUN,    "weak_stun",    -40,  -40,  0 },
    { AFF_BLIND,        "blind",        -200, -200, 0 },
    { AFF_SLEEP,        "sleep",        -250, -250, 0 },
    { AFF_CHARM,        "charm",        -250, -250, 0 },
    { AFF_CURSE,        "curse",        -35,  -35,  0 },
    { AFF_CORRUPTION,   "corruption",   -50,  -50,  0 },
    { AFF_POISON,       "poison",       -45,  -45,  0 },
    { AFF_PLAGUE,       "plague",       -60,  -60,  0 },
    { AFF_CALM,         "calm",         -25,  -5,   0 },
    { AFF_WEAKEN,       "weaken",       -10,  -10,  0 },
    { AFF_FAERIE_FIRE,  "faerie_fire",  -15,  -15,  0 },
};

static double flag_base(const FlagValue &fv, bool caster, int level)
{
    if (level >= 0 && item_model_enabled()) {
        const Json::Value &byLevel = item_value_object("flags_by_level", fv.key);
        if (byLevel.isObject())
            return item_points_by_level(byLevel, level, caster ? 1 : 0);
    }
    return item_value("flags", fv.key, caster ? fv.caster : fv.melee, caster ? 1 : 0);
}

double item_flag_base(bitstring_t flag, bool caster, int level)
{
    for (auto const &fv: flag_values)
        if (fv.flag == flag)
            return flag_base(fv, caster, level);
    return 0;
}

double item_flag_points(bitstring_t bits, bool caster, Character *target, bitstring_t heldFlags,
                        int level)
{
    double s = 0;

    for (auto const &fv: flag_values) {
        if (!IS_SET(bits, fv.flag))
            continue;

        double base = flag_base(fv, caster, level);
        // A second copy of a boon the char already has for free is worth nothing.
        // Curses still count: nobody "already has" a curse as a boon.
        if (base > 0 && IS_SET(heldFlags, fv.flag))
            continue;
        if (fv.spell != 0 && item_fit_self_cast(target, fv.spell))
            s += base * 0.1;
        else
            s += base;
    }

    return s;
}

/*
 * res/imm/vuln values ([res, imm, vuln] in item_value.json "res"). The three tables
 * share bit positions (immune_from_flags tests ONE bit against imm/res/vuln alike), so
 * IMM_* constants index every table. Resists do not stack (immunity.cpp: a binary
 * RESISTANT bit), and imm_bash FULLY blocks bash weapon-hits. weapon/spell are the
 * broad channels; bash/pierce/slash are their measured share of the weapon channel;
 * elementals are danger-tiered (fire worst; negative/mental/energy nukes; the rest at
 * the floor). res = imm/3, vuln = -imm/2, floor 10/30/-20. spell == magic == prayer:
 * counted once. DAMTYPE_MODEL.md.
 */
struct ResValue {
    bitstring_t mask;
    const char *key;
    double res, imm, vuln;
};

static const ResValue res_values[] = {
    { IMM_WEAPON,                         "weapon",    140, 420, -210 },
    { IMM_SPELL | IMM_MAGIC | IMM_PRAYER, "spell",     50,  150, -75 },
    { IMM_BASH,                           "bash",      72,  216, -108 },
    { IMM_PIERCE,                         "pierce",    34,  103, -52 },
    { IMM_SLASH,                          "slash",     33,  100, -50 },
    { IMM_FIRE,                           "fire",      23,  70,  -35 },
    { IMM_ENERGY,                         "energy",    17,  50,  -25 },
    { IMM_NEGATIVE,                       "negative",  17,  50,  -25 },
    { IMM_LIGHTNING,                      "lightning", 13,  40,  -20 },
    { IMM_ACID,                           "acid",      13,  40,  -20 },
    { IMM_HOLY,                           "holy",      13,  40,  -20 },
    { IMM_COLD,                           "cold",      13,  40,  -20 },
    { IMM_POISON,                         "poison",    13,  40,  -20 },
    { IMM_CHARM,                          "charm",     10,  30,  -20 },
    { IMM_MENTAL,                         "mental",    17,  50,  -25 },
    { IMM_DISEASE,                        "disease",   10,  30,  -20 },
    { IMM_DROWNING,                       "drowning",  10,  30,  -20 },
    { IMM_LIGHT,                          "light",     10,  30,  -20 },
    { IMM_SOUND,                          "sound",     10,  30,  -20 },
    { IMM_SUMMON,                         "summon",    10,  30,  -20 },
    { IMM_IRON,                           "iron",      10,  30,  -20 },
    { IMM_WOOD,                           "wood",      10,  30,  -20 },
    { IMM_SILVER,                         "silver",    10,  30,  -20 },
    { IMM_MITHRIL,                        "mithril",   10,  30,  -20 },
};

static double res_column(const ResValue &rv, int kind)
{
    double def = kind == 1 ? rv.imm : (kind == 2 ? rv.vuln : rv.res);
    return item_value("res", rv.key, def, kind == 1 ? 1 : (kind == 2 ? 2 : 0));
}

double item_res_points(bitstring_t bits, int kind)
{
    double s = 0;
    for (auto const &rv: res_values)
        if (bits & rv.mask)
            s += res_column(rv, kind);
    return s;
}

double item_res_points_fit(bitstring_t bits, int kind, bitstring_t ownedImm, bitstring_t ownedRes)
{
    double s = 0;

    for (auto const &rv: res_values) {
        if (!(bits & rv.mask))
            continue;

        bool imm = (ownedImm & rv.mask) != 0;
        bool res = (ownedRes & rv.mask) != 0;

        if (imm)
            continue;                    // immune wins over anything an item adds
        if (kind == 1)
            s += res_column(rv, 1) - (res ? res_column(rv, 0) : 0);
        else if (kind == 0) {
            if (!res)
                s += res_column(rv, 0);  // resists do not stack
        }
        else
            s += res_column(rv, 2);
    }

    return s;
}

double item_fit_stat(int base, int delta, int cap)
{
    int before = URANGE(MIN_STAT, base, cap);
    int after  = URANGE(MIN_STAT, base + delta, cap);
    return after - before;
}

bool item_fit_self_cast(Character *ch, const char *skillName)
{
    if (ch == 0)
        return false;
    Skill *sk = skillManager->findExisting(skillName);
    return sk != 0 && sk->available(ch);
}

double item_fit_weapon_skill(Character *ch, int weaponSn)
{
    int pct = ch != 0 ? ch->getSkill(weaponSn) : 100;
    return (20 + pct) / 100.0;
}

double item_fit_material(Character *ch, const DLString &material)
{
    if (ch == 0)
        return 1.0;
    const material_t *m = material_by_name(material);
    if (m && IS_SET(m->type, material_types_forbidden(ch)))
        return 0.0;
    return 1.0;
}

double item_fit_alignment(Character *ch, const DLString &weaponFlag)
{
    if (ch == 0)
        return 1.0;
    if (weaponFlag == "holy")
        return IS_GOOD(ch) ? 1.0 : 0.0;
    // flag.points_by_level prices vorpal for a good wielder (1% per hit beheads);
    // anyone else beheads at 0.5% (onehit_undef.cpp damEffectVorpal).
    if (weaponFlag == "vorpal")
        return IS_GOOD(ch) ? 1.0 : 0.5;
    if (weaponFlag == "vampiric")
        return IS_EVIL(ch) ? 1.0 : 0.0;
    return 1.0;
}

/*
 * What an item carries outside its affect list (P6).
 */
static const Json::Value *affix_entry(const char *section, const DLString &value)
{
    const Json::Value &all = item_affixes_config();
    if (!all.isObject() || !all.isMember(section))
        return 0;
    const Json::Value &values = all[section]["values"];
    if (!values.isArray())
        return 0;
    for (auto const &affix: values)
        if (affix["value"].asString() == value)
            return &affix;
    return 0;
}

double item_weapon_flag_points(int weaponFlags, int level, bool caster, Character *target)
{
    const Json::Value &all = item_affixes_config();
    if (weaponFlags == 0 || !all.isObject() || !all.isMember("flag"))
        return 0;

    double s = 0;
    for (auto const &affix: all["flag"]["values"]) {
        if (!affix.isMember("points_by_level"))
            continue;
        DLString name = affix["value"].asString();
        bitnumber_t bit = weapon_type2.value(name);
        if (bit == (bitnumber_t)NO_FLAG || bit == 0 || !IS_SET(weaponFlags, bit))
            continue;
        s += item_points_by_level(affix["points_by_level"], level, caster ? 1 : -1)
             * item_fit_alignment(target, name);
    }
    return s;
}

double item_detect_points(bitstring_t bits, bool caster, Character *target)
{
    double s = 0;
    for (int i = 0; i < detect_flags.size; i++) {
        bitstring_t bit = detect_flags.fields[i].value;
        if (bit == 0 || !IS_SET(bits, bit))
            continue;
        const char *name = detect_flags.fields[i].name;
        double base = item_value("detects", name, 0, caster ? 1 : 0);
        if (base > 0 && target != 0) {
            DLString spell = DLString("detect ") + name;
            if (item_fit_self_cast(target, spell.c_str()) || item_fit_self_cast(target, name))
                base *= 0.1;
        }
        s += base;
    }
    return s;
}

double item_extra_points(bitstring_t bits, bool caster)
{
    double s = 0;
    for (int i = 0; i < extra_flags.size; i++) {
        bitstring_t bit = extra_flags.fields[i].value;
        if (bit != 0 && IS_SET(bits, bit))
            s += item_value("extras", extra_flags.fields[i].name, 0, caster ? 1 : 0);
    }
    return s;
}

double item_material_points(const DLString &material, int level, bool caster)
{
    if (material.empty())
        return 0;
    return item_value("materials", material.c_str(), 0, caster ? 1 : 0)
         + item_points_by_level(item_value_object("materials_combat", material.c_str()), level);
}

double item_wornbuff_points(const DLString &buff, int level, bool caster)
{
    const Json::Value *affix = affix_entry("worn_buff", buff);
    if (affix == 0 || !affix->isMember("points_by_level"))
        return 0;
    return item_points_by_level((*affix)["points_by_level"], level, caster ? 1 : 0);
}

static ItemSpellTierFn spellTierFn = 0;

void item_set_spell_tier_fn(ItemSpellTierFn fn)
{
    spellTierFn = fn;
}

double item_proc_points(const DLString &spell, double chance, double count, int level, bool caster)
{
    if (chance <= 0)
        return 0;
    if (count <= 0)
        count = 1;
    if (count > 10)          // the firing cap (ocombatcast_fight)
        count = 10;

    double v = spell_combat_value(spell);
    if (v > 0)
        v *= level / spell_combat_level_ref();
    else if (spellTierFn != 0)
        v = spellTierFn(spell, level) * spell_combat_save_factor();
    if (v <= 0)
        return 0;

    return item_combat_points(v * chance / 100.0 * count, 0, level, caster);
}
