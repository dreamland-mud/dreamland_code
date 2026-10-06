#include <jsoncpp/json/json.h>

#include "weaponaffixes.h"
#include "configurable.h"

/** Kept for the weapon name forms (adjectives/nouns _en/_ua) the name repair reads;
 *  the affixes themselves roll from item_affixes.json. */
Json::Value weapon_affixes;
CONFIGURABLE_LOADED(fight, weapon_affixes)
{
    weapon_affixes = value;
}
