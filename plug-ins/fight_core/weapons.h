/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEAPONS_H
#define WEAPONS_H

class Character;
class Object;
class Skill;
struct obj_index_data;

Object * get_wield( Character *ch, bool secondary );
int         get_weapon_sn( Character *ch, bool secondary );
int          get_weapon_sn( Object *wield );
int          get_weapon_sn( struct obj_index_data *pWield );
Skill *  get_weapon_skill( Object *wield );

/*
 * A weapon's fighting stats: the values it was built with, plus whatever the
 * affects sitting on it say. An affect overrides the class or the attack type
 * outright (APPLY_WEAPON_CLASS, APPLY_WEAPON_ATTACK) and shifts the dice
 * (APPLY_DICE_NUMBER, APPLY_DICE_SIZE).
 *
 * A non-weapon gets its plain value back, so these are safe to call blind, and
 * so is an override outside its table -- the base value wins instead.
 *
 * THE LINE, because a half-converted tree is worse than either extreme: a
 * reader goes through these when it decides how the weapon FIGHTS or reports
 * that to a player. It reads value0()..value3() directly when it is talking
 * about the physical object.
 *
 * Through here: the hit path and damage type, weapon skill and second-weapon
 * chance, every class-specific combat branch (weaponsmaster, fightmaster,
 * antipaladin, thief, ranger, hunter, battlerager), mob combat AI, thrown
 * weapons, and on the Fenia side identify, compare, missile damage, the death
 * cry and .tmp.object.getWeaponClass.
 *
 * Deliberately NOT through here, all reading the item as an object:
 *   - the weapon generator and weapon calculator -- they build the base values
 *   - OLC, save, ostat, auction -- they show or store what the item IS
 *   - the sheath wearlocation and the arrow quiver -- physical fit
 *   - Fenia utils/craft tool detection, skillcommand/temper, utils/object:
 *     dimensions and utils/weight -- shape, weight and smithing
 * Changing any of those to the affected value is a design decision, not a
 * tidy-up. Say why here if you do.
 */
int get_weapon_class(Object *wield);
int get_weapon_attack(Object *wield);
int get_weapon_dice_number(Object *wield);
int get_weapon_dice_size(Object *wield);

/** Return average damage for a weapon. */
int weapon_ave(Object *wield);

/** Return average damage for a weapon prototype. */
int weapon_ave(struct obj_index_data *pWield);


#endif
