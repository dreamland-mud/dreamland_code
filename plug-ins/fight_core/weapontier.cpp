#include "weapontier.h"
#include "core/object.h"
#include "behavior.h"
#include "merc.h"
#include "def.h"

BHV(random_weapon);

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
    max_points = value["max_points"].asInt();
    worst_penalty = value.isMember("max_penalty") ? value["max_penalty"].asInt() : -1000;
    weeks = value["weeks"].asInt();
    chance = value["chance"].asInt();
}

json_vector<weapon_tier_t> weapon_tier_table;
CONFIGURABLE_LOADED(fight, weapon_tiers)
{
    weapon_tier_table.fromJson(value);
}

static int valid_tier(const DLString &tierName)
{
    if (!tierName.isNumber())
        return 0;

    int tier = tierName.toInt();        
    if (tier < BEST_TIER || tier > WORST_TIER)
        return 0;

    return tier;
}

int get_item_tier(Object *obj)
{
    DLString tierName = obj->getProperty("tier");
    return valid_tier(tierName);
}

int get_item_tier(OBJ_INDEX_DATA *pObj)
{
    DLString tierName = pObj->getProperty("bestTier");

    if (tierName.empty())
        return 0;

    return valid_tier(tierName);
}

DLString get_tier_aura(Object *obj)
{
    int tier = get_item_tier(obj);
    if (tier > 0) {
        weapon_tier_t &one_tier = weapon_tier_table[tier - 1];
        return one_tier.aura;
    }

    return DLString::emptyString;
}

bool item_is_random(obj_index_data *pObj) 
{
    return pObj->behaviors.isSet(bhv_random_weapon);
}

