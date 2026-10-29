#include <algorithm>
#include <cmath>

#include "armorgenerator.h"
#include "weapongenerator.h"
#include "weapontier.h"
#include "itemvalue.h"
#include "itemmodel.h"
#include "itemeconomy.h"
#include "magicitemgenerator.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillreference.h"
#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "profession.h"
#include "profflags.h"
#include "flagtableregistry.h"

#include "damageflags.h"
#include "morphology.h"
#include "material.h"
#include "material-table.h"
#include "loadsave.h"
#include "dl_math.h"
#include "alignment.h"
#include "date.h"
#include "merc.h"
#include "def.h"

GSN(none);

static Json::Value item_affixes;
CONFIGURABLE_LOADED(fight, item_affixes)
{
    item_affixes = value;
}

static Json::Value armor_names;
CONFIGURABLE_LOADED(fight, armor_names)
{
    armor_names = value;
}

const Json::Value & item_affixes_config()
{
    return item_affixes;
}

bool armor_slot_exists(const DLString &slot)
{
    return !slot.empty() && slot.at(0) != '_'
            && armor_names.isMember(slot) && armor_names[slot].isArray();
}

/** Proc chance per combat round by the affix's tier floor (Kit 2026-10-02):
 *  rare 3%, epic 5%, legendary 8%. */
static int proc_chance(int tierFloor)
{
    if (tierFloor <= 1)
        return 8;
    if (tierFloor == 2)
        return 5;
    return 3;
}

/*--------------------------------------------------------------------------
 * ItemAffixRoller
 *-------------------------------------------------------------------------*/
ItemAffixRoller::ItemAffixRoller(Object *obj, PCharacter *pch, int tier, const DLString &kind, const DLString &slot)
        : crafted(false), rollsOverride(0), obj(obj), pch(pch), tier(tier), kind(kind), slot(slot), isCaster(false), align(ALIGN_NONE),
          sigPool(false), chosenTotal(0), extraFlags(0, &extra_flags)
{
}

ItemAffixRoller::~ItemAffixRoller()
{
}

/** Groups the killer actually uses: a +1 to a group they never touch is a
 *  paperweight, same reasoning as the weapon class filter. */
void ItemAffixRoller::learnPlayerGroups()
{
    if (!pch)
        return;

    for (int sn = 0; sn < skillManager->size(); sn++) {
        PCSkillData &data = pch->getSkillData(sn);
        if (data.learned <= 1 || data.isTemporary())
            continue;
        Skill *skill = skillManager->find(sn);
        for (auto g: skill->getGroups().toArray())
            groupCounts[g]++;
    }

    // A group needs 2+ learned skills to be worth a +level (Kit 2026-10-04).
    for (auto const &gc: groupCounts)
        if (gc.second >= 2)
            playerGroups.insert(gc.first);
}

void ItemAffixRoller::flushAffects()
{
    for (auto &af: affects) {
        if (af.bitvector.getTable() != 0 || !af.global.empty())
            affect_to_obj(obj, &af);
        else
            affect_enhance(obj, &af);
    }
}

/*--------------------------------------------------------------------------
 * ArmorGenerator
 *-------------------------------------------------------------------------*/
ArmorGenerator::ArmorGenerator(Object *obj, PCharacter *pch, int tier, const DLString &slot)
        : ItemAffixRoller(obj, pch, tier, "armor", slot), sigIndex(-1),
          procs(Json::arrayValue), keepName(false), craftMin(0), craftMax(0)
{
}

bool ArmorGenerator::run()
{
    if (!keepName && !armor_slot_exists(slot)) {
        warn("Armor generator: no names configured for slot %s.", slot.c_str());
        return false;
    }

    if (tier < BEST_TIER || tier > WORST_TIER || (int)weapon_tier_table.size() < tier) {
        warn("Armor generator: bad tier %d.", tier);
        return false;
    }

    if (obj->level < 1)
        obj->level = 1;

    // A reroll starts from the plain weight; +weight sets it again if it rolls.
    obj->removeProperty("weight_pct");

    if (keepName) {
        // The first listed material decides the AC hardness; material affixes are off.
        materialName = obj->getMaterial();
        DLString::size_type comma = materialName.find(',');
        if (comma != DLString::npos)
            materialName = materialName.substr(0, comma);
        materialName.stripWhiteSpace();
    } else if (!pickNoun())
        return false;

    learnPlayerGroups();

    for (auto const &w: forcedSpec.split(" ")) {
        std::list<DLString> parts = w.split(":");
        if (parts.size() < 2)
            continue;
        auto p = parts.begin();
        DLString sec = *p++;
        DLString value = *p;
        forcedKeys.insert(sec + ":" + value.replaces("~", " "));
    }
    collectCandidates();
    std::map<int, int> forced = resolveForced(forcedSpec);
    int sig = crafted ? -1 : pickSignature();
    if (sig >= 0)
        forced[sig] = 1;
    sigIndex = sig;

    const weapon_tier_t &t = weapon_tier_table[tier - 1];
    if (crafted) {
        pickAffixes(craftMin, craftMax, t.worst_penalty_m, t.min_affixes_m, t.max_affixes_m, t.max_negatives_m, forced);
    } else {
        double curve = windowCurve();
        pickAffixes((int)(t.min_m * curve), (int)(t.max_m * curve), t.worst_penalty_m, t.min_affixes_m,
                    t.max_affixes_m, t.max_negatives_m, forced);
    }
    applyAffixes();

    if (sig >= 0)
        obj->setProperty("signature", pool[sig].section + ":" + pool[sig].value);

    // The material affix, if any, was applied above; otherwise the noun's own.
    if (!keepName) {
        if (materialName.empty())
            materialName = defaultMaterial();
        obj->setMaterial(materialName.c_str());
    }

    // A crafted piece keeps the AC and the colour its craft gave it.
    if (!crafted) {
        assignAC();
        if (keepName)
            assignColours();
        else if (sigIndex < 0 || !assignSignatureNames())
            assignNames();
    }
    assignFlags();
    if (crafted)
        obj->setProperty("craft_picks", picksSpec());

    flushAffects();

    notice("rand_armor: created item %s [%d] [%lld] slot %s tier %d affixes [%s] level %d",
            obj->getShortDescr('1', LANG_DEFAULT).c_str(),
            obj->pIndexData->vnum, obj->getID(), slot.c_str(), tier,
            obj->getProperty("affixes").c_str(), obj->level);

    return true;
}

/*--------------------------------------------------------------------------
 * Boss signature
 *-------------------------------------------------------------------------*/
/** Legendary armor from a boss carries one signature affix (Kit 2026-10-05):
 *  50/50 a defense against the boss's damage type or one of its spells as a
 *  proc. The defense is a resist, or at signature.imm_pct an immunity when the
 *  pool has one for that damage type (never a physical one; the elemental
 *  immunities exist only for signatures, needs_boss). The affix comes from the
 *  ordinary pool, so the killer's fit, the caps and the price all apply; when
 *  that half has nothing in the pool the other is tried, and with neither the
 *  item rolls as usual. */
int ArmorGenerator::pickSignature() const
{
    if (tier != BEST_TIER || (sigDamtype.empty() && sigSpells.empty()))
        return -1;

    int resist = -1, immune = -1;
    std::vector<int> procIdx;
    for (int i = 0; i < (int)pool.size(); i++) {
        const Candidate &c = pool[i];
        if (c.section == "resists" && c.value == sigDamtype)
            resist = i;
        else if (c.section == "immunes" && c.value == sigDamtype)
            immune = i;
        else if (c.section == "proc"
                 && std::find(sigSpells.begin(), sigSpells.end(), c.value) != sigSpells.end())
            procIdx.push_back(i);
    }

    if (immune >= 0 && number_percent() <= item_affixes["signature"]["imm_pct"].asInt())
        resist = immune;

    int proc = procIdx.empty() ? -1 : procIdx.at(number_range(0, procIdx.size() - 1));
    if (number_range(0, 1) == 0)
        return resist >= 0 ? resist : proc;
    return proc >= 0 ? proc : resist;
}

/*--------------------------------------------------------------------------
 * Noun and material
 *-------------------------------------------------------------------------*/
static bool material_is_metal(const DLString &name)
{
    const material_t *m = material_by_name(name);
    return m && IS_SET(m->type, MAT_METAL);
}

/** True when the item's own affects (instance or prototype) already carry a
 *  res/vuln/imm bit for this damage type. */
static bool obj_has_damtype_bit(Object *obj, const DLString &value)
{
    static const char *tables[] = { "res_flags", "vuln_flags", "imm_flags" };
    for (int k = 0; k < 3; k++) {
        const FlagTable *ft = FlagTableRegistry::getTable(tables[k]);
        if (!ft)
            continue;
        Affect probe;
        probe.bitvector.setTable(ft);
        probe.bitvector.setBits(value);
        bitstring_t b = probe.bitvector;
        if (b == 0)
            continue;

        for (auto &paf: obj->affected)
            if (paf->bitvector.getTable() == ft && ((bitstring_t)paf->bitvector & b))
                return true;
        for (auto &paf: obj->pIndexData->affected)
            if (paf->bitvector.getTable() == ft && ((bitstring_t)paf->bitvector & b))
                return true;
    }
    return false;
}

/** Materials: only what the chosen noun is made of. */
bool ArmorGenerator::valueAllowed(const DLString &secName, const DLString &value) const
{
    if (keepName) {
        // A hand-made piece keeps its own res/vuln/imm: never roll a second one
        // on the same damage type (res cold twice, res fire next to vuln fire).
        // Crafted pieces replay their forced picks and are left alone.
        if (!crafted && (secName == "resists" || secName == "vulns" || secName == "immunes")
                && obj_has_damtype_bit(obj, value))
            return false;
        // A crafted piece rolls worn buffs too: any item with a "wornbuff"
        // property casts it on equip (global onEquipWornBuff).
        return secName != "material" && (crafted || secName != "worn_buff");
    }

    return secName != "material" || materialAllowed(value);
}

bool ArmorGenerator::materialAllowed(const DLString &name) const
{
    if (pch && material_is_metal(name)
            && IS_SET(material_types_forbidden(pch), MAT_METAL))
        return false;

    for (auto const &m: nounConfig["materials"])
        if (m.asString() == name)
            return true;

    return false;
}

/** The noun's first material the killer may wear: the list is ordered default-first. */
DLString ArmorGenerator::defaultMaterial() const
{
    for (auto const &m: nounConfig["materials"])
        if (materialAllowed(m.asString()))
            return m.asString();

    return nounConfig["materials"].empty() ? DLString("leather") : DLString(nounConfig["materials"][0u].asString());
}

bool ArmorGenerator::pickNoun()
{
    const Json::Value &configs = armor_names[slot];
    vector<Json::ArrayIndex> allowed;

    // A druid never gets a noun that only comes in metal.
    for (Json::ArrayIndex i = 0; i < configs.size(); i++) {
        nounConfig = configs[i];
        if (!defaultMaterial().empty() && materialAllowed(defaultMaterial()))
            allowed.push_back(i);
    }

    if (allowed.empty()) {
        warn("Armor generator: no wearable noun for slot %s.", slot.c_str());
        for (Json::ArrayIndex i = 0; i < configs.size(); i++)
            allowed.push_back(i);
    }

    if (allowed.empty())
        return false;

    nounConfig = configs[allowed.at(number_range(0, allowed.size() - 1))];
    return true;
}

/*--------------------------------------------------------------------------
 * Affix pool
 *-------------------------------------------------------------------------*/
bool ItemAffixRoller::candidateAllowed(const Json::Value &section, const Json::Value &affix, int floor) const
{
    // tier floor: allowed when the item's tier is that good or better (1 = legendary).
    if (tier > floor)
        return false;

    // The signature section is a marker: ArmorGenerator::pickSignature forces the
    // boss's resist or proc from the sections that price them. A needs_boss affix
    // (an elemental immunity) joins the pool only for a signature roll, and only
    // pickSignature may take it.
    if (section["needs_boss"].asBool())
        return false;
    if (affix["needs_boss"].asBool() && !sigPool)
        return false;

    if ((section["needs_player"].asBool() || affix["needs_player"].asBool()) && !pch)
        return false;

    if (crafted && section["no_craft"].asBool())
        return false;

    if (affix.isMember("align") && align != ALIGN_NONE) {
        const Json::Value &range = affix["align"];
        if (range.size() == 2 && (align < range[0u].asInt() || align > range[1u].asInt()))
            return false;
    }

    // Level windows (one item model): ac ends at 40, sanctuary-family buffs start at 30.
    if (!item_level_window_ok(affix, obj->level))
        return false;

    // A worn buff that wants a skill of the wearer's own (concentrate).
    if (affix["needs_skill"].asBool()) {
        Skill *skill = skillManager->findExisting(affix["value"].asString());
        if (!pch || !skill || !skill->available(pch))
            return false;
    }

    return true;
}

bool ItemAffixRoller::valueAllowed(const DLString &secName, const DLString &value) const
{
    return true;
}

int ItemAffixRoller::candidatePrice(const DLString &secName, const Json::Value &affix, bool &allowed) const
{
    allowed = true;
    if (tier == BEST_TIER && affix.isMember("price_legendary"))
        return affix["price_legendary"].asInt();

    double points;
    if (modelPoints(secName, affix, points))
        return modelPrice(points);

    return (isCaster ? affix["price_caster"] : affix["price_melee"]).asInt();
}

static double table_points(const DLString &table, const DLString &bitNames, bool caster, int level)
{
    const FlagTable *t = FlagTableRegistry::getTable(table);
    if (!t)
        return 0;

    if (table == "detect_flags") {
        double s = 0;
        StringList names(bitNames);
        for (auto const &n: names)
            s += item_value("detects", n.c_str(), 0, caster ? 1 : 0);
        return s;
    }

    Affect af;
    af.bitvector.setTable(t);
    af.bitvector.setBits(bitNames);
    bitstring_t bits = af.bitvector;

    if (table == "affect_flags")
        return item_flag_points(bits, caster, 0, 0, level);
    if (table == "res_flags")
        return item_res_points(bits, 0);
    if (table == "imm_flags")
        return item_res_points(bits, 1);
    if (table == "vuln_flags")
        return item_res_points(bits, 2);
    return 0;
}

bool ItemAffixRoller::modelPoints(const DLString &secName, const Json::Value &affix, double &points) const
{
    ItemWeights w;
    item_weights(w, isCaster, obj->level);
    int col = isCaster ? 1 : 0;
    DLString value = affix["value"].asString();
    DLString norm = (!value.empty() && (value.at(0) == '-' || value.at(0) == '+')) ? value.substr(1) : value;

    points = 0;

    // A hand-priced affix (the penalties the model has no weight for: size, age,
    // weight, a language): "points" is a number or [melee, caster].
    if (affix.isMember("points")) {
        const Json::Value &p = affix["points"];
        points = p.isArray() ? p[(Json::ArrayIndex)col].asDouble() : p.asDouble();
        return true;
    }

    // Senses are priced by affix (item_value.json detects), whatever bits they set.
    if (secName == "senses") {
        points = item_value("detects", value.c_str(), 0, col);
        return true;
    }

    if (affix.isMember("affects")) {
        for (auto const &one: affix["affects"]) {
            if (one.isMember("apply"))
                points += item_apply_points(apply_flags.value(one["apply"].asString()), modifier(one, 1), w);
            else if (one.isMember("table"))
                points += table_points(one["table"].asString(), one["bits"].asString(), isCaster, obj->level);
        }
        return true;
    }

    if (secName == "armor_stats" || secName == "affects_by_level" || secName == "primary_stats") {
        points = item_apply_points(apply_flags.value(norm), modifier(affix, 1), w);
        return true;
    }
    if (secName == "skill_group") {
        points = w.skillLevel * max(1, affix["mod"].asInt());
        return true;
    }
    if (secName == "player") {
        points = (value == "learned" ? w.learnSkill * learnedValueMult() : w.skillLevel) * max(1, affix["mod"].asInt());
        return true;
    }
    if (secName == "extra") {
        points = item_value("extras", value.c_str(), 0, col);
        return true;
    }
    if (secName == "resists" || secName == "vulns" || secName == "immunes"
            || secName == "affects_with_bits") {
        DLString table = affix.isMember("table") ? affix["table"].asString()
                       : secName == "resists" ? "res_flags"
                       : secName == "vulns" ? "vuln_flags"
                       : secName == "immunes" ? "imm_flags"
                       : "affect_flags";
        points = table_points(table, value, isCaster, obj->level);
        return true;
    }
    if (secName == "worn_buff" && affix.isMember("points_by_level")) {
        points = item_points_by_level(affix["points_by_level"], obj->level, col);
        return true;
    }
    // Procs: the combat-effect model at the chance this affix will roll with
    // (applyOne: proc_chance of its tier floor), same as the sage scores it.
    if (secName == "proc") {
        const Json::Value &section = item_affixes[secName];
        int floor = affix.isMember("tier") ? affix["tier"].asInt()
                  : section.isMember("tier") ? section["tier"].asInt() : WORST_TIER;
        points = item_proc_points(value, proc_chance(floor), 1, obj->level, isCaster);
        return true;
    }
    if (secName == "material" || secName == "weapon_material") {
        points = item_value("materials", value.c_str(), 0, col)
               + item_points_by_level(item_value_object("materials_combat", value.c_str()), obj->level);
        return true;
    }

    return false;
}

int ItemAffixRoller::modelPrice(double points) const
{
    double oneM = item_one_m(obj->level, isCaster, slot);
    if (oneM <= 0)
        return 0;
    return (int)std::round(100 * points / oneM);
}

DLString ItemAffixRoller::pickProfile() const
{
    if (isCaster)
        return "caster";
    if (pch && pch->getProfession()->getFlags(pch).isSet(PROF_HYBRID))
        return "hybrid";
    return "melee";
}

double ItemAffixRoller::pickMultiplier(const Json::Value &section, const Json::Value &affix) const
{
    DLString cls = pch ? pch->getProfession()->getName() : DLString::emptyString;
    DLString profile = pickProfile();

    for (const Json::Value *src: { &affix, &section }) {
        const Json::Value &pick = (*src)["pick"];
        if (!pick.isObject())
            continue;
        for (const DLString &key: { cls, profile, DLString("all") })
            if (!key.empty() && pick.isMember(key))
                return item_points_by_level(pick[key], obj->level, -1);
    }

    return 1;
}

bool ItemAffixRoller::fitAllowed(const DLString &secName, const Json::Value &affix) const
{
    if (!pch)
        return true;

    static const char *names[] = { "str", "int", "wis", "dex", "con", "cha" };
    static const int stats[] = { STAT_STR, STAT_INT, STAT_WIS, STAT_DEX, STAT_CON, STAT_CHA };
    auto capped = [this](const DLString &name) {
        for (int k = 0; k < 6; k++)
            if (name == names[k])
                return pch->perm_stat[stats[k]] >= pch->getMaxStat(stats[k]);
        return false;
    };

    DLString value = affix["value"].asString();

    if (secName == "primary_stats") {
        if (!value.empty() && value.at(0) == '-')
            return true;
        DLString norm = (!value.empty() && value.at(0) == '+') ? DLString(value.substr(1)) : value;
        return !capped(norm);
    }

    // A pack with anything besides primary stats (mana, hitroll) keeps its worth.
    if (affix.isMember("affects")) {
        bool anyCapped = false;
        for (auto const &one: affix["affects"]) {
            DLString apply = one["apply"].asString();
            bool primary = false;
            for (int k = 0; k < 6; k++)
                if (apply == names[k])
                    primary = true;
            if (!primary)
                return true;
            if (capped(apply))
                anyCapped = true;
        }
        return !anyCapped;
    }

    return true;
}

double ItemAffixRoller::windowCurve() const
{
    double curve = item_level_curve(obj->level);
    // measure.armor_budget_mult [melee, caster]: armor tier windows per profile.
    // Melee armor came out ~1.35x today's stat strength on honest prices; Kit
    // cut it by a quarter (2026-10-04). Weapons keep the plain curve.
    if (kind == "armor")
        curve *= item_value("measure", "armor_budget_mult", 1.0, isCaster ? 1 : 0);
    return curve;
}

/** Slot weight: "deny" lists the slots it never rolls on (0), "slots" the
 *  boosted ones; the affix's own list wins over its section's. Every other
 *  slot weighs 1. */
double ItemAffixRoller::candidateWeight(const DLString &secName, const Json::Value &affix) const
{
    const Json::Value &section = item_affixes[secName];
    const Json::Value &deny = affix.isMember("deny") ? affix["deny"] : section["deny"];
    for (auto const &d: deny)
        if (d.asString() == slot)
            return 0;

    const Json::Value &slots = affix.isMember("slots") ? affix["slots"] : section["slots"];
    if (slots.isObject() && slots.isMember(slot))
        return slots[slot].asDouble();
    // slots_only: the listed slots are the only ones it rolls on.
    return section["slots_only"].asBool() ? 0 : 1;
}

/** Alignment band of an align value: good, evil or neutral, as align_bonus names it. */
static const char * align_band(int align)
{
    if (align >= 350)
        return "good";
    if (align <= -350)
        return "evil";
    return "neutral";
}

bool ItemAffixRoller::chargedPick(const Json::Value &section, const Json::Value &affix, Candidate &c) const
{
    const Json::Value &rule = section["by_kind"][kind];
    const Json::Value &tc = magic_items_config()["tiers"][DLString(tier).c_str()];
    if (!rule.isObject() || !tc.isObject())
        return false;

    std::vector<MagicSpellPick> cands;
    int a = align != ALIGN_NONE ? align : (pch ? (int)pch->alignment : 0);
    if (!magic_item_pool(rule["pool_prefix"].asString() + affix["pool"].asString(), tier, obj->level, a, cands))
        return false;

    // The charges may take this share of the tier window, the stats get the rest.
    const Json::Value &share = rule["share_pct"];
    double top = weapon_tier_table[tier - 1].max_m * windowCurve();
    double lo = top * share[0u].asInt() / 100.0;
    double hi = top * share[1u].asInt() / 100.0;

    // A charge range (wand_charges) runs from 1, a single number (staff_charges) is fixed.
    const Json::Value &range = tc[rule["charges"].asString()];
    int cmin = 1, cmax;
    if (range.isArray())
        cmax = range[1u].asInt();
    else
        cmin = cmax = range.asInt();
    cmax = min(cmax, magic_items_config()["max_charges"].asInt());
    if (cmax < cmin)
        return false;

    for (int i = (int)cands.size() - 1; i > 0; i--)
        std::swap(cands[i], cands[number_range(0, i)]);

    for (auto const &p: cands) {
        double one = p.price * 100;
        int fitLo = max(cmin, (int)ceil(lo / one));
        int fitHi = min(cmax, (int)floor(hi / one));
        if (fitLo > fitHi)
            continue;

        c.spell = p.spell;
        c.charges = number_range(fitLo, fitHi);
        c.price = (int)std::round(one * c.charges);
        return true;
    }

    return false;
}

int ItemAffixRoller::pickTongue(const Json::Value &affix) const
{
    if (!pch)
        return -1;

    vector<int> tongues;
    for (auto const &name: affix["skills"]) {
        Skill *skill = skillManager->findExisting(name.asString());
        if (!skill || !skill->usable(pch, false))
            continue;
        int learned = pch->getSkillData(skill->getIndex()).learned;
        if (learned >= 1 && learned < 100)
            tongues.push_back(skill->getIndex());
    }

    if (tongues.empty())
        return -1;
    return tongues.at(number_range(0, tongues.size() - 1));
}

// The +N% "learned" affix lands on a random skill the wearer knows (see apply),
// so price it at the mean value of those skills: a thief full of sneak/haggle
// pays less for it than a warrior whose picks are all combat skills.
double ItemAffixRoller::learnedValueMult() const
{
    if (!pch)
        return 1.0;

    double sum = 0;
    int cnt = 0;
    for (int sn = 0; sn < skillManager->size(); sn++) {
        PCSkillData &data = pch->getSkillData(sn);
        Skill *skill = skillManager->find(sn);
        if (data.learned > 1 && !data.isTemporary() && skill && skill->available(pch)) {
            sum += item_learn_skill_mult(sn);
            cnt++;
        }
    }
    return cnt > 0 ? sum / cnt : 1.0;
}

void ItemAffixRoller::collectCandidates()
{
    // Weight groups (item_affixes "_groups"): the members of a group (an affix's
    // or its section's "group") share the group's weight, so a section of
    // fourteen vulns weighs like one affix, not fourteen.
    const Json::Value &groupWeights = item_affixes["_groups"];
    std::map<DLString, int> groupSize;
    for (auto const &secName: item_affixes.getMemberNames()) {
        if (secName.empty() || secName.at(0) == '_')
            continue;
        const Json::Value &section = item_affixes[secName];
        DLString items = section.isMember("items") ? section["items"].asString() : DLString("armor");
        if (items != kind && items != "both")
            continue;
        for (auto const &affix: section["values"]) {
            DLString g = affix.isMember("group") ? affix["group"].asString() : section["group"].asString();
            if (!g.empty())
                groupSize[g]++;
        }
    }

    for (auto const &secName: item_affixes.getMemberNames()) {
        if (secName.empty() || secName.at(0) == '_')
            continue;

        const Json::Value &section = item_affixes[secName];
        DLString items = section.isMember("items") ? section["items"].asString() : DLString("armor");
        if (items != kind && items != "both")
            continue;

        int secFloor = section.isMember("tier") ? section["tier"].asInt() : WORST_TIER;

        for (auto const &affix: section["values"]) {
            DLString value = affix["value"].asString();
            int floor = affix.isMember("tier") ? affix["tier"].asInt() : secFloor;

            // A crafted piece's forced picks (its material, a refit replay) pass
            // every gate but the tier floor: the level window, alignment, class
            // weights, skill groups and the fit clauses. The piece keeps what its
            // material or its last roll gave it, whoever wears it at what level.
            bool forcedPick = crafted && forcedKeys.count(secName + ":" + value) > 0;
            if (forcedPick) {
                if (tier > floor || section["needs_boss"].asBool() || affix["needs_boss"].asBool())
                    continue;
            } else if (!candidateAllowed(section, affix, floor))
                continue;

            if (!valueAllowed(secName, value))
                continue;

            if (!forcedPick && !fitAllowed(secName, affix))
                continue;
            if (affix.isMember("skills") && pickTongue(affix) < 0)
                continue;

            // Skill groups: only groups the killer has a learned skill in.
            if (secName == "skill_group" && pch) {
                // hasElement first: lookup() registers a dummy for an unknown name.
                if (!skillGroupManager->hasElement(value))
                    continue;
                int gn = skillGroupManager->lookup(value);
                if (!forcedPick && playerGroups.count(gn) == 0)
                    continue;
            }

            double slotWeight = candidateWeight(secName, affix);
            if (slotWeight <= 0 && !forcedPick)
                continue;
            if (slotWeight <= 0)
                slotWeight = 1;

            double pick = pickMultiplier(section, affix);
            if (pick <= 0 && !forcedPick)
                continue;
            if (pick <= 0)
                pick = 1;

            // Weights are relative: x1000 leaves room for fractional multipliers.
            double w = slotWeight * 1000 * pick;
            DLString group = affix.isMember("group") ? affix["group"].asString() : section["group"].asString();
            if (!group.empty() && groupSize[group] > 0)
                w *= (groupWeights.isMember(group) ? groupWeights[group].asDouble() : 1.0) / groupSize[group];
            // align_bonus: an affix that suits the wearer's alignment comes up twice as often.
            if (align != ALIGN_NONE && affix.isMember("align_bonus")
                    && affix["align_bonus"].asString() == align_band(align))
                w *= 2;
            int weight = max(1, (int)std::round(w));

            Candidate c;
            c.charges = 0;

            bool allowed = true;
            int price;
            if (secName == "charged") {
                if (!chargedPick(section, affix, c))
                    continue;
                price = c.price;
            } else {
                price = candidatePrice(secName, affix, allowed);
                if (!allowed)
                    continue;
            }

            c.section = secName;
            c.value = value;
            c.norm = (!value.empty() && (value.at(0) == '-' || value.at(0) == '+')) ? value.substr(1) : value;
            c.affix = &affix;
            c.price = price;
            c.stack = affix.isMember("stack") ? max(1, affix["stack"].asInt()) : 1;
            c.weight = weight;
            c.tierFloor = floor;
            for (auto const &cf: affix["conflicts"])
                c.conflicts.insert(cf.asString());
            c.conflictsWith = section["conflictsWith"].asString();
            c.sigOnly = affix["needs_boss"].asBool();

            pool.push_back(c);
        }
    }
}

/** Two picks clash when either names the other, when their section allows one
 *  per item, or when they are the same thing under another section or sign:
 *  res fire / vuln fire / imm fire, hit / -hit, regeneration bit / regeneration
 *  buff. Stacking the very same affix is governed by 'stack', not here. */
bool ItemAffixRoller::conflicts(const Candidate &c, const std::map<int, int> &picked) const
{
    for (auto const &p: picked) {
        const Candidate &o = pool[p.first];

        if (&o == &c)
            continue;
        if (c.conflicts.count(o.value) || o.conflicts.count(c.value))
            return true;
        if (c.section == o.section && c.conflictsWith == "same_section")
            return true;
        if (c.norm == o.norm)
            return true;
    }

    return false;
}

std::map<int, int> ItemAffixRoller::resolveForced(const DLString &spec) const
{
    std::map<int, int> forced;
    for (auto const &w: spec.split(" ")) {
        std::list<DLString> parts = w.split(":");
        if (parts.size() < 2)
            continue;
        auto p = parts.begin();
        DLString sec = *p++;
        DLString value = *p++;
        // Values with spaces (worn buffs, procs) are written with '~':
        // '_' is a legal character of values (heal_gain, pack_hp).
        value.replaces("~", " ");
        int count = p != parts.end() ? max(1, atoi(p->c_str())) : 1;

        for (int i = 0; i < (int)pool.size(); i++) {
            if (pool[i].section != sec || pool[i].value != value)
                continue;
            if (forced.count(i))
                break;
            forced[i] = min(count, pool[i].stack);
            break;
        }
    }

    return forced;
}

DLString ItemAffixRoller::picksSpec() const
{
    DLString spec;
    for (auto const &p: chosen) {
        DLString value = pool[p.first].value;
        value.replaces(" ", "~");
        if (!spec.empty())
            spec += " ";
        spec += pool[p.first].section + ":" + value + ":" + DLString(p.second);
    }
    return spec;
}

/** Weighted random fill of the tier's M window. Negatives are ordinary picks:
 *  they buy room for more positives, down to the tier's penalty floor. The
 *  exhaustive weapon walk is not reused on purpose: 150 affixes at M prices have
 *  far too many subsets for it, and its depth-first reservoir would lean to the
 *  cheap end of the price list.
 *
 *  The tier caps the number of distinct affixes and of negative ones (Kit
 *  2026-10-02: an item with 15 crumbs has no character). To reach the budget
 *  inside the cap, a step prefers picks worth at least half of what each free
 *  slot still has to carry; restacking an affix already taken is always open. */
void ItemAffixRoller::pickAffixes(int minM, int maxM, int worstPenalty, int minAffixesM, int maxAffixesM,
                                  int maxNegativesM, const std::map<int, int> &forced)
{
    std::map<int, int> best;
    int bestDistance = -1, bestTotal = 0;

    int maxAffixes = maxAffixesM > 0 ? maxAffixesM : 1000;
    int maxNegatives = maxNegativesM > 0 ? maxNegativesM : 1000;

    // Model calibration knobs (one item model P7), data-only and off when absent:
    // measure.stat_floor {level: share | [melee, caster]} = least share of the target spent on stat
    // sections (measure.stat_sections), so a low-level item is not a bag of flat
    // effects; measure.max_affixes_bonus {level: n} = extra distinct affixes where
    // honest prices make affixes cheap (high level).
    double statFloor = 0;
    std::vector<bool> isStat(pool.size(), false);
    // floor_exempt (the sanctuary family): may be picked while the floor is unmet,
    // and once picked the floor is lifted for that item (Kit 2026-10-04).
    std::vector<bool> floorExempt(pool.size(), false);
    // Core picks (Kit 2026-10-04): measure.core_chance [p1, p2...] = percent chance of
    // each forced pick from measure.core_affixes[profile] (melee dr/hr/hp, caster
    // hp/mana/regen) before the weighted draw.
    std::vector<bool> isCore(pool.size(), false);
    // Top effects ("top": true -- sanctuary family, haste, resist weapon): at most
    // measure.top_max per item, and once one is on, the next weighs top_repeat_weight
    // (Kit 2026-10-04: two tops max, very rarely).
    std::vector<bool> isTop(pool.size(), false);
    for (int i = 0; i < (int)pool.size(); i++)
        isTop[i] = (*pool[i].affix)["top"].asBool();
    int topMax = (int)item_value("measure", "top_max", 1000);
    double topRepeat = item_value("measure", "top_repeat_weight", 1);
    // measure.primary_stat_cap: at most this many points of str/int/wis/dex/con/cha
    // per item, summed over every affix and stack (Kit 2026-10-27: +5).
    static const std::set<DLString> primaryNames = { "str", "int", "wis", "dex", "con", "cha" };
    std::vector<int> statPts(pool.size(), 0);
    for (int i = 0; i < (int)pool.size(); i++) {
        if (pool[i].price <= 0)
            continue;
        const Json::Value &affix = *pool[i].affix;
        if (pool[i].section == "primary_stats" && primaryNames.count(pool[i].norm))
            statPts[i] = max(0, modifier(affix, 1));
        else if (affix.isMember("affects"))
            for (auto const &one: affix["affects"])
                if (one.isMember("apply") && primaryNames.count(one["apply"].asString()))
                    statPts[i] += max(0, modifier(one, 1));
    }
    int statCap = (int)item_value("measure", "primary_stat_cap", 1000);

    std::vector<int> coreChance;
    // A crafted piece already carries the crafter's fixed stats: no core picks.
    if (kind == "armor" && !crafted) {
        const Json::Value &core = item_value_object("measure", "core_affixes")[isCaster ? "caster" : "melee"];
        std::set<DLString> coreNames;
        for (auto const &n: core)
            coreNames.insert(n.asString());
        for (int i = 0; i < (int)pool.size(); i++)
            isCore[i] = coreNames.count(pool[i].value) > 0 && pool[i].price > 0;
        for (auto const &c: item_value_object("measure", "core_chance"))
            coreChance.push_back(c.asInt());
    }
    // A value may be a number (both profiles) or [melee, caster].
    int col = isCaster ? 1 : 0;
    statFloor = item_points_by_level(item_value_object("measure", "stat_floor"), obj->level, col);
    // Legendary armor keeps a share of its budget in stats (Kit 2026-10-04).
    if (tier == BEST_TIER && kind == "armor")
        statFloor = max(statFloor, item_value("measure", "legendary_stat_share", 0));
    // ...and no stat floor either: the crafted fixed share is that floor.
    if (crafted)
        statFloor = 0;
    if (maxAffixesM > 0)
        maxAffixes += (int)std::round(item_points_by_level(item_value_object("measure", "max_affixes_bonus"), obj->level, col));
    int minAffixes = URANGE(1, minAffixesM > 0 ? minAffixesM : maxAffixes, maxAffixes);

    const Json::Value &secs = item_value_object("measure", "stat_sections");
    std::set<DLString> statSections;
    if (secs.isArray())
        for (auto const &sec: secs)
            statSections.insert(sec.asString());
    else
        statSections = { "armor_stats", "primary_stats", "affect_packs" };
    for (int i = 0; i < (int)pool.size(); i++) {
        isStat[i] = statSections.count(pool[i].section) > 0 && pool[i].price > 0;
        floorExempt[i] = (*pool[i].affix)["floor_exempt"].asBool();
    }

    auto draw = [](const vector<int> &eligible, const std::vector<int> &weightNow) -> int {
        int weights = 0;
        for (int e: eligible)
            weights += weightNow[e];
        if (weights <= 0)
            return -1;
        int dice = number_range(1, weights);
        for (int e: eligible) {
            dice -= weightNow[e];
            if (dice <= 0)
                return e;
        }
        return -1;
    };

    // The item rolls a random count of distinct positive affixes in the tier's
    // range (Kit 2026-10-26); penalties come on top, inside max_negatives. Each
    // is a plain weighted draw: no preference for dear affixes and no hunt for
    // the best fit, so items differ from each other.
    // Stacks of what was drawn then fill the budget towards a random target.
    // Three tries; the one closest to the window wins.
    for (int attempt = 0; attempt < 3; attempt++) {
        std::map<int, int> picked;
        int total = 0, penalty = 0, negatives = 0, positives = 0, statSpent = 0, tops = 0, statSum = 0;
        std::vector<int> weightNow(pool.size(), 0);
        int target = number_range(minM, maxM);
        int count = number_range(minAffixes, maxAffixes);
        bool relaxed = false;   // no stat pick fits any more: draw freely
        // A refit replay (window 0..0) keeps exactly its forced picks.
        if (maxM <= 0)
            count = 0;

        for (auto const &f: forced) {
            int i = f.first, n = f.second;
            picked[i] = n;
            total += pool[i].price * n;
            statSum += statPts[i] * n;
            if (isStat[i])
                statSpent += pool[i].price * n;
            if (pool[i].price < 0) {
                penalty += pool[i].price * n;
                negatives++;
            } else
                positives++;
            if (isTop[i])
                tops++;
        }

        for (int chance: coreChance) {
            if (positives >= count)
                break;
            if (number_range(1, 100) > chance)
                continue;
            vector<int> core;
            for (int i = 0; i < (int)pool.size(); i++) {
                weightNow[i] = pool[i].weight;
                if (!isCore[i] || pool[i].sigOnly || picked.count(i) || total + pool[i].price > maxM)
                    continue;
                if (statSum + statPts[i] > statCap)
                    continue;
                if (conflicts(pool[i], picked))
                    continue;
                core.push_back(i);
            }
            int pick = draw(core, weightNow);
            if (pick < 0)
                break;
            picked[pick] = 1;
            positives++;
            total += pool[pick].price;
            statSum += statPts[pick];
            if (isStat[pick])
                statSpent += pool[pick].price;
        }

        for (int step = 0; step < 32 && positives < count; step++) {
            vector<int> eligible;
            bool needStats = !relaxed && statFloor > 0 && statSpent < statFloor * target;

            for (int i = 0; i < (int)pool.size(); i++) {
                const Candidate &c = pool[i];

                if (c.sigOnly || picked.count(i))
                    continue;
                if (needStats && !isStat[i] && !floorExempt[i])
                    continue;
                if (c.price < 0 && negatives >= maxNegatives)
                    continue;
                if (total + c.price > maxM)
                    continue;
                if (statSum + statPts[i] > statCap)
                    continue;
                if (c.price < 0 && penalty + c.price < worstPenalty)
                    continue;
                if (conflicts(c, picked))
                    continue;

                weightNow[i] = c.weight;
                if (isTop[i]) {
                    if (tops >= topMax)
                        continue;
                    if (tops > 0)
                        weightNow[i] = max(1, (int)std::round(c.weight * topRepeat));
                }
                eligible.push_back(i);
            }

            int i = draw(eligible, weightNow);
            if (i < 0) {
                if (needStats) {
                    relaxed = true;   // the floor cannot be met inside the caps: retry this step freely
                    step--;
                    continue;
                }
                break;
            }

            if (pool[i].price < 0) {
                negatives++;
                penalty += pool[i].price;
            } else
                positives++;
            if (isTop[i])
                tops++;
            picked[i] = 1;
            total += pool[i].price;
            statSum += statPts[i];
            if (isStat[i])
                statSpent += pool[i].price;
            if (floorExempt[i])
                relaxed = true;
        }

        // Stacks: more of what the item already has, towards the target.
        for (int step = 0; step < 16 && total < target; step++) {
            vector<int> eligible;
            for (auto const &p: picked) {
                const Candidate &c = pool[p.first];
                if (c.sigOnly || c.price <= 0 || p.second >= c.stack || total + c.price > maxM)
                    continue;
                if (statSum + statPts[p.first] > statCap)
                    continue;
                weightNow[p.first] = c.weight;
                eligible.push_back(p.first);
            }
            int i = draw(eligible, weightNow);
            if (i < 0)
                break;
            picked[i]++;
            total += pool[i].price;
            statSum += statPts[i];
        }

        int distance = total < minM ? minM - total : (total > maxM ? total - maxM : 0);
        if (bestDistance < 0 || distance < bestDistance) {
            best = picked;
            bestDistance = distance;
            bestTotal = total;
        }
        if (distance == 0)
            break;
    }

    chosen = best;
    chosenTotal = bestTotal;
}

/*--------------------------------------------------------------------------
 * Applying the picks
 *-------------------------------------------------------------------------*/
int ItemAffixRoller::rolls() const
{
    if (rollsOverride > 0)
        return rollsOverride;
    return item_rolls(obj->level, slot);
}

/** unit: per measure roll (stats). mult: weapon style, per level. mod: flat. */
int ItemAffixRoller::modifier(const Json::Value &affix, int count) const
{
    int result;

    if (affix.isMember("unit"))
        result = affix["unit"].asInt() * rolls() * count;
    else if (affix.isMember("mult"))
        result = (int)(affix["mult"].asFloat() * count * obj->level) + affix["mod"].asInt();
    else
        result = affix["mod"].asInt() * count;

    return result;
}

void ItemAffixRoller::remember(Affect &af)
{
    af.type = gsn_none;
    af.duration = -1;
    af.level = obj->level;
    affects.push_back(af);
}

void ItemAffixRoller::applyPack(const Json::Value &list, int count)
{
    for (auto const &one: list) {
        Affect af;

        if (one.isMember("apply")) {
            af.location = apply_flags.value(one["apply"].asString());
            af.modifier = modifier(one, count);
            if (af.modifier != 0)
                remember(af);

        } else if (one.isMember("table")) {
            af.bitvector.setTable(FlagTableRegistry::getTable(one["table"].asString()));
            af.bitvector.setBits(one["bits"].asString());
            remember(af);
        }
    }
}

bool ItemAffixRoller::applyShared(const Candidate &c, int count)
{
    const Json::Value &affix = *c.affix;
    const DLString &sec = c.section;

    affixNames.insert(c.value);
    extraFlags.setBits(affix["extra"].asString());

    // +weight: the piece weighs weight_pct percent of its model weight
    // (item_weight reads the property).
    if (affix.isMember("weight_pct")) {
        obj->setProperty("weight_pct", affix["weight_pct"].asInt());
        return true;
    }

    // Any affix that spells out its affects is a pack, whatever its section.
    if (affix.isMember("affects")) {
        applyPack(affix["affects"], count);
        return true;
    }

    if (sec == "armor_stats" || sec == "affects_by_level" || sec == "primary_stats") {
        Affect af;
        af.location = apply_flags.value(c.norm);
        af.modifier = modifier(affix, count);
        if (af.modifier != 0)
            remember(af);

    } else if (sec == "skill_group") {
        if (!skillGroupManager->hasElement(c.value))
            return true;
        Affect af;
        af.global.setRegistry(skillGroupManager);
        af.global.fromString(c.value);
        af.location = APPLY_LEVEL;
        af.modifier = max(1, affix["mod"].asInt()) * count;
        remember(af);

    } else if (sec == "player") {
        if (!pch)
            return true;

        if (c.value == "skillgroup") {
            int gn = -1;
            // Same 2+ skills rule as the named groups, weighted by how many the killer has.
            int weights = 0;
            for (int g: playerGroups)
                weights += groupCounts[g];
            int dice = weights > 0 ? number_range(1, weights) : 0;
            for (int g: playerGroups) {
                dice -= groupCounts[g];
                if (dice <= 0) {
                    gn = g;
                    break;
                }
            }
            if (gn < 0)
                return true;
            Affect af;
            af.global.setRegistry(skillGroupManager);
            af.global.set(gn);
            af.location = APPLY_LEVEL;
            af.modifier = max(1, affix["mod"].asInt()) * count;
            remember(af);

        } else if (affix.isMember("skills")) {
            // An ancient language the wearer knows but not natively.
            int sn = pickTongue(affix);
            if (sn < 0)
                return true;
            Affect af;
            af.global.setRegistry(skillManager);
            af.global.set(sn);
            af.location = APPLY_LEARNED;
            af.modifier = affix["mod"].asInt() * count;
            remember(af);

        } else if (c.norm == "learned") {
            // Melee picks defensive/fightmaster skills x1.25 (Kit 2026-10-04).
            vector<int> preferred;
            if (pickProfile() == "melee")
                for (const char *g: { "defensive", "fightmaster" })
                    if (skillGroupManager->hasElement(g))
                        preferred.push_back(skillGroupManager->lookup(g));
            auto skillWeight = [&preferred](Skill *skill) {
                for (int g: preferred)
                    if (skill->getGroups().isSet(g))
                        return 5;
                return 4;
            };
            vector<int> mine;
            int weights = 0;
            for (int sn = 0; sn < skillManager->size(); sn++) {
                PCSkillData &data = pch->getSkillData(sn);
                Skill *skill = skillManager->find(sn);
                if (data.learned > 1 && !data.isTemporary() && skill && skill->available(pch)) {
                    mine.push_back(sn);
                    weights += skillWeight(skill);
                }
            }
            if (mine.empty())
                return true;
            int dice = number_range(1, weights), sn = mine.back();
            for (int m: mine) {
                dice -= skillWeight(skillManager->find(m));
                if (dice <= 0) {
                    sn = m;
                    break;
                }
            }
            Affect af;
            af.global.setRegistry(skillManager);
            af.global.set(sn);
            af.location = APPLY_LEARNED;
            af.modifier = affix["mod"].asInt() * count;
            remember(af);
        }

    } else if (sec == "extra") {
        extraFlags.setBits(c.value);

    } else if (sec == "charged") {
        // Read by 'use' (chargedprops.h). A charged ring or staff keeps its
        // stats at 0 charges; recharge refills it.
        const Json::Value &affixes = item_affixes;
        const Json::Value &rule = affixes[sec]["by_kind"][kind];
        const Json::Value &tc = magic_items_config()["tiers"][DLString(tier).c_str()];
        Json::Value ch;
        ch["spell"] = c.spell;
        ch["level"] = max(1, obj->level + tc["cast_offset"].asInt());
        ch["charges"] = c.charges;
        ch["max"] = c.charges;
        ch["where"] = rule["where"].asString();
        ch["crumble"] = false;
        obj->props["charged"] = ch;

    } else if (sec == "resists" || sec == "vulns" || sec == "immunes"
                || sec == "senses" || sec == "affects_with_bits") {
        DLString table = affix.isMember("table") ? affix["table"].asString()
                       : sec == "resists" ? "res_flags"
                       : sec == "vulns" ? "vuln_flags"
                       : sec == "immunes" ? "imm_flags"
                       : sec == "senses" ? "detect_flags"
                       : "affect_flags";
        Affect af;
        af.bitvector.setTable(FlagTableRegistry::getTable(table));
        af.bitvector.setBits(c.value);
        remember(af);

    } else {
        return false;
    }

    return true;
}

void ArmorGenerator::applyOne(const Candidate &c, int count)
{
    const Json::Value &affix = *c.affix;
    const DLString &sec = c.section;

    if (applyShared(c, count))
        return;

    if (sec == "worn_buff") {
        wornBuff = c.value;

    } else if (sec == "proc") {
        Json::Value p;
        p["spell"] = c.value;
        p["chance"] = proc_chance(c.tierFloor);
        if (affix.isMember("hp_below"))
            p["hp_below"] = affix["hp_below"].asInt();
        procs.append(p);

    } else if (sec == "material") {
        materialName = c.value;

    } else {
        warn("Armor generator: affix %s in unknown section %s.", c.value.c_str(), sec.c_str());
    }
}

void ArmorGenerator::applyAffixes()
{
    for (auto const &p: chosen)
        applyOne(pool[p.first], p.second);
}

/** Crafted armor's AC (craft/armor generateAC): level 0..100 -> 1..40, the tier
 *  shifts the top by 10% a step around rare, harder material adds, capped at 60.
 *  Indestructible materials (hardness -1) count as the hardest. */
void ArmorGenerator::assignAC()
{
    const material_t *m = material_by_name(materialName);
    int hardness = m ? m->hardness : 5;
    if (hardness < 0)
        hardness = 10;

    int a = 1 + obj->level * 39 / 100;
    int b = a * (100 + (3 - tier) * 10) / 100 + (hardness - 5);
    b = URANGE(1, b, 60);
    int lo = min(a, b), hi = max(a, b);

    // keepIdentity: rings, cloaks, belts and the like keep their own AC; on the
    // seven armor slots the rolled AC only ever raises the item's own.
    if (keepName) {
        if (!armor_slot_exists(slot))
            return;
        obj->value0(max(obj->value0(), number_range(lo, hi)));
        obj->value1(max(obj->value1(), number_range(lo, hi)));
        obj->value2(max(obj->value2(), number_range(lo, hi)));
        obj->value3(max(obj->value3(), number_range(lo, hi) / 2));
        return;
    }

    obj->value0(number_range(lo, hi));
    obj->value1(number_range(lo, hi));
    obj->value2(number_range(lo, hi));
    obj->value3(number_range(lo, hi) / 2);   // exotic, as crafted armor
}

/*--------------------------------------------------------------------------
 * Names
 *-------------------------------------------------------------------------*/
/** One prefix and one suffix out of the picks that carry words, the dearer the
 *  likelier. Weight is |price|: a named drawback (curse, glow) shows as often as
 *  a bonus of its size, so the name never hides what the item does to you. */
static const Json::Value *pick_part(const vector<ItemAffixRoller::NamePart> &parts)
{
    int total = 0;
    for (auto const &p: parts)
        total += p.weight;
    if (total <= 0)
        return 0;

    int dice = number_range(1, total);
    for (auto const &p: parts) {
        dice -= p.weight;
        if (dice <= 0)
            return p.affix;
    }
    return 0;
}

/** The chosen picks join the caller's own name parts (a weapon's two-handed
 *  adjective) before one of each is drawn. */
void ItemAffixRoller::pickNameParts(vector<NamePart> &prefixes, vector<NamePart> &suffixes,
                                    const Json::Value *&pre, int &a,
                                    const Json::Value *&suf, int &n) const
{
    for (auto const &p: chosen) {
        const Candidate &c = pool[p.first];
        int weight = max(1, abs(c.price * p.second));
        if ((*c.affix)["adjectives"].size() > 0)
            prefixes.push_back({weight, c.affix});
        if ((*c.affix)["nouns"].size() > 0)
            suffixes.push_back({weight, c.affix});
    }

    pre = pick_part(prefixes);
    suf = pick_part(suffixes);
    a = pre ? number_range(0, (*pre)["adjectives"].size() - 1) : -1;
    n = suf ? number_range(0, (*suf)["nouns"].size() - 1) : -1;
}

void ArmorGenerator::assignNames()
{
    vector<NamePart> prefixes, suffixes;
    const Json::Value *pre, *suf;
    int a, n;
    pickNameParts(prefixes, suffixes, pre, a, suf, n);

    auto word = [](const Json::Value *affix, const char *field, int idx) -> DLString {
        if (!affix || idx < 0)
            return DLString::emptyString;
        const Json::Value &forms = (*affix)[field];
        return idx < (int)forms.size() ? DLString(forms[idx].asString()) : DLString::emptyString;
    };

    DLString gender = nounConfig["gender"].asString();
    obj->gram_gender = MultiGender(gender.c_str());

    // Russian: Flexer pads, the adjective declined to the noun's gender.
    DLString adjRu = word(pre, "adjectives", a);
    obj->setShortDescr(random_item_compose_short(
        adjRu.empty() ? adjRu : Morphology::adjective(adjRu, obj->gram_gender),
        nounConfig["short"].asString(),
        word(suf, "nouns", n)), LANG_RU);

    // English: plain words.
    obj->setShortDescr(random_item_compose_short(
        word(pre, "adjectives_en", a),
        nounConfig["short_en"].asString(),
        word(suf, "nouns_en", n)), LANG_EN);

    // Ukrainian: nominatives declined by the sidecar; suffixes are fixed genitives.
    DLString gtag = random_item_gender_tag(nounConfig.isMember("gender_ua")
                        ? nounConfig["gender_ua"].asString() : gender);
    DLString baseUa, adjUa;
    random_item_decline_ua(nounConfig["short_ua"].asString(), "NOUN", gtag, baseUa);
    DLString adjUaWord = word(pre, "adjectives_ua", a);
    if (!adjUaWord.empty())
        random_item_decline_ua(adjUaWord, "ADJF", gtag, adjUa);
    obj->setShortDescr(random_item_compose_short(adjUa, baseUa, word(suf, "nouns_ua", n)), LANG_UA);

    obj->setDescription(nounConfig["long"].asString().c_str(), LANG_RU);
    obj->setDescription(nounConfig["long_en"].asString().c_str(), LANG_EN);
    obj->setDescription(nounConfig["long_ua"].asString().c_str(), LANG_UA);

    StringList keywords(nounConfig["name"].asString());
    DLString ua = nounConfig["short_ua"].asString();
    if (ua.find('|') == DLString::npos)
        keywords.addUnique(ua);
    obj->setKeyword(keywords.join(" ").c_str());

    // Tier colour on every language's name, as random weapons.
    DLString colour = weapon_tier_table[tier - 1].colour;
    if (!colour.empty())
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            DLString s = obj->getShortDescr((lang_t)l);
            obj->setShortDescr("{" + colour + s.colourStrip() + "{x", (lang_t)l);
        }
}

/** A boss's name for its signature piece: colours off, cut at the first comma
 *  ("Мизелак, Лич-Повелитель" -> "Мизелак"). */
static DLString sig_boss_name(NPCharacter *boss, char gcase, lang_t lang)
{
    DLString name = boss->getNameP(gcase, lang).colourStrip();
    DLString::size_type comma = name.find(',');
    if (comma != DLString::npos)
        name = name.substr(0, comma);
    name.stripWhiteSpace();
    return name;
}

ArmorGenerator & ArmorGenerator::signatureBoss(Character *boss)
{
    if (!boss || !boss->is_npc())
        return *this;

    NPCharacter *mob = boss->getNPC();
    sigBossRu = sig_boss_name(mob, '2', LANG_RU);
    sigBossUa = sig_boss_name(mob, '2', LANG_UA);

    // English shorts carry no article: "the red dragon" -> "red dragon's".
    DLString en = sig_boss_name(mob, '1', LANG_EN);
    for (const char *art: { "the ", "a ", "an " }) {
        DLString a(art);
        if (en.size() > a.size() && DLString(en.substr(0, a.size())).toLower() == a) {
            en = en.substr(a.size());
            break;
        }
    }
    sigBossEn = en;
    return *this;
}

/** A signature piece is named after the boss and a random trait from
 *  item_affixes signature.names: "Jher's malice", "злоба Джэра". The long
 *  description and keywords still come from the wear noun, so the piece says
 *  what it is when it lies on the floor. */
bool ArmorGenerator::assignSignatureNames()
{
    const Json::Value &names = item_affixes["signature"]["names"];
    if (!names.isArray() || names.size() == 0 || sigBossRu.empty())
        return false;

    const Json::Value &trait = names[(Json::ArrayIndex)number_range(0, names.size() - 1)];

    obj->gram_gender = MultiGender(trait["gender"].asString().c_str());

    obj->setShortDescr(random_item_compose_short(
        DLString::emptyString, trait["short"].asString(), sigBossRu), LANG_RU);
    obj->setShortDescr(random_item_compose_short(
        DLString::emptyString, trait["short_ua"].asString(),
        sigBossUa.empty() ? sigBossRu : sigBossUa), LANG_UA);

    DLString en = trait["short_en"].asString();
    if (!sigBossEn.empty()) {
        char last = sigBossEn.at(sigBossEn.size() - 1);
        en = sigBossEn + (last == 's' ? "' " : "'s ") + en;
    }
    obj->setShortDescr(en, LANG_EN);

    obj->setDescription(nounConfig["long"].asString().c_str(), LANG_RU);
    obj->setDescription(nounConfig["long_en"].asString().c_str(), LANG_EN);
    obj->setDescription(nounConfig["long_ua"].asString().c_str(), LANG_UA);

    StringList keywords(nounConfig["name"].asString());
    StringList traitWords(trait["name"].asString());
    for (auto const &w: traitWords)
        keywords.addUnique(w);
    obj->setKeyword(keywords.join(" ").c_str());

    DLString colour = weapon_tier_table[tier - 1].colour;
    if (!colour.empty())
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            DLString s = obj->getShortDescr((lang_t)l);
            obj->setShortDescr("{" + colour + s.colourStrip() + "{x", (lang_t)l);
        }

    return true;
}

void ArmorGenerator::assignColours()
{
    DLString colour = weapon_tier_table[tier - 1].colour;
    if (colour.empty())
        return;

    // An empty language stays empty, so display still falls back to Russian.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        DLString s = obj->getShortDescr((lang_t)l);
        if (!s.empty())
            obj->setShortDescr("{" + colour + s.colourStrip() + "{x", (lang_t)l);
    }
}

void ArmorGenerator::assignFlags()
{
    weapon_tier_t &t = weapon_tier_table[tier - 1];

    obj->setProperty("tier", tier);
    obj->setProperty("affixes", affixNames.toString());
    // What the affixes cost, in centi-M: diagnostics, and the base an enchant tops up from.
    obj->setProperty("measure_m", chosenTotal);

    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, t.extra.getValue());

    if (t.timer() > 0)
        obj->timer = t.timer();

    obj->cost = item_model_cost(chosenTotal, obj->level);

    if (!wornBuff.empty())
        obj->setProperty("wornbuff", wornBuff);
    else
        obj->removeProperty("wornbuff");

    // Shape for the Fenia weight helper (.tmp.weight.calculateWeight): the noun's
    // heft, and its own coverage where it covers less than the heft says (a circlet).
    if (nounConfig.isMember("heft"))
        obj->setProperty("heft", nounConfig["heft"].asString());
    if (nounConfig.isMember("coverage"))
        obj->setProperty("coverage", nounConfig["coverage"].asInt());

    if (!procs.empty())
        obj->props["combatcast"] = procs;
    // A crafted reroll may roll no proc where the last roll had one.
    else if (crafted && obj->props.isObject())
        obj->props.removeMember("combatcast");

    // Weight once material, heft and coverage are final: the item model's, or
    // the prototype's for a hand-made piece that keeps its shape.
    obj_set_weight(obj, item_instance_weight(obj));
}
