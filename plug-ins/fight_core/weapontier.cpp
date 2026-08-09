#include "weapontier.h"
#include "core/object.h"
#include "behavior.h"
#include "dl_math.h"
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

int random_weapon_tier_range(int bestTier, int worstTier, int legendaryPerMille)
{
    int minTier = URANGE(BEST_TIER, bestTier, WORST_TIER);
    int maxTier = URANGE(minTier, worstTier, WORST_TIER);

    // The legendary jackpot deliberately ignores the requested window: it is the
    // one way a drop can beat the tier its site asked for.
    if (legendaryPerMille > 0 && number_range(1, 1000) <= legendaryPerMille)
        return BEST_TIER;

    for (int i = minTier - 1; i < maxTier; i++)
        if (chance(weapon_tier_table[i].chance))
            return i + 1;

    return maxTier;
}

int random_weapon_tier(int bestTier, int legendaryPerMille)
{
    return random_weapon_tier_range(bestTier, WORST_TIER, legendaryPerMille);
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

