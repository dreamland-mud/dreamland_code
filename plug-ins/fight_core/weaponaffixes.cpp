#include <jsoncpp/json/json.h>
#include <algorithm>
#include <random>

#include "weaponaffixes.h"
#include "weapontier.h"
#include "profiler.h"
#include "logstream.h"
#include "configurable.h"
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
affix_info::affix_info(const string &section, const string &affixName, int price, int stack, const Json::Value &entry) 
            : section(section), affixName(affixName), price(price), stack(stack), entry(entry)
{        
}

affix_info::~affix_info()
{
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

int affix_generator::getAffixIndex(const DLString &name)
{
    for (unsigned int i = 0; i < affixes.size(); i++)
        if (affixes[i].entry["value"].asString() == name)
            return i;
    return -1;
}

list<int> affix_generator::getAffixIndexes(const DLString &name)
{
    list<int> result;
    for (unsigned int i = 0; i < affixes.size(); i++)
        if (affixes[i].entry["value"].asString() == name)
            result.push_back(i);
    return result;
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
        for (auto const &affix: weapon_affixes[section]) {

            if (!checkTierThreshold(affix))
                continue;

            if (checkForbidden(affix))
                continue;

            if (!checkAlign(affix))
                continue;

            if (checkRequirementConflict(affix))
                continue;

            int price = affix["price"].asInt();
            DLString affixName = affix["value"].asString();
            sorted.push_back(affix_info(section, affixName, price, 1, affix));

#if 0
            // See if negative counterpart has to be generated for this affix.
            bool both = affix.isMember("both") ? affix["both"].asBool() : false;

            // Decide how many times this affix has to be repeated, from 1 to 'stack'.
            int stack = affix.isMember("stack") ? affix["stack"].asInt() : 1;
            for (int s = 1; s <= stack; s++) {
                int price = affix["price"].asInt();                
                sorted.push_back(affix_info(section, price * s, s, affix));
                if (both)
                    sorted.push_back(affix_info(section, -price * s, s, affix));
            }
#endif                
        }
    }

    // Sort all by price in ascending order.
    sorted.sort(sort_by_price);
    for (auto &p: sorted)
        affixes.push_back(p);

    // Exclude affixes that conflict with required ones.
    set<string> toErase;
    for (auto const &reqName: required) {
        int r = getAffixIndex(reqName);
        for (auto &c: affixes[r].entry["conflicts"]) {
            toErase.insert(c.asString());
        }
    }
    for (auto &affix: toErase) {
        int a = getAffixIndex(affix);
        affixes.erase(affixes.begin() + a);
    }

    // TODO: exclude affixes based on align bonuses, preferences and probabilities.
}

bool affix_generator::checkRequirementConflict(const Json::Value &affix) const
{
    for (auto const &conflictName: affix["conflicts"])
        if (required.count(conflictName.asString()) > 0)
            return true;

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

/** Expresses affix names from 'required' field as a bit mask. */
void affix_generator::markRequirements()
{
    for (auto const &reqName: required) {
        int r = getAffixIndex(reqName);
        requirements.set(r);
    }
}

/** Creates a NxN matrix of affix flags, marking those that are mutually exclusive.
 */
void affix_generator::markExclusions() 
{
    exclusions.resize(affixes.size());

    for (unsigned int i = 0; i < affixes.size(); i++) {            
        bucket_mask_t &exclusion = exclusions[i];
        affix_info &p = affixes[i];

        if (p.section == "material")
            for (unsigned int j = 0; j < affixes.size(); j++)
                if (i != j && affixes[j].section == p.section)
                    exclusion.set(j);

        for (auto &conflictName: p.entry["conflicts"]) {
            int index = getAffixIndex(conflictName.asString());
            if (index >= 0) {
                exclusion.set(index);
                exclusions[index].set(i);
            }
        }
    }

#if 0
    // Mark all stacked values such as +hr, -hr as mutually exclusive.
    for (unsigned int i = 0; i < affixes.size(); i++) {
        const affix_info &p = affixes[i];
        for (auto &same: getAffixIndexes(p.entry["value"].asString()))
            if (i != same)
                exclusions[i].set(same);
    }
#endif         
}



// Debug util: grab several good affix combinations for the tier. 
list<list<string>> random_weapon_affixes(int tier, int count, int align)
{
    list<list<string>> allNames;
    affix_generator gen(tier);
    gen.setAlign(align);
    gen.run();

    for (int i = 0; i < min(count, gen.getResultSize()); i++) {
        list<string> names;
        for (auto &pi: gen.getSingleResult())
            names.push_back(pi.entry["value"].asString());
        allNames.push_back(names);
    }

    return allNames;
}

