#ifndef WEAPON_AFFIXES_H
#define WEAPON_AFFIXES_H

#include <jsoncpp/json/json.h>

/** Whole affix config, as loaded from fight/weapon_affixes.json: one member per
 *  section, each holding a "values" array. Defined in weaponaffixes.cpp. */
extern Json::Value weapon_affixes;

#endif
