#ifndef WEAPON_AFFIXES_H
#define WEAPON_AFFIXES_H

#include <string>
#include <bitset>
#include <set>
#include <list>
#include <unordered_set>
#include <vector>
#include <jsoncpp/json/json.h>

using namespace std;
class DLString;
class weapon_tier_t;

/** Helper structure to access prefix configuration. */
struct prefix_info {
    prefix_info(const string &section, const string &affixName, int price, int stack, Json::Value entry);

    string section;
    string affixName;
    int price;
    int stack;
    Json::Value entry;
};


/** A mask that can hold boolean information about 100 affixes. */
typedef bitset<100> bucket_mask_t;

/** This class can generate all combinations of prefixes that match given tier and its price range. */
struct prefix_generator {

    prefix_generator(int tier);

    /** Remember align restriction. */
    void setAlign(int align);

    /** Mark a certain affix as forbidden in all combinations. */
    void addForbidden(const DLString &name);

    /** Mark a certain affix as required (always chosen). */
    void addRequired(const DLString &name);

    void run();

    /** Produces a single random prefix combination out of all generated ones. */
    list<prefix_info> getSingleResult() const;

    int getResultSize() const;

private:

    int getPrefixIndex(const DLString &name);

    list<int> getPrefixIndexes(const DLString &name);

    /** Choose a random set element. */
    bucket_mask_t randomBucket() const;

    /** Recursively produce masks were 1 marks an included prefix, 0 marks an excluded prefix.
     *  Each mask denotes a combination of prefixes those total price matches prices for the tier. 
     */
    void generateBuckets(int currentTotal, long unsigned int index, bucket_mask_t currentMask);

    /** Creates a vector of all prefixes that are allowed for the tier, sorted by price in ascending order. */
    void collectPrefixesForTier();

    bool checkRequirementConflict(const Json::Value &affix) const;

    bool checkTierThreshold(const Json::Value &affix) const;

    bool checkForbidden(const Json::Value &affix) const;

    bool checkAlign(const Json::Value &affix) const;

    /** Expresses affix names from 'required' field as a bit mask. */
    void markRequirements();

    /** Creates a NxN matrix of prefix flags, marking those that are mutually exclusive.
     */
    void markExclusions();

    /** Keeps info about current tier. */
    weapon_tier_t &tier;

    /** Keeps all avaialble prefixes sorted by price. */
    vector<prefix_info> prefixes;

    /** Keeps all possible combination matching tier's price. If a bit M is set in a bucket mask, then prefix M is included. */
    unordered_set<bucket_mask_t> buckets;

    /** Marks prefixes that need to always be included in the result. */
    bucket_mask_t requirements;

    /** Keeps all exclusions for prefixes. If entry N has a bit M set, then prefixes N and M are mutually exclusive. */
    vector<bucket_mask_t> exclusions;

    int minPrice;
    int maxPrice;
    int align;

    /** Pre-set forbidden affixes. */
    set<DLString> forbidden;

    /** Pre-set required affixes. */
    set<DLString> required;
};

#endif