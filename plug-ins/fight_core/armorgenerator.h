#ifndef ARMORGENERATOR_H
#define ARMORGENERATOR_H

#include <list>
#include <map>
#include <set>
#include <vector>
#include <jsoncpp/json/json.h>

#include "dlstring.h"
#include "flags.h"
#include "stringset.h"
#include "affect.h"

class Object;
class PCharacter;

/** The seven armor slots the generator knows, in armor_names.json order. */
bool armor_slot_exists(const DLString &slot);

/** Price of one item_affixes.json affix in centi-M for the profile, 0 when unknown.
 *  The gear sage scores what it can't see as an affect (a worn buff) by this. */
int item_affix_price(const DLString &section, const DLString &value, bool caster);

/**
 * Random armor on one of the base prototypes (limbo 110, 112-117), the armor
 * twin of WeaponGenerator. Affixes come from fight/item_affixes.json, priced in
 * centi-M (100 = one item measure) against the tier window in weapon_tiers.json
 * (min_m/max_m/max_penalty_m). Nouns, genders and materials come from
 * fight/armor_names.json. Stats scale in measure rolls: level / slot factor,
 * the same unit crafted armor uses (item_value.json "measure").
 *
 * The caller sets obj->level first and the weight afterwards (Fenia
 * .tmp.weight.calculateWeight knows the material densities).
 *
 * Specials the engine reads off the instance:
 *   property "wornbuff"   -- one spell the base vnum's onEquip casts permanently;
 *   props["combatcast"]   -- [{spell, chance, hp_below}] procs.
 */
class ArmorGenerator {
public:
    ArmorGenerator(Object *obj, PCharacter *pch, int tier, const DLString &slot);

    ArmorGenerator & caster(bool caster) { this->isCaster = caster; return *this; }
    ArmorGenerator & alignment(int align) { this->align = align; return *this; }

    /** Roll everything. False when the configuration can't serve this slot. */
    bool run();

private:
    struct Candidate {
        DLString section;
        DLString value;
        DLString norm;      // value without a leading '-' or '+'
        const Json::Value *affix;
        int price;          // centi-M, one stack
        int stack;
        int weight;
        int tierFloor;
        std::set<DLString> conflicts;
        DLString conflictsWith;
    };

    bool pickNoun();
    void collectCandidates();
    bool candidateAllowed(const Json::Value &section, const Json::Value &affix, int floor) const;
    bool conflicts(const Candidate &c, const std::map<int, int> &chosen) const;
    void pickAffixes();
    void applyAffixes();
    void applyOne(const Candidate &c, int count);
    void applyPack(const Json::Value &affects, int count);
    void assignAC();
    void assignNames();
    void assignFlags();

    int rolls() const;
    int modifier(const Json::Value &affix, int count) const;
    void remember(Affect &af);
    bool materialAllowed(const DLString &name) const;
    DLString defaultMaterial() const;

    Object *obj;
    PCharacter *pch;
    int tier;
    DLString slot;
    bool isCaster;
    int align;

    Json::Value nounConfig;
    DLString materialName;
    std::set<int> playerGroups;

    std::vector<Candidate> pool;
    std::map<int, int> chosen;          // pool index -> stack count
    int chosenTotal;                    // centi-M actually spent

    Flags extraFlags;
    std::list<Affect> affects;
    DLString wornBuff;
    Json::Value procs;
    StringSet affixNames;
};

#endif
