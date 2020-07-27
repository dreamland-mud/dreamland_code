/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEAPONS_H
#define WEAPONS_H

class Character;
class Object;
class Skill;

Object * get_wield( Character *ch, bool secondary );
int         get_weapon_sn( Character *ch, bool secondary );
int          get_weapon_sn( Object *wield );
Skill *  get_weapon_skill( Object *wield );

/**
 * Weapon generator: calculate best value1 for a weapon of given class, level and tier.
 */
int weapon_value1(int level, int tier, bitnumber_t wclass);

/**
 * Weapon generator: return fixed value2 based on weapon class.
 */
int weapon_value2(bitnumber_t wclass);

/**
 * Weapon generator: return fixed average damage for a level and a tier.
 */
int weapon_ave(int level, int tier);

/**
 * Weapon generator: return fixed damroll bonus for a level and a tier.
 */
int weapon_damroll(int level, int tier);

#endif
