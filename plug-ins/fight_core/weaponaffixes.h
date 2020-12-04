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

/** Helper structure to access affix configuration. */
struct affix_info {
    affix_info(const string &section, const string &affixName, int price, int stack, const Json::Value &entry);
    virtual ~affix_info();

    string section;
    string affixName;
    int price;
    int stack;
    Json::Value entry;
};


/** A mask that can hold boolean information about 100 affixes. */
typedef bitset<100> bucket_mask_t;

/** This class can generate all combinations of affixes that match given tier and its price range. */
struct affix_generator {

    affix_generator(int tier);

    /** Remember align restriction. */
    void setAlign(int align);

    /** Mark a certain affix as forbidden in all combinations. */
    void addForbidden(const DLString &name);

    /** Mark a certain affix as required (always chosen). */
    void addRequired(const DLString &name);

    void setup();
    void run();

    /** Produces a single random affix combination out of all generated ones. */
    list<affix_info> getSingleResult() const;

    int getResultSize() const;

    const vector<affix_info> & getAffixes() const { return affixes; }

private:

    int getAffixIndex(const DLString &name);

    list<int> getAffixIndexes(const DLString &name);

    /** Choose a random set element. */
    bucket_mask_t randomBucket() const;

    /** Recursively produce masks were 1 marks an included affix, 0 marks an excluded affix.
     *  Each mask denotes a combination of affixes those total price matches prices for the tier. 
     */
    void generateBuckets(int currentTotal, long unsigned int index, bucket_mask_t currentMask);

    /** Creates a vector of all affixes that are allowed for the tier, sorted by price in ascending order. */
    void collectAffixesForTier();

    bool checkRequirementConflict(const Json::Value &affix) const;

    bool checkTierThreshold(const Json::Value &affix) const;

    bool checkForbidden(const Json::Value &affix) const;

    bool checkAlign(const Json::Value &affix) const;

    /** Expresses affix names from 'required' field as a bit mask. */
    void markRequirements();

    /** Creates a NxN matrix of affix flags, marking those that are mutually exclusive.
     */
    void markExclusions();

    /** Keeps info about current tier. */
    weapon_tier_t &tier;

    /** Keeps all avaialble affixes sorted by price. */
    vector<affix_info> affixes;

    /** Keeps all possible combination matching tier's price. If a bit M is set in a bucket mask, then affix M is included. */
    unordered_set<bucket_mask_t> buckets;

    /** Marks affixes that need to always be included in the result. */
    bucket_mask_t requirements;

    /** Keeps all exclusions for affixes. If entry N has a bit M set, then affixes N and M are mutually exclusive. */
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
