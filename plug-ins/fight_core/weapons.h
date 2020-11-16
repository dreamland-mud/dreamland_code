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
Skill *  get_weapon_skill( Object *wield );

/** Return average damage for a weapon. */
int weapon_ave(Object *wield);

/** Return average damage for a weapon prototype. */
int weapon_ave(struct obj_index_data *pWield);

/**
 * Weapon generator: calculate best value1 for a weapon of given class, level and v2.
 */
int weapon_value1(int level, int tier, int value2, bitnumber_t wclass);

/**
 * Weapon generator: return fixed value2 based on weapon class.
 */
int weapon_value2(bitnumber_t wclass);

/**
 * Weapon generator: return fixed average damage for a level and a tier.
 */
int weapon_ave(int level, int tier, bitnumber_t wclass);

/**
 * Weapon generator: return fixed damroll bonus for a level and a tier.
 */
int weapon_damroll(int level, int tier, bitnumber_t wclass);

struct WeaponGenerator {
    WeaponGenerator & item(Object *obj);
    WeaponGenerator & assignValues(int tier);
    WeaponGenerator & assignHitroll(int tier, int sn = -1);
    WeaponGenerator & assignDamroll(int tier, int sn = -1);

private:
    Object *obj;    
};

#endif
