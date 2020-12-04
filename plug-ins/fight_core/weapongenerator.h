#ifndef WEAPON_GENERATOR_H
#define WEAPON_GENERATOR_H

#include <vector>
#include <jsoncpp/json/json.h>
#include "flags.h"

class Object;

/** Weapon generator: calculate and assign various weapon parameters based on requested input data. */
struct WeaponGenerator {
    WeaponGenerator();
    virtual ~WeaponGenerator();

    WeaponGenerator & item(Object *obj);
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
    WeaponGenerator & alignment(int align) { this->align = align; return *this; }

    WeaponGenerator &  randomNames();
    WeaponGenerator &  randomAffixes();

    const WeaponGenerator & assignValues() const;
    
    const WeaponGenerator & assignHitroll() const;
    const WeaponGenerator & assignDamroll() const;
    
    const WeaponGenerator & assignStartingHitroll() const;
    const WeaponGenerator & assignStartingDamroll() const;
    
    const WeaponGenerator & incrementHitroll() const;
    const WeaponGenerator & incrementDamroll() const;

    const WeaponGenerator & assignNames() const;
    const WeaponGenerator & assignFlags() const;
    const WeaponGenerator & assignDamageType() const;
    const WeaponGenerator & assignAffects() const;

private:
    void setAffect(int location, int modifier) const;
    void setName() const;
    void setShortDescr() const;
    DLString findMaterial() const;
    int maxDamroll() const;
    int maxHitroll() const;
    int minDamroll() const;
    int minHitroll() const;

    Json::Value nameConfig;
    Json::Value wclassConfig;
    Flags extraFlags;
    Flags weaponFlags;
    DLString materialName;
    vector<DLString> adjectives;
    vector<DLString> nouns;
    int align;

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
    DLString wclass;
};


#endif