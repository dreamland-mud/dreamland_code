#include "weapontier.h"

/*-----------------------------------------------------------------------------
 * Weapon tiers
 *-----------------------------------------------------------------------------*/
void weapon_tier_t::fromJson(const Json::Value &value)
{
    num = value["num"].asInt();
    rname = value["rname"].asString();
    aura = value["aura"].asString();
    colour = value["colour"].asString();
    extra.fromJson(value["extra"]);
    min_points = value["min_points"].asInt();
    max_points = value["min_points"].asInt();
}

json_vector<weapon_tier_t> weapon_tier_table;
CONFIGURABLE_LOADED(fight, weapon_tiers)
{
    weapon_tier_table.fromJson(value);
}


