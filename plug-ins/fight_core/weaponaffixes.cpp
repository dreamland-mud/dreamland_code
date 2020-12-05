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
    align = ALIGN_NONE;
    requirements = 0L;
    retainChance = 100;
}

string affix_generator::dump() const
{
    ostringstream buf;
    buf << dlprintf("Tier %d, align %d, requirements %s, ", tier.num, align, requirements.to_string().c_str());
    buf << dlprintf("%d affixes, %d requires, %d forbids, %d preferences, ", 
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

    generateBuckets(0, 0, 0L);

    notice("Weapon generator: found %d result buckets for tier %d and %d affixes", 
            buckets.size(), tier.num, affixes.size());
}

/** Produces a single random affix combination out of all generated ones. */
list<affix_info> affix_generator::getSingleResult() const
{
    list<affix_info> result;
    bucket_mask_t bucket = randomBucket();

    for (unsigned int i = 0; i < affixes.size(); i++)
        if (bucket.test(i))
            result.push_back(affixes[i]);

    return result;
}

int affix_generator::getResultSize() const
{
    return buckets.size();
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

/** Choose a random set element. */
bucket_mask_t affix_generator::randomBucket() const 
{
    vector<bucket_mask_t> random_sample;
    sample(buckets.begin(), buckets.end(), 
            back_inserter(random_sample), 1, mt19937{random_device{}()});
    return random_sample.front();
}

/** Recursively produce masks were 1 marks an included affix, 0 marks an excluded affix.
 *  Each mask denotes a combination of affixes those total price matches prices for the tier. 
 */
void affix_generator::generateBuckets(int currentTotal, long unsigned int index, bucket_mask_t currentMask) 
{
    // Good combo, remember it and continue.
    if (currentTotal >= minPrice && currentTotal <= maxPrice)
        buckets.insert(currentMask);

    // Stop now: reached the end of affixes vector.
    if (index >= affixes.size())
        return;

    // Stop now: adding this or any subsequent price will still exceed maxPrice.
    if (currentTotal + affixes[index].price > maxPrice)
        return;

    // First check whether current affix doesn't conflict with any affix chosen earlier.
    if ((currentMask & exclusions[index]).none()) {
        // Explore all further combinations that can happen if this affix is included.
        currentMask.set(index);
        generateBuckets(currentTotal + affixes[index].price, index + 1, currentMask);
    }

    // First check whether current affix is required and cannot be excluded.
    if (!requirements.test(index)) {
        // Explore all further combinations that can happen if this affix is excluded.
        currentMask.reset(index);
        generateBuckets(currentTotal, index + 1, currentMask);
    }
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
        for (auto const &affix: weapon_affixes[section]["values"]) {

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

        if (!chance(retainChance))
            toErase.insert(ai.affixName);        
    }
    
    for (auto &affixName: toErase) {
//      warn("...erasing %s", affixName.c_str());
        int a;
        while ((a = getAffixIndex(affixName)) >= 0)
            affixes.erase(affixes.begin() + a);
    }
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

