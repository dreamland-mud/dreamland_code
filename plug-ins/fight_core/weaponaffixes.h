#ifndef WEAPON_AFFIXES_H
#define WEAPON_AFFIXES_H

#include <string>
#include <bitset>
#include <set>
#include <list>
#include <unordered_set>
#include <vector>
#include <jsoncpp/json/json.h>

#include "bitstring.h"

using namespace std;
class DLString;
class weapon_tier_t;
class PCharacter;

/** Helper structure to access affix configuration. */
struct affix_info {
    affix_info(const string &section, int stack, const Json::Value &entry);
    virtual ~affix_info();

    /** Two affix infos are equal if they represet the same name and price. */
    bool equals(const affix_info &other) const;

    /** Return affix name without initial '-' or '+'; */
    string normalizedName() const;

    string section;
    string affixName;
    int price;
    int stack;
    set<string> conflicts;
    bitnumber_t alignBonus;
    Json::Value affix;
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

    /** Mark a certain affix as preferred (always included in initial set). */
    void addPreference(const DLString &name);

    /** Decide probability an affix that is not preferred gets removed from initial set. */
    void setRetainChance(int chance) { this->retainChance = chance; }

    /** Will generate player-specific affixes for this player. */
    void setPlayer(PCharacter *pch) { this->pch = pch; }

    void setup();
    void run();

    /** Produces a single random affix combination out of all generated ones. */
    list<affix_info> getSingleResult() const;

    int getResultSize() const;

    const vector<affix_info> & getAffixes() const { return affixes; }

    string dump() const;
    
private:

    list<affix_info *> getAffixes(const DLString &name);

    int getAffixIndex(const DLString &name);

    /** Recursively walk every affix combination whose total price matches the tier,
     *  reservoir-sampling ONE of them uniformly. 1 marks an included affix, 0 excluded.
     */
    void generateBuckets(int currentTotal, int currentPenalty, long unsigned int index, bucket_mask_t currentMask);

    /** Creates a vector of all affixes that are allowed for the tier, sorted by price in ascending order. */
    void collectAffixesForTier();

    bool checkPlayer(const Json::Value &one_section) const;
    
    bool checkMutualConflict(const affix_info &a1, const affix_info &a2);

    bool checkTierThreshold(const Json::Value &affix) const;

    bool checkForbidden(const Json::Value &affix) const;

    bool checkAlign(const Json::Value &affix) const;

    bool checkAlignBonus(const affix_info &ai) const;

    /** Expresses affix names from 'required' field as a bit mask. */
    void markRequirements();

    /** Creates a NxN matrix of affix flags, marking those that are mutually exclusive.
     */
    void markExclusions();

    /** Keeps info about current tier. */
    weapon_tier_t &tier;

    /** Keeps all avaialble affixes sorted by price. */
    vector<affix_info> affixes;

    /** Reservoir sampler over the valid affix combinations. Storing every matching
     *  subset made tier 1 enumerate tens of millions of masks -- gigabytes of memory
     *  and a multi-second freeze of the whole (single-threaded) server. Instead
     *  generateBuckets keeps ONE uniformly-random valid combination on the fly:
     *  chosenBucket is that pick, bucketCount how many valid combinations were seen (a
     *  bit M set means affix M is in), visitCount a hard cap on the walk. */
    bucket_mask_t chosenBucket;
    unsigned long bucketCount;
    unsigned long visitCount;

    /** Marks affixes that need to always be included in the result. */
    bucket_mask_t requirements;

    /** Keeps all exclusions for affixes. If entry N has a bit M set, then affixes N and M are mutually exclusive. */
    vector<bucket_mask_t> exclusions;

    int minPrice;
    int maxPrice;
    int worstPenalty;
    int align;
    PCharacter *pch;

    int retainChance;

    /** Pre-set forbidden affixes. */
    set<DLString> forbidden;

    /** Pre-set required affixes. */
    set<DLString> required;

    /** Pre-set preferences. */
    set<DLString> preferences;
};

/** Whole affix config, as loaded from fight/weapon_affixes.json: one member per
 *  section, each holding a "values" array. Defined in weaponaffixes.cpp. */
extern Json::Value weapon_affixes;

#endif
