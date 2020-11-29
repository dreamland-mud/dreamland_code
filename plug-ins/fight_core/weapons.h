/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEAPONS_H
#define WEAPONS_H

#include <vector>
#include "jsoncpp/json/json.h"
#include "configurable.h"

class Character;
class Object;
class Skill;
struct obj_index_data;
extern const FlagTable extra_flags;

Object * get_wield( Character *ch, bool secondary );
int         get_weapon_sn( Character *ch, bool secondary );
int          get_weapon_sn( Object *wield );
Skill *  get_weapon_skill( Object *wield );

/** Return average damage for a weapon. */
int weapon_ave(Object *wield);

/** Return average damage for a weapon prototype. */
int weapon_ave(struct obj_index_data *pWield);


/** Weapon tier: determines how cool is the weapon and all related tier settings. */
struct weapon_tier_t {
    int num;
    DLString rname;
    DLString aura;
    DLString colour;
    json_flag<&extra_flags> extra;
    int min_points;
    int max_points;

    void fromJson(const Json::Value &value);
};

extern json_vector<weapon_tier_t> weapon_tier_table;

#define BEST_TIER 1
#define WORST_TIER 5

/** Weapon parameter calculator: figure out v1, v2 and damroll for given tier, level and weapon class. */
struct WeaponCalculator {
    WeaponCalculator(int tier, int level, bitnumber_t wclass, int index_bonus = 0);

    int getValue1() const { return value1; }
    int getValue2() const { return value2; }
    int getDamroll() const { return damroll; }
    int getAve() const { return ave; }
    int getRealAve() const { return real_ave; }

private:
    void calcValue2Range();
    void calcAve();
    void calcValues();
    void calcDamroll();
    int getTierIndex() const;

    int tier;
    int level;
    bitnumber_t wclass;
    int v2_min;
    int v2_max;
    int value1;
    int value2;
    int ave;
    int real_ave;
    int damroll;
    int index_bonus;
};

/** Weapon generator: calculate and assign various weapon parameters based on requested input data. */
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
    WeaponGenerator & hitrollIndexBonus(int bonus) { this->hrIndexBonus = bonus; return *this; }
    WeaponGenerator & damrollIndexBonus(int bonus) { this->drIndexBonus = bonus; return *this; }
    
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
    int hrIndexBonus;
    int drIndexBonus;

    Object *obj;    
};

#endif
