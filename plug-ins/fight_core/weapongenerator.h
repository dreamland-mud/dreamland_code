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
    WeaponGenerator();
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
    WeaponGenerator & randomTier(int bestTier, int legendaryPerMille = 0);
    WeaponGenerator & addRequirement(const DLString &req) { this->required.insert(req); return *this; }
    WeaponGenerator & addForbidden(const DLString &fbd) { this->forbidden.insert(fbd); return *this; }
    /** Price affixes for a caster killer (item_affixes.json price_caster). */
    WeaponGenerator & caster(bool caster) { this->isCaster = caster; return *this; }
    /** Affix budget: 1 = M (item_affixes.json), 0 = old points (weapon_affixes.json),
     *  -1 = whatever item_affixes.json _weapons.use_m says. */
    WeaponGenerator & budgetMode(int mode) { this->mMode = mode; return *this; }

    // Main method to handle rand_stat logic, after all parameters have been set up by the calls above.
    WeaponGenerator& randomizeStats();

    // Main method to handle rand_all logic, after all parameters have been set up by the calls above.
    WeaponGenerator& randomizeAll();

    WeaponGenerator & randomWeaponClass();

    /** Pin the weapon class instead of rolling one. randomizeAll() then leaves the class
     *  alone. Unknown names are ignored with a warning -- validate with
     *  weapon_class_exists() at the caller's boundary if a hard error is wanted.
     *  Call after item(); an empty name is a no-op.
     *  Do NOT combine with randomizeStats(): that path regenerates neither names nor
     *  damage type, so the weapon would end up wearing its old class's identity.
     */
    WeaponGenerator & weaponClass(const DLString &name);

    WeaponGenerator & randomNames();
    WeaponGenerator & randomAffixes();
    /** randomAffixes() on the M budget, shared pool with random armor. */
    WeaponGenerator & randomAffixesM();

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
    void applyWeaponClass(const DLString &name);
    void setAffect(int location, int modifier) const;
    void setName() const;
    void setShortDescr() const;
    bool rejectsMetal() const;
    DLString findMaterial() const;
    DLString nonMetalDefault() const;
    void rememberAffect(Affect &af);
    bool useM() const;
    void decideTwoHands();
    bool nameFitsHands(const Json::Value &config) const;
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
    // Parallel per-language affix forms (same index/subset as adjectives/nouns);
    // EN = plain, UA = nominative (declined at gen-time via Morphology::declineUa).
    vector<DLString> adjectives_en, adjectives_ua;
    vector<DLString> nouns_en, nouns_ua;

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
    bool isCaster;
    int mMode;

    // Two-hander share on the M budget: base ave and damroll are scaled by it.
    bool twoHands;
    bool twoHandsDecided;
    float aveMult;
    float damrollMult;

    // Additional requirements set by test suite.
    set<DLString> required; 

    // Forbidden affixes configured during area reset.
    set<DLString> forbidden;

    // A chance for random affix to remain in the initial set.
    int retainChance; 

    Object *obj;
    DLString wclass;

    // Set by weaponClass(): tells randomizeAll() not to roll a class of its own.
    bool wclassFixed;
};

/** Backfill the English and Ukrainian names of a weapon generated before the
 *  generator learned all three languages, and drop Russian text an older binary
 *  pinned into those slots on a re-statted one. Recovers which affix adjective
 *  and noun the Russian name was built from, so the result is what the generator
 *  itself would have written. Never touches Russian, never invents a translation:
 *  a part it cannot decode is logged and the slot left for a later read.
 *  Returns true when anything changed. Safe to call on any object -- everything
 *  without a generated name returns immediately. */
bool weapon_repair_names(Object *obj);

/* Name helpers shared with the armor generator (armorgenerator.cpp), so random
 * weapons and random armor glue and decline their names the same way. */
DLString random_item_compose_short(const DLString &adjective, const DLString &base, const DLString &noun);
DLString random_item_gender_tag(const DLString &gender);
bool random_item_decline_ua(const DLString &word, const DLString &pos, const DLString &gtag, DLString &result);
int random_item_skillgroup(PCharacter *pch);

/** True when this weapon class name is present in the weapon_classes config. */
bool weapon_class_exists(const DLString &name);

/** Weapon class the player has trained highest among those available to them.
 *  Empty when there is no player or nothing qualifies -- which is the same
 *  "roll one yourself" sentinel that weaponClass() already treats as a no-op.
 */
DLString best_weapon_class(PCharacter *pch);


#endif