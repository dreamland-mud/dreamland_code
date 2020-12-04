#include <jsoncpp/json/json.h>
#include <math.h>

#include "weaponcalculator.h"
#include "logstream.h"
#include "configurable.h"
#include "math_utils.h"
#include "merc.h"
#include "def.h"

Json::Value weapon_level_penalty;
CONFIGURABLE_LOADED(fight, weapon_level_penalty)
{
    weapon_level_penalty = value;
}

Json::Value weapon_value2_by_class;
CONFIGURABLE_LOADED(fight, weapon_value2)
{
    weapon_value2_by_class = value;
}

Json::Value weapon_ave_penalty;
CONFIGURABLE_LOADED(fight, weapon_ave_penalty)
{
    weapon_ave_penalty = value;
}

Json::Value weapon_ave_tiers;
CONFIGURABLE_LOADED(fight, weapon_ave_tiers)
{
    weapon_ave_tiers = value;
}

Json::Value weapon_damroll_tiers;
CONFIGURABLE_LOADED(fight, weapon_damroll_tiers)
{
    weapon_damroll_tiers = value;
}

/** Helper struct to keep interim result of value1/value2 calculations. 
 *  A list of these structures will be sorted to find the one closest to requested ave.
 */
struct weapon_value_t {
    weapon_value_t(int v1, int v2, int ave) {
        this->v1 = v1;
        this->v2 = v2;
        this->ave = ave;
        real_ave = dice_ave(v1, v2);
    }

    int distance() const { 
        return abs(real_ave - ave); 
    }

    int v1;
    int v2;
    int real_ave;
    int ave;
};

static bool sort_by_ave_distance(const weapon_value_t &w1, const weapon_value_t &w2)
{
    return w1.distance() <= w2.distance();
}



/*--------------------------------------------------------------------------
 * WeaponCalculator
 *-------------------------------------------------------------------------*/
WeaponCalculator:: WeaponCalculator(int tier, int level, bitnumber_t wclass, int index_bonus) 
    : tier(tier), level(level), wclass(wclass), index_bonus(index_bonus)
{
    calcValue2Range();
    calcAve();
    calcValues();
    calcDamroll();
}

int WeaponCalculator::getTierIndex() const
{
    int index = level / 5;
    int penalty = weapon_level_penalty[wclass].asInt();
    index = max(0, index + penalty + index_bonus);
    return index;
}

/** Retrieve min and max value2 for a given weapon class. */
void WeaponCalculator::calcValue2Range()
{
    if (wclass < 0 || wclass >= (int)weapon_value2_by_class.size()) {
        bug("weapon_value2: invalid weapon class %d", wclass);
        return;
    }

    Json::Value &entry = weapon_value2_by_class[wclass];
    if (entry.isInt()) {
        v2_min = v2_max = entry.asInt();
        return;
    }

    if (entry.isArray() && entry.size() == 2) {
        v2_min = entry[0].asInt();
        v2_max = entry[1].asInt();
        return;
    }

    bug("weapon_value2: invalid values provided for class %d", wclass);
}

/** Retrieve desired ave damage for this tier and weapon class. */
void WeaponCalculator::calcAve() 
{
    if (tier <= 0 || tier > (int)weapon_ave_tiers.size()) {
        bug("weapon_ave: invalid tier %d for level %d", tier, level);
        return;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_ave: invalid level %d for tier %d", level, tier);
        return;
    }

    if (wclass < 0 || wclass >= (int)weapon_ave_penalty.size()) {
        bug("weapon_ave: invalid weapon class %d for penalty table", wclass);
        return;
    }

    Json::Value &one_tier = weapon_ave_tiers[tier-1];
    int index = getTierIndex();
    if (index >= (int)one_tier.size()) {
        bug("weapon_ave: tier %d of size %d doesn't have enough values for level %d", tier, one_tier.size(), level);
        return;
    }

    float multiplier = weapon_ave_penalty[wclass].asFloat();
    int tier_ave = one_tier[index].asInt();
    ave = (int)(multiplier * tier_ave);
}

/** Calculate value1 and resulting value2 (between min and max) for the requested ave damage. */
void WeaponCalculator::calcValues() 
{
    if (ave <= 0)
        return;

    // Calculate all possible v1 and v2, and their respective real average damage.
    list<weapon_value_t> weapon_value_candidates;
    for (int v2 = v2_min; v2 <= v2_max; v2++) {
        float value1_float = 2 * ave / (v2 + 1);
        int value1 = ceil(value1_float);

        weapon_value_candidates.push_back(weapon_value_t(value1, v2, ave));
        weapon_value_candidates.push_back(weapon_value_t(value1-1, v2, ave));
        weapon_value_candidates.push_back(weapon_value_t(value1+1, v2, ave));
    }

    // Find the best v1/v2 combo that gives us a dice closest to the ave from json tables.
    weapon_value_candidates.sort(sort_by_ave_distance);
    const weapon_value_t &winner = weapon_value_candidates.front();    
    value2 = winner.v2;
    value1 = winner.v1;
    real_ave = winner.real_ave;
}

/** Retrieve damroll for this tier and level. */
void WeaponCalculator::calcDamroll() 
{
    if (tier <= 0 || tier > (int)weapon_damroll_tiers.size()) {
        bug("weapon_damroll: invalid tier %d for level %d", tier, level);
        return;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_damroll: invalid level %d for tier %d", level, tier);
        return;
    }

    Json::Value &one_tier = weapon_damroll_tiers[tier-1];
    int index = getTierIndex();

    if (index >= (int)one_tier.size()) {
        bug("weapon_damroll: tier %d of size %d doesn't have enough values for level %d", tier, one_tier.size(), level);
        return;
    }

    damroll = one_tier[index].asInt();
}

