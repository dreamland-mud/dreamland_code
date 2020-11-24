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
namespace Json { struct Value; }

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
    WeaponGenerator();

    WeaponGenerator & item(Object *obj) { this->obj = obj; return *this; }
    WeaponGenerator & skill(int sn) { this->sn = sn; return *this; }
    WeaponGenerator & valueTier(int tier) { this->valTier = tier; return *this; }
    WeaponGenerator & hitrollTier(int tier) { this->hrTier = tier; return *this; }
    WeaponGenerator & damrollTier(int tier) { this->drTier = tier; return *this; }
    WeaponGenerator & hitrollStartPenalty(float coef) { this->hrCoef = coef; return *this; }
    WeaponGenerator & damrollStartPenalty(float coef) { this->drCoef = coef; return *this; }
    WeaponGenerator & hitrollMinStartValue(int minValue) { this->hrMinValue = minValue; return *this; }
    WeaponGenerator & damrollMinStartValue(int minValue)  { this->drMinValue = minValue; return *this; }
    
    const WeaponGenerator & assignValues() const;
    
    const WeaponGenerator & assignHitroll() const;
    const WeaponGenerator & assignDamroll() const;
    
    const WeaponGenerator & assignStartingHitroll() const;
    const WeaponGenerator & assignStartingDamroll() const;
    
    const WeaponGenerator & incrementHitroll() const;
    const WeaponGenerator & incrementDamroll() const;

    const WeaponGenerator & assignNames() const;
    const WeaponGenerator & assignDamageType() const;

private:
    void setAffect(int location, int modifier) const;
    DLString findMaterial(Json::Value &entry) const;
    int maxDamroll() const;
    int maxHitroll() const;
    int minDamroll() const;
    int minHitroll() const;

    int sn;
    int valTier;
    int hrTier;
    int drTier;
    float hrCoef;
    float drCoef;
    int hrMinValue;
    int drMinValue;

    Object *obj;    
};

#endif
