#include <algorithm>
#include <cmath>

#include "armorgenerator.h"
#include "weapongenerator.h"
#include "weapontier.h"
#include "itemvalue.h"
#include "itemmodel.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillreference.h"
#include "core/object.h"
#include "pcharacter.h"
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

int item_affix_price(const DLString &section, const DLString &value, bool caster)
{
    if (!item_affixes.isMember(section))
        return 0;

    for (auto const &affix: item_affixes[section]["values"])
        if (affix["value"].asString() == value)
            return (caster ? affix["price_caster"] : affix["price_melee"]).asInt();

    return 0;
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
        : obj(obj), pch(pch), tier(tier), kind(kind), slot(slot), isCaster(false), align(ALIGN_NONE),
          chosenTotal(0), extraFlags(0, &extra_flags)
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
            playerGroups.insert(g);
    }
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
        : ItemAffixRoller(obj, pch, tier, "armor", slot), procs(Json::arrayValue)
{
}

bool ArmorGenerator::run()
{
    if (!armor_slot_exists(slot)) {
        warn("Armor generator: no names configured for slot %s.", slot.c_str());
        return false;
    }

    if (tier < BEST_TIER || tier > WORST_TIER || (int)weapon_tier_table.size() < tier) {
        warn("Armor generator: bad tier %d.", tier);
        return false;
    }

    if (obj->level < 1)
        obj->level = 1;

    if (!pickNoun())
        return false;

    learnPlayerGroups();
    collectCandidates();

    const weapon_tier_t &t = weapon_tier_table[tier - 1];
    double curve = windowCurve();
    pickAffixes((int)(t.min_m * curve), (int)(t.max_m * curve), t.worst_penalty_m,
                t.max_affixes_m, t.max_negatives_m, std::set<int>());
    applyAffixes();

    // The material affix, if any, was applied above; otherwise the noun's own.
    if (materialName.empty())
        materialName = defaultMaterial();
    obj->setMaterial(materialName.c_str());

    assignAC();
    assignNames();
    assignFlags();

    flushAffects();

    notice("rand_armor: created item %s [%d] [%lld] slot %s tier %d affixes [%s] level %d",
            obj->getShortDescr('1', LANG_DEFAULT).c_str(),
            obj->pIndexData->vnum, obj->getID(), slot.c_str(), tier,
            obj->getProperty("affixes").c_str(), obj->level);

    return true;
}

/*--------------------------------------------------------------------------
 * Noun and material
 *-------------------------------------------------------------------------*/
static bool material_is_metal(const DLString &name)
{
    const material_t *m = material_by_name(name);
    return m && IS_SET(m->type, MAT_METAL);
}

/** Materials: only what the chosen noun is made of. */
bool ArmorGenerator::valueAllowed(const DLString &secName, const DLString &value) const
{
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

    // Boss signatures are filled from the dead boss by the drop site, not rolled.
    if (section["needs_boss"].asBool() || affix["needs_boss"].asBool())
        return false;

    if ((section["needs_player"].asBool() || affix["needs_player"].asBool()) && !pch)
        return false;

    if (affix.isMember("align") && align != ALIGN_NONE) {
        const Json::Value &range = affix["align"];
        if (range.size() == 2 && (align < range[0u].asInt() || align > range[1u].asInt()))
            return false;
    }

    // Level windows (one item model): ac ends at 40, sanctuary-family buffs start at 30.
    if (item_model_enabled() && !item_level_window_ok(affix, obj->level))
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
    if (item_model_enabled() && modelPoints(secName, affix, points))
        return modelPrice(points);

    return (isCaster ? affix["price_caster"] : affix["price_melee"]).asInt();
}

static double table_points(const DLString &table, const DLString &bitNames, bool caster)
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
        return item_flag_points(bits, caster, 0, 0);
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
                points += table_points(one["table"].asString(), one["bits"].asString(), isCaster);
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
        points = (value == "learned" ? w.learnSkill : w.skillLevel) * max(1, affix["mod"].asInt());
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
        points = table_points(table, value, isCaster);
        return true;
    }
    if (secName == "worn_buff" && affix.isMember("points_by_level")) {
        points = item_points_by_level(affix["points_by_level"], obj->level, col);
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

bool ItemAffixRoller::fitAllowed(const DLString &secName, const DLString &value) const
{
    if (!pch)
        return true;

    if (secName == "primary_stats") {
        DLString norm = (!value.empty() && (value.at(0) == '-' || value.at(0) == '+')) ? DLString(value.substr(1)) : value;
        static const char *names[] = { "str", "int", "wis", "dex", "con", "cha" };
        static const int stats[] = { STAT_STR, STAT_INT, STAT_WIS, STAT_DEX, STAT_CON, STAT_CHA };
        for (int k = 0; k < 6; k++)
            if (norm == names[k] && value.at(0) != '-')
                return pch->perm_stat[stats[k]] + pch->mod_stat[stats[k]] < pch->getMaxStat(stats[k]);
        return true;
    }

    if (secName == "resists" || secName == "immunes") {
        Affect af;
        af.bitvector.setTable(FlagTableRegistry::getTable(secName == "resists" ? "res_flags" : "imm_flags"));
        af.bitvector.setBits(value);
        bitstring_t bits = af.bitvector;
        if (bits == 0)
            return true;
        if (((bitstring_t)pch->imm_flags & bits) == bits)
            return false;
        if (secName == "resists" && ((bitstring_t)pch->res_flags & bits) == bits)
            return false;
    }

    return true;
}

double ItemAffixRoller::windowCurve() const
{
    return item_model_enabled() ? item_level_curve(obj->level) : 1.0;
}

int ItemAffixRoller::candidateWeight(const DLString &secName, const Json::Value &affix) const
{
    const Json::Value &section = item_affixes[secName];
    const Json::Value &slots = affix.isMember("slots") ? affix["slots"] : section["slots"];
    return slots.isObject() && slots.isMember(slot) ? slots[slot].asInt() : 1;
}

void ItemAffixRoller::collectCandidates()
{
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

            if (!candidateAllowed(section, affix, floor))
                continue;

            if (!valueAllowed(secName, value))
                continue;

            if (item_model_enabled() && !fitAllowed(secName, value))
                continue;

            // Skill groups: only groups the killer has a learned skill in.
            if (secName == "skill_group" && pch) {
                // hasElement first: lookup() registers a dummy for an unknown name.
                if (!skillGroupManager->hasElement(value))
                    continue;
                int gn = skillGroupManager->lookup(value);
                if (playerGroups.count(gn) == 0)
                    continue;
            }

            int weight = candidateWeight(secName, affix);
            if (weight <= 0)
                continue;

            bool allowed = true;
            int price = candidatePrice(secName, affix, allowed);
            if (!allowed)
                continue;

            Candidate c;
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
void ItemAffixRoller::pickAffixes(int minM, int maxM, int worstPenalty, int maxAffixesM, int maxNegativesM,
                                  const std::set<int> &forced)
{
    std::map<int, int> best;
    int bestDistance = -1, bestTotal = 0;

    int maxAffixes = maxAffixesM > 0 ? maxAffixesM : 1000;
    int maxNegatives = maxNegativesM > 0 ? maxNegativesM : 1000;

    for (int attempt = 0; attempt < 30; attempt++) {
        std::map<int, int> picked;
        int total = 0, penalty = 0, negatives = 0;
        int target = number_range(minM, maxM);

        for (int i: forced) {
            picked[i] = 1;
            total += pool[i].price;
            if (pool[i].price < 0) {
                penalty += pool[i].price;
                negatives++;
            }
        }

        for (int step = 0; step < 16 && total < target; step++) {
            vector<int> big, small;
            int bigWeights = 0, smallWeights = 0;
            int slotsLeft = max(1, maxAffixes - (int)picked.size());
            int floorPrice = (target - total) / slotsLeft / 2;

            for (int i = 0; i < (int)pool.size(); i++) {
                const Candidate &c = pool[i];
                auto it = picked.find(i);
                bool fresh = (it == picked.end());

                if (!fresh && it->second >= c.stack)
                    continue;
                if (fresh && (int)picked.size() >= maxAffixes)
                    continue;
                if (fresh && c.price < 0 && negatives >= maxNegatives)
                    continue;
                if (total + c.price > maxM)
                    continue;
                if (c.price < 0 && penalty + c.price < worstPenalty)
                    continue;
                if (fresh && conflicts(c, picked))
                    continue;

                // A negative buys budget room, it never fills a slot's share.
                if (c.price < 0 || c.price >= floorPrice) {
                    big.push_back(i);
                    bigWeights += c.weight;
                } else {
                    small.push_back(i);
                    smallWeights += c.weight;
                }
            }

            vector<int> &eligible = big.empty() ? small : big;
            int weights = big.empty() ? smallWeights : bigWeights;

            if (eligible.empty() || weights <= 0)
                break;

            int dice = number_range(1, weights), i = -1;
            for (int e: eligible) {
                dice -= pool[e].weight;
                if (dice <= 0) {
                    i = e;
                    break;
                }
            }
            if (i < 0)
                break;

            if (picked.find(i) == picked.end() && pool[i].price < 0)
                negatives++;
            picked[i]++;
            total += pool[i].price;
            if (pool[i].price < 0)
                penalty += pool[i].price;
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
            int gn = random_item_skillgroup(pch);
            if (gn < 0)
                return true;
            Affect af;
            af.global.setRegistry(skillGroupManager);
            af.global.set(gn);
            af.location = APPLY_LEVEL;
            af.modifier = max(1, affix["mod"].asInt()) * count;
            remember(af);

        } else if (c.value == "learned") {
            vector<int> mine;
            for (int sn = 0; sn < skillManager->size(); sn++) {
                PCSkillData &data = pch->getSkillData(sn);
                Skill *skill = skillManager->find(sn);
                if (data.learned > 1 && !data.isTemporary() && skill && skill->available(pch))
                    mine.push_back(sn);
            }
            if (mine.empty())
                return true;
            Affect af;
            af.global.setRegistry(skillManager);
            af.global.set(mine[number_range(0, mine.size() - 1)]);
            af.location = APPLY_LEARNED;
            af.modifier = affix["mod"].asInt() * count;
            remember(af);
        }

    } else if (sec == "extra") {
        extraFlags.setBits(c.value);

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

void ArmorGenerator::assignFlags()
{
    weapon_tier_t &t = weapon_tier_table[tier - 1];

    obj->setProperty("tier", tier);
    obj->setProperty("affixes", affixNames.toString());
    // What the affixes cost, in centi-M: diagnostics, and the base an enchant tops up from.
    obj->setProperty("measure_m", chosenTotal);

    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, t.extra.getValue());

    if (t.weeks > 0)
        obj->timer = t.weeks * Date::SECOND_IN_WEEK / Date::SECOND_IN_MINUTE;

    obj->cost = item_model_enabled() ? item_model_cost(chosenTotal, obj->level)
                                     : 5 * (WORST_TIER + 1 - tier) * obj->level;

    if (!wornBuff.empty())
        obj->setProperty("wornbuff", wornBuff);

    // Shape for the Fenia weight helper (.tmp.weight.calculateWeight): the noun's
    // heft, and its own coverage where it covers less than the heft says (a circlet).
    if (nounConfig.isMember("heft"))
        obj->setProperty("heft", nounConfig["heft"].asString());
    if (nounConfig.isMember("coverage"))
        obj->setProperty("coverage", nounConfig["coverage"].asInt());

    if (!procs.empty())
        obj->props["combatcast"] = procs;
}
