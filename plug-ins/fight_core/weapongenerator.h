#ifndef WEAPON_GENERATOR_H
#define WEAPON_GENERATOR_H

#include <vector>
#include <list>
#include <set>
#include <jsoncpp/json/json.h>
#include "flags.h"
#include "affect.h"
#include "plugin.h"

class Object;
class PCharacter;
struct affix_info;

/** Weapon generator: calculate and assign various weapon parameters based on requested input data. */
struct WeaponGenerator {
    WeaponGenerator(bool debug = false);
    virtual ~WeaponGenerator();

    WeaponGenerator & item(Object *obj);
    WeaponGenerator & tier(int tier);
    WeaponGenerator & player(PCharacter *pch) { this->pch = pch; return *this; }
    WeaponGenerator & skill(int sn) { this->sn = sn; return *this; }
    WeaponGenerator & valueTier(int tier) { this->valTier = tier; return *this; }
    WeaponGenerator & hitrollTier(int tier) { this->hrTier = tier; return *this; }
    WeaponGenerator & damrollTier(int tier) { this->drTier = tier; return *this; }
    WeaponGenerator & hitrollStartPenalty(float coef) { this->hrCoef = coef; return *this; }
    WeaponGenerator & damrollStartPenalty(float coef) { this->drCoef = coef; return *this; }
    WeaponGenerator & hitrollMinStartValue(int minValue) { this->hrMinValue = minValue; return *this; }
    WeaponGenerator & damrollMinStartValue(int minValue)  { this->drMinValue = minValue; return *this; }
    WeaponGenerator & hitrollIndexBonus(float bonus) { this->hrIndexBonus = bonus; return *this; }
    WeaponGenerator & damrollIndexBonus(float bonus) { this->drIndexBonus = bonus; return *this; }
    WeaponGenerator & valueIndexBonus(float bonus) { this->aveIndexBonus = bonus; return *this; }
    WeaponGenerator & alignment(int align) { this->align = align; return *this; }
    WeaponGenerator & setRetainChance(int retainChance) { this->retainChance = retainChance; return *this; }
    WeaponGenerator & randomTier(int bestTier);
    WeaponGenerator & addRequirement(const DLString &req) { this->required.insert(req); return *this; }
    WeaponGenerator & addForbidden(const DLString &fbd) { this->forbidden.insert(fbd); return *this; }
    WeaponGenerator & randomWeaponClass();
    WeaponGenerator & randomNames();
    WeaponGenerator & randomAffixes();

    const WeaponGenerator & assignValues() const;
    
    const WeaponGenerator & assignHitroll() const;
    const WeaponGenerator & assignDamroll() const;
    
    const WeaponGenerator & assignStartingHitroll() const;
    const WeaponGenerator & assignStartingDamroll() const;
    
    const WeaponGenerator & incrementHitroll() const;
    const WeaponGenerator & incrementDamroll() const;

    const WeaponGenerator & assignNames() const;
    const WeaponGenerator & assignColours() const;
    const WeaponGenerator & assignFlags() const;
    const WeaponGenerator & assignDamageType() const;
    const WeaponGenerator & assignAffects() const;
    const WeaponGenerator & assignTimers() const;

private:
    void setAffect(int location, int modifier) const;
    void setName() const;
    void setShortDescr() const;
    DLString findMaterial() const;
    void rememberAffect(Affect &af);
    int calcAffectModifier(const Json::Value &afConfig, const affix_info &info) const;
    int maxDamroll() const;
    int maxHitroll() const;
    int minDamroll() const;
    int minHitroll() const;

    Json::Value nameConfig;
    Json::Value wclassConfig;
    Flags extraFlags;
    Flags weaponFlags;
    DLString materialName;
    list<Affect> affects;
    vector<DLString> adjectives;
    vector<DLString> nouns;

    PCharacter *pch;
    int sn;
    int valTier;
    int hrTier;
    int drTier;
    float hrCoef;
    float drCoef;
    int hrMinValue;
    int drMinValue;
    float hrIndexBonus;
    float drIndexBonus;
    float aveIndexBonus;
    int align;

    // Additional requirements set by test suite.
    set<DLString> required; 

    // Forbidden affixes configured during area reset.
    set<DLString> forbidden;

    // A chance for random affix to remain in the initial set.
    int retainChance; 

    Object *obj;    
    DLString wclass;

    bool debug;
};


#endif