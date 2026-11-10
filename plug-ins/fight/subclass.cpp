/*
 * Warrior subclass records (fight/subclasses.json), see subclass.h.
 */
#include "subclass.h"

#include "character.h"
#include "pcharacter.h"
#include "core/object.h"
#include "objectfactory.h"
#include "commonattributes.h"
#include "configurable.h"
#include "weapons.h"
#include "attacks.h"
#include "loadsave.h"
#include "damageflags.h"
#include "merc.h"
#include "def.h"

static Json::Value subclassesConfig;
CONFIGURABLE_LOADED(fight, subclasses)
{
    subclassesConfig = value;
}

const Json::Value & subclass_rules( const DLString &name )
{
    // The const operator[] gives a null value for a missing key instead of
    // adding one to the config.
    const Json::Value &config = subclassesConfig;
    if (name.empty( ) || !config.isObject( ))
        return Json::Value::null;
    return config[name];
}

const Json::Value & subclass_rules( Character *ch )
{
    if (!ch || ch->is_npc( ))
        return Json::Value::null;
    return subclass_rules( get_string_attribute( ch->getPC( ), "subclass" ) );
}

static bool json_lists( const Json::Value &list, const DLString &value )
{
    if (!list.isArray( ))
        return false;
    for (auto &v: list)
        if (v.asString( ) == value)
            return true;
    return false;
}

// The rule itself, on a weapon's class, attack and weapon_type2 flags.
static bool subclass_weapon_rule( const Json::Value &rules, int wclass, int attack, bitstring_t wflags )
{
    DLString cls = weapon_class.name( wclass );
    const Json::Value &classes = rules["weapon_classes"];
    if (classes.isArray( ) && !json_lists( classes, cls ))
        return false;
    if (json_lists( rules["banned_classes"], cls ))
        return false;

    const Json::Value &damTypes = rules["damage_types"];
    if (damTypes.isArray( )) {
        int dam = attack_damage( attack );
        if (dam == -1)
            dam = DAM_BASH;
        if (!json_lists( damTypes, damage_table.name( dam ) ))
            return false;
    }

    const Json::Value &banned = rules["banned_flags"];
    if (banned.isArray( ))
        for (auto &f: banned) {
            bitstring_t bits = weapon_type2.bitstring( f.asString( ), true );
            if (bits != (bitstring_t)NO_FLAG && IS_SET( wflags, bits ))
                return false;
        }

    if (rules["one_handed"].asBool( ) && IS_SET( wflags, WEAPON_TWO_HANDS ))
        return false;

    return true;
}

bool subclass_weapon_fits( const Json::Value &rules, Object *obj )
{
    if (!obj || obj->item_type != ITEM_WEAPON)
        return false;
    return subclass_weapon_rule( rules, get_weapon_class( obj ), get_weapon_attack( obj ), obj->value4( ) );
}

bool subclass_weapon_fits( const Json::Value &rules, obj_index_data *pObj )
{
    if (!pObj || pObj->item_type != ITEM_WEAPON)
        return false;
    return subclass_weapon_rule( rules, pObj->value[0], pObj->value[3], pObj->value[4] );
}

bool subclass_one_hand( const Json::Value &rules )
{
    return rules["single"].asBool( ) || rules["shield"].asBool( );
}
