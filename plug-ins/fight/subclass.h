/*
 * Warrior subclasses (fencer, berserker, sentinel): the fight/subclasses.json
 * records the engine reads. Fenia reads the whole file (dreamland_fenia
 * "utils/subclass"); the skill gate itself is the <subclasses> nodes of each
 * skill (GenericSkill).
 */
#ifndef __FIGHT_SUBCLASS_H__
#define __FIGHT_SUBCLASS_H__

#include <jsoncpp/json/json.h>

class DLString;
class Character;
class Object;
struct obj_index_data;

/** The record of subclass name, a null value when there is none. */
const Json::Value & subclass_rules( const DLString &name );

/** The record of ch's subclass (PC string attribute "subclass"), a null
 *  value for NPCs, for no subclass and for an unknown one. */
const Json::Value & subclass_rules( Character *ch );

/**
 * One weapon against the weapon rules of a subclass record, the one copy of
 * the rule (Fenia .tmp.subclass.weaponFits calls it through root
 * .subclassWeaponFits): its class is in weapon_classes (when there
 * is such a list) and not in banned_classes, its damage type is in
 * damage_types (when there is such a list), it carries none of banned_flags,
 * and it is not two-handed when one_handed is set. False for a non-weapon.
 * An instance is judged by its affected class, attack and flags, a
 * prototype by its values.
 */
bool subclass_weapon_fits( const Json::Value &rules, Object *obj );
bool subclass_weapon_fits( const Json::Value &rules, obj_index_data *pObj );

/** Does the record keep the off hand free of a weapon: single (one weapon)
 *  or shield (a shield on that arm)? */
bool subclass_one_hand( const Json::Value &rules );

/**
 * Does the record bar worn item obj from the hands? A weapon that fails
 * subclass_weapon_fits, a shield with no_shield, and with offHand set any
 * weapon in the off hand of a subclass_one_hand record. False for a null
 * obj or a null record (no subclass).
 */
bool subclass_bars_hand_item( const Json::Value &rules, Object *obj, bool offHand );

#endif
