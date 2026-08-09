#ifndef WEAPON_TIER_H
#define WEAPON_TIER_H

#include <jsoncpp/json/json.h>
#include "configurable.h"
#include "flags.h"

extern const FlagTable extra_flags;
class Object;
struct obj_index_data;

/** Weapon tier: determines how cool is the weapon and all related tier settings. */
struct weapon_tier_t {
    int num;
    DLString rname;
    DLString aura;
    DLString colour;
    json_flag<&extra_flags> extra;
    int min_points;
    int max_points;
    int worst_penalty;
    int weeks;
    int chance;

    void fromJson(const Json::Value &value);
};

extern json_vector<weapon_tier_t> weapon_tier_table;

#define BEST_TIER    1
#define DEFAULT_TIER 3
#define WORST_TIER   5

/** Roll a tier according to each tier's configured chance, but no better than bestTier.
 *  Tiers are tried from the best allowed one downwards, so a tier with chance 0 can only
 *  ever be granted explicitly, never rolled -- which is how the legendary tier works.
 *  Each drop site decides how often it hands one out, in parts per thousand, because
 *  the interesting values are well below one percent.
 */
int random_weapon_tier(int bestTier, int legendaryPerMille = 0);

/** Same roll, but confined to a [bestTier, worstTier] window instead of running all the
 *  way down to WORST_TIER. Falls back to worstTier when no tier in the window rolls in.
 *  The legendary jackpot still ignores the window -- see the implementation.
 *  Deliberately a separate name rather than an overload: random_weapon_tier(3, 20) would
 *  otherwise silently mean two different things depending on the argument count.
 */
int random_weapon_tier_range(int bestTier, int worstTier, int legendaryPerMille);

// Return tier number stored in this item's properties.
int get_item_tier(Object *obj);

// Return tier number for the prototype.
int get_item_tier(obj_index_data *pObj);

// Return a screenreader aura configured for this item's tier.
DLString get_tier_aura(Object *obj);

// Check if this weapon prototype is a random one.
bool item_is_random(obj_index_data *pObj);

#endif