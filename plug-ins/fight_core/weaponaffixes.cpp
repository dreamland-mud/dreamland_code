#include <jsoncpp/json/json.h>
#include <algorithm>
#include <random>

#include "weaponaffixes.h"
#include "weapontier.h"
#include "profiler.h"
#include "logstream.h"
#include "configurable.h"
#include "alignment.h"
#include "dl_math.h"
#include "act.h"
#include "merc.h"
#include "def.h"

Json::Value weapon_affixes;
CONFIGURABLE_LOADED(fight, weapon_affixes)
{
    weapon_affixes = value;
}

/** Hard ceiling on how many recursion nodes generateBuckets may walk. The price and
 *  penalty prunes keep a healthy tier well under this, so it is only a safety net
 *  against a pathological affix pool -- but it guarantees the walk can never freeze the
 *  server again (the reservoir already holds a valid random pick when it trips). Each
 *  node is a few integer ops now (no set insert), so this bound is a fraction of a
 *  second, far below the 26-133s freezes the old full enumeration produced. */
static const unsigned long MAX_BUCKET_VISITS = 5000000;

/*-----------------------------------------------------------------------------
 * Weapon affixes
 *-----------------------------------------------------------------------------*/

/** Helper structure to access affix configuration. */
affix_info::affix_info(const string &section, int stack, const Json::Value &affix)
{
    this->section = section;
    this->stack = stack;
    this->affix = affix;

    affixName = affix["value"].asString();
    price = affix["price"].asInt() * stack;
    alignBonus = ALIGN_NONE;

    if (affix.isMember("align_bonus")) {
        DLString bonus = affix["align_bonus"].asString();
        alignBonus = align_table.value(bonus);
    }

    for (auto &c: affix["conflicts"])
        conflicts.insert(c.asString());
}

affix_info::~affix_info()
{
}

bool affix_info::equals(const affix_info &other) const
{
    return other.affixName == affixName
            && other.price == price;
}

string affix_info::normalizedName() const
{
    if (affixName.empty())
        return affixName;
        
    if (affixName.at(0) == '-' || affixName.at(0) == '+')
        return affixName.substr(1);

    return affixName;
}

static bool sort_by_price(const affix_info &p1, const affix_info &p2)
{
    return p1.price <= p2.price;
}

affix_generator::affix_generator(int t) : tier(weapon_tier_table[t-1]) 
{
    minPrice = tier.min_points;
    maxPrice = tier.max_points;
    worstPenalty = tier.worst_penalty;
    align = ALIGN_NONE;
    requirements = 0L;
    retainChance = 100;
    pch = 0;
}

string affix_generator::dump() const
{
    ostringstream buf;
    buf << fmt(0, "Tier %d, align %d, prices %d-%d (%d), requirements %s, ", 
                tier.num, align, 
                minPrice, maxPrice, worstPenalty,
                requirements.to_string().c_str());
    buf << fmt(0, "%d affixes, %d requires, %d forbids, %d preferences, ", 
                     affixes.size(), required.size(), forbidden.size(), preferences.size()) << endl;
    buf << "Affixes: ";
    for (auto const &pi: affixes)
        buf << pi.affixName << " ";
    buf << endl;
    return buf.str();
}

/** Remember align restriction. */
void affix_generator::setAlign(int align) 
{
    this->align = align;
}

/** Mark a certain affix as forbidden in all combinations. */
void affix_generator::addForbidden(const DLString &name)
{
    forbidden.insert(name);
}

/** Mark a certain affix as required (always chosen). */
void affix_generator::addRequired(const DLString &name)
{
    required.insert(name);
}

/** Mark a certain affix as preferred (always included in initial set). */
void affix_generator::addPreference(const DLString &name)
{
    preferences.insert(name);
}

void affix_generator::setup() 
{
    collectAffixesForTier();
    markRequirements();
    markExclusions();
}

void affix_generator::run() 
{
    ProfilerBlock prof("generate affixes", 10);

    if (affixes.empty())
        setup();

    chosenBucket.reset();
    bucketCount = 0;
    visitCount = 0;
    generateBuckets(0, 0, 0, 0L);

    notice("Weapon generator: found %lu result buckets for tier %d and %d affixes (%lu nodes walked)",
            bucketCount, tier.num, (int)affixes.size(), visitCount);
}

/** Produces the single random affix combination the reservoir landed on. */
list<affix_info> affix_generator::getSingleResult() const
{
    list<affix_info> result;

    for (unsigned int i = 0; i < affixes.size(); i++)
        if (chosenBucket.test(i))
            result.push_back(affixes[i]);

    return result;
}

int affix_generator::getResultSize() const
{
    return bucketCount;
}

list<affix_info *> affix_generator::getAffixes(const DLString &name)
{
    list<affix_info *> result;

    for (auto &ai: affixes)
        if (ai.affixName == name)
            result.push_back(&ai);

    return result;
}

int affix_generator::getAffixIndex(const DLString &name)
{
    for (unsigned int i = 0; i < affixes.size(); i++)
        if (affixes[i].affixName == name)
            return i;
    return -1;
}

/** Recursively walk the affix combinations, reservoir-sampling one uniformly.
 *
 *  Affixes are sorted by price ascending. A "bucket" is a complete subset: the affixes
 *  chosen so far with everything from 'index' on excluded. Such a subset is FINAL --
 *  and gets recorded -- at exactly one place: either the last affix is decided
 *  (index == size), or the cheapest remaining affix already overshoots the price
 *  ceiling, so nothing more can be added. Recording it there, once, is what keeps each
 *  distinct combination counted a single time, so the reservoir stays uniform (the old
 *  code recorded at every node and leaned on an unordered_set to dedup -- which is
 *  exactly the set that grew to tens of millions of entries).
 *
 *  Two prunes hold the walk down: the price ceiling stops the include branch, and the
 *  penalty floor (worstPenalty, from the tier's max_penalty) stops it packing on more
 *  negative affixes than the tier allows -- without a real floor, tier 1 admitted every
 *  negative pile and exploded. MAX_BUCKET_VISITS is the final backstop.
 */
void affix_generator::generateBuckets(int currentTotal, int currentPenalty, long unsigned int index, bucket_mask_t currentMask)
{
    if (visitCount >= MAX_BUCKET_VISITS)
        return;
    visitCount++;

    // Final subset: no affix left to decide, or the cheapest remaining one (affixes are
    // ascending) already overshoots the ceiling, so nothing more can be added. Record it
    // once if it lands in the tier's price window and honours the penalty floor.
    if (index >= affixes.size()
        || currentTotal + affixes[index].price > maxPrice)
    {
        if (currentTotal >= minPrice && currentTotal <= maxPrice
            && currentPenalty >= worstPenalty)
        {
            bucketCount++;
            // Reservoir sampling of size 1: the k-th valid combination replaces the
            // pick with probability 1/k, so chosenBucket ends up uniform over all of
            // them -- the same distribution the old enumerate-then-sample produced,
            // without holding them all in memory.
            if (number_range(1, bucketCount) == 1)
                chosenBucket = currentMask;
        }
        return;
    }

    int myPrice = affixes[index].price;
    int nextTotal = currentTotal + myPrice;
    int nextPenalty = myPrice < 0 ? (currentPenalty + myPrice) : currentPenalty;

    // Include this affix -- unless it would breach the penalty floor (a negative that
    // drops the running penalty past worstPenalty can never recover: penalty only
    // decreases) or conflict with one already chosen. The price ceiling is guaranteed
    // by the gate above.
    if (nextPenalty >= worstPenalty && (currentMask & exclusions[index]).none()) {
        currentMask.set(index);
        generateBuckets(nextTotal, nextPenalty, index + 1, currentMask);
        currentMask.reset(index);
    }

    // Exclude this affix -- unless it is required and cannot be left out.
    if (!requirements.test(index))
        generateBuckets(currentTotal, currentPenalty, index + 1, currentMask);
}

/** Creates a vector of all affixes that are allowed for the tier, sorted by price in ascending order. */
void affix_generator::collectAffixesForTier()
{
    list<affix_info> sorted;

    if (!weapon_affixes.isObject()) {
        bug("weapon_affixes is not a well-formed json object.");
        return;
    }

    // Collect all affixes that are not forbidden or restricted by align or price.
    for (auto &section: weapon_affixes.getMemberNames()) {
        const Json::Value &one_section = weapon_affixes[section];

        // Exclude player-specific sections if no player is configured.
        if (!checkPlayer(one_section))
            continue;

        for (auto const &affix: one_section["values"]) {

            if (!checkTierThreshold(affix))
                continue;

            if (checkForbidden(affix))
                continue;

            if (!checkAlign(affix))
                continue;

            // Decide how many times this affix has to be repeated, from 1 to 'stack'.
            int stack = affix.isMember("stack") ? affix["stack"].asInt() : 1;
            for (int s = 1; s <= stack; s++) {
                sorted.push_back(affix_info(section, s, affix));
            }
        }
    }

    // Sort all by price in ascending order.
    sorted.sort(sort_by_price);
    for (auto &p: sorted) {
        affixes.push_back(p);
    }

    set<string> toErase;

    // Exclude affixes that conflict with required ones.
    for (auto const &reqName: required) {
        for (auto const &req: getAffixes(reqName))
            // For each of the 'required' affixes, compare them with all available ones.
            for (auto const &affix: affixes)
                if (checkMutualConflict(*req, affix))
                    toErase.insert(affix.affixName);
    }

    // Keep preferred affixes, all others have a chance to get evicted.
    for (auto &ai: affixes) {
        if (required.count(ai.affixName) > 0)
            continue;

        if (preferences.count(ai.affixName) > 0)
            continue;
            
        if (checkAlignBonus(ai))
            continue;

        // A non-preferred affix has a chance to be evicted from this roll's pool --
        // negatives too, not only positives. Keeping every negative available on a
        // legendary was half of why tier 1 branched into tens of millions of combos;
        // culling them at the same gentle rate keeps per-weapon variety (a weapon still
        // draws random negatives) while shrinking the walk. Across many weapons every
        // negative still appears -- a different subset survives each roll.
        if (!chance(retainChance/2))
            toErase.insert(ai.affixName);
    }
    
    for (auto &affixName: toErase) {
        int a;
        while ((a = getAffixIndex(affixName)) >= 0)
            affixes.erase(affixes.begin() + a);
    }
}

/** See if this section is player-specific, false by default. */
bool affix_generator::checkPlayer(const Json::Value &one_section) const
{
    if (pch)
        return true;

    if (!one_section.isMember("needs_player"))
        return true;

    return !one_section["needs_player"].asBool();
}

bool affix_generator::checkMutualConflict(const affix_info &a1, const affix_info &a2)
{
    if (a1.equals(a2))
        return false;

    // Check if the affixes are mentioned in "conflicts" field of each other.
    if (a1.conflicts.count(a2.affixName) > 0)
        return true;

    if (a2.conflicts.count(a1.affixName) > 0)
        return true;
    
    // Check if section's "conflictsWith" field is applicable.
    if (a1.section != a2.section)
        return false;
    if (!weapon_affixes[a1.section].isMember("conflictsWith"))
        return false;

    DLString mode = weapon_affixes[a1.section]["conflictsWith"].asString();
    if (mode == "same_section")
        return a1.affixName != a2.affixName;

    if (mode == "same_value")
        return a1.normalizedName() == a2.normalizedName();

    return false;
}

bool affix_generator::checkTierThreshold(const Json::Value &affix) const
{
    int threshold = affix.isMember("tier") ? affix["tier"].asInt() : WORST_TIER;
    return tier.num <= threshold;
}

bool affix_generator::checkForbidden(const Json::Value &affix) const
{
    return forbidden.count(affix["value"].asString()) > 0;
}

bool affix_generator::checkAlign(const Json::Value &affix) const
{
    if (align == ALIGN_NONE)
        return true;

    if (!affix.isMember("align"))
        return true;

    const Json::Value &range = affix["align"];
    if (!range.isArray() || range.size() != 2) {
        bug("weapon generator: invalid align range for affix %s", affix["value"].asCString());
        return true;
    }

    int align_min = range[0].asInt();
    int align_max = range[1].asInt();
    return align_min <= align && align <= align_max;
}

bool affix_generator::checkAlignBonus(const affix_info &ai) const
{
    if (align == ALIGN_NONE)
        return false;

    return ALIGN_NUMBER(align) == ai.alignBonus;
}

/** Expresses affix names from 'required' field as a bit mask. */
void affix_generator::markRequirements()
{
    for (auto const &reqName: required) {
        int r = getAffixIndex(reqName);
        if (r < 0)
            warn("weapon generator: requirement %s not honoured", reqName.c_str());
        else
            requirements.set(r);
    }
}

/** Creates a NxN matrix of affix flags, marking those that are mutually exclusive.
 */
void affix_generator::markExclusions() 
{
    exclusions.resize(affixes.size());

    for (unsigned int a = 0; a < affixes.size(); a++) {
        affix_info &affix = affixes[a];

        for (unsigned int o = 0; o < affixes.size(); o++) {
            affix_info &other = affixes[o];
            if (checkMutualConflict(affix, other)) {
                exclusions[a].set(o);
                exclusions[o].set(a);
            }
        }
    }
}

// Debug util: grab several good affix combinations for the tier. 
list<list<string>> random_weapon_affixes(int tier, int count, int align, int chance)
{
    list<list<string>> allNames;
    affix_generator gen(tier);
    gen.setAlign(align);
    gen.setRetainChance(chance);
    gen.run();

    for (int i = 0; i < min(count, gen.getResultSize()); i++) {
        list<string> names;
        for (auto &pi: gen.getSingleResult())
            names.push_back(pi.affixName);
        allNames.push_back(names);
    }

    return allNames;
}

