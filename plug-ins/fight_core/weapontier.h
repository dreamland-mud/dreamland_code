#ifndef WEAPON_TIER_H
#define WEAPON_TIER_H

#include <jsoncpp/json/json.h>
#include "configurable.h"
#include "flags.h"

extern const FlagTable extra_flags;

/** Weapon tier: determines how cool is the weapon and all related tier settings. */
struct weapon_tier_t {
    int num;
    DLString rname;
    DLString aura;
    DLString colour;
    json_flag<&extra_flags> extra;
    int min_points;
    int max_points;

    void fromJson(const Json::Value &value);
};

extern json_vector<weapon_tier_t> weapon_tier_table;

#define BEST_TIER 1
#define WORST_TIER 5

#endif