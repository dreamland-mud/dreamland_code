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

/** Helper structure to access prefix configuration. */
prefix_info::prefix_info(const string &section, const string &affixName, int price, int stack, Json::Value entry) 
            : section(section), affixName(affixName), price(price), stack(stack), entry(entry)
{        
}

static bool sort_by_price(const prefix_info &p1, const prefix_info &p2)
{
    return p1.price <= p2.price;
}

prefix_generator::prefix_generator(int t) : tier(weapon_tier_table[t-1]) 
{
    minPrice = tier.min_points;
    maxPrice = tier.max_points;
    align = ALIGN_NONE;
    requirements = 0L;
}

/** Remember align restriction. */
void prefix_generator::setAlign(int align) 
{
    this->align = align;
}

/** Mark a certain affix as forbidden in all combinations. */
void prefix_generator::addForbidden(const DLString &name)
{
    forbidden.insert(name);
}

/** Mark a certain affix as required (always chosen). */
void prefix_generator::addRequired(const DLString &name)
{
    required.insert(name);
}

void prefix_generator::run() 
{
    ProfilerBlock prof("generate prefixes", 10);

    collectPrefixesForTier();
    markRequirements();
    markExclusions();
    generateBuckets(0, 0, 0L);

    notice("Weapon generator: found %d result buckets for tier %d and %d prefixes", 
            buckets.size(), tier.num, prefixes.size());
}

/** Produces a single random prefix combination out of all generated ones. */
list<prefix_info> prefix_generator::getSingleResult() const
{
    list<prefix_info> result;
    bucket_mask_t bucket = randomBucket();

    for (unsigned int i = 0; i < prefixes.size(); i++)
        if (bucket.test(i))
            result.push_back(prefixes[i]);

    return result;
}

int prefix_generator::getResultSize() const
{
    return buckets.size();
}

int prefix_generator::getPrefixIndex(const DLString &name)
{
    for (unsigned int i = 0; i < prefixes.size(); i++)
        if (prefixes[i].entry["value"].asString() == name)
            return i;
    return -1;
}

list<int> prefix_generator::getPrefixIndexes(const DLString &name)
{
    list<int> result;
    for (unsigned int i = 0; i < prefixes.size(); i++)
        if (prefixes[i].entry["value"].asString() == name)
            result.push_back(i);
    return result;
}

/** Choose a random set element. */
bucket_mask_t prefix_generator::randomBucket() const 
{
    vector<bucket_mask_t> random_sample;
    sample(buckets.begin(), buckets.end(), 
            back_inserter(random_sample), 1, mt19937{random_device{}()});
    return random_sample.front();
}

/** Recursively produce masks were 1 marks an included prefix, 0 marks an excluded prefix.
 *  Each mask denotes a combination of prefixes those total price matches prices for the tier. 
 */
void prefix_generator::generateBuckets(int currentTotal, long unsigned int index, bucket_mask_t currentMask) 
{
    if (currentTotal >= minPrice && currentTotal <= maxPrice)
        // Good combo, remember it and continue.
        buckets.insert(currentMask);
    else if (currentTotal > maxPrice) 
        // Stop now: adding more prefixes will only make the price bigger.
        return;

    if (index >= prefixes.size())
        // Stop now: reached the end of prefixes vector.
        return;

    // Stop now: adding this or any subsequent price will still exceed maxPrice.
    // TODO: measure if it gives any advantage in processing time.
    if (currentTotal + prefixes[index].price > maxPrice)
        return;

    // First check whether current prefix doesn't conflict with any prefix chosen earlier.
    if ((currentMask & exclusions[index]).none()) {
        // Explore all further combinations that can happen if this prefix is included.
        currentMask.set(index);
        generateBuckets(currentTotal + prefixes[index].price, index + 1, currentMask);
    }

    // First check whether current prefix is required and cannot be excluded.
    if (!requirements.test(index)) {
        // Explore all further combinations that can happen if this prefix is excluded.
        currentMask.reset(index);
        generateBuckets(currentTotal, index + 1, currentMask);
    }
}

/** Creates a vector of all prefixes that are allowed for the tier, sorted by price in ascending order. */
void prefix_generator::collectPrefixesForTier()
{
    list<prefix_info> sorted;

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
            sorted.push_back(prefix_info(section, affixName, price, 1, affix));

#if 0
            // See if negative counterpart has to be generated for this affix.
            bool both = affix.isMember("both") ? affix["both"].asBool() : false;

            // Decide how many times this affix has to be repeated, from 1 to 'stack'.
            int stack = affix.isMember("stack") ? affix["stack"].asInt() : 1;
            for (int s = 1; s <= stack; s++) {
                int price = affix["price"].asInt();                
                sorted.push_back(prefix_info(section, price * s, s, affix));
                if (both)
                    sorted.push_back(prefix_info(section, -price * s, s, affix));
            }
#endif                
        }
    }

    // Sort all by price in ascending order.
    sorted.sort(sort_by_price);
    for (auto &p: sorted)
        prefixes.push_back(p);

    // Exclude affixes that conflict with required ones.
    for (auto const &reqName: required) {
        int r = getPrefixIndex(reqName);
        
        for (auto const &c: prefixes[r].entry["conflicts"]) {
            DLString conflictName = c.asString();
            prefixes.erase(
                remove_if(prefixes.begin(), prefixes.end(),
                    [&conflictName](const prefix_info &pi) { return pi.affixName == conflictName; }),
                prefixes.end()
            );
        }
    }

    // TODO: exclude affixes based on align bonuses, preferences and probabilities.

    for (auto &p: prefixes)
        notice("...affix %s [%d]", p.affixName.c_str(), p.price);
}

bool prefix_generator::checkRequirementConflict(const Json::Value &affix) const
{
    for (auto const &conflictName: affix["conflicts"])
        if (required.count(conflictName.asString()) > 0)
            return true;

    return false;
}

bool prefix_generator::checkTierThreshold(const Json::Value &affix) const
{
    int threshold = affix.isMember("tier") ? affix["tier"].asInt() : WORST_TIER;
    return tier.num <= threshold;
}

bool prefix_generator::checkForbidden(const Json::Value &affix) const
{
    return forbidden.count(affix["value"].asString()) > 0;
}

bool prefix_generator::checkAlign(const Json::Value &affix) const
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
void prefix_generator::markRequirements()
{
    for (auto const &reqName: required) {
        int r = getPrefixIndex(reqName);
        requirements.set(r);
    }
}

/** Creates a NxN matrix of prefix flags, marking those that are mutually exclusive.
 */
void prefix_generator::markExclusions() 
{
    exclusions.resize(prefixes.size());

    for (unsigned int i = 0; i < prefixes.size(); i++) {            
        bucket_mask_t &exclusion = exclusions[i];
        prefix_info &p = prefixes[i];

        if (p.section == "material")
            for (unsigned int j = 0; j < prefixes.size(); j++)
                if (i != j && prefixes[j].section == p.section)
                    exclusion.set(j);

        for (auto &conflictName: p.entry["conflicts"]) {
            int index = getPrefixIndex(conflictName.asString());
            if (index >= 0) {
                exclusion.set(index);
                exclusions[index].set(i);
            }
        }
    }

#if 0
    // Mark all stacked values such as +hr, -hr as mutually exclusive.
    for (unsigned int i = 0; i < prefixes.size(); i++) {
        const prefix_info &p = prefixes[i];
        for (auto &same: getPrefixIndexes(p.entry["value"].asString()))
            if (i != same)
                exclusions[i].set(same);
    }
#endif         
}



// Debug util: grab several good prefix combinations for the tier. 
list<list<string>> random_weapon_affixes(int tier, int count, int align)
{
    list<list<string>> allNames;
    prefix_generator gen(tier);
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

