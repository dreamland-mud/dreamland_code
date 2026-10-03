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

/** The whole fight/item_affixes.json, for the weapon side of the pool. */
const Json::Value & item_affixes_config();

/**
 * The shared half of the random item generators: the affix pool from
 * fight/item_affixes.json, the weighted fill of a tier's M window, the shared
 * sections' affects, and the name parts. A section is served to a generator
 * when its 'items' is the generator's kind or "both"; a section without
 * 'items' counts as armor. Prices are centi-M (100 = one item measure).
 */
class ItemAffixRoller {
public:
    ItemAffixRoller(Object *obj, PCharacter *pch, int tier, const DLString &kind, const DLString &slot);
    virtual ~ItemAffixRoller();

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

    /** One prefix or suffix candidate for the name: an affix entry and its weight. */
    struct NamePart {
        int weight;
        const Json::Value *affix;
    };

protected:
    void learnPlayerGroups();
    void collectCandidates();
    /** Weighted fill of [minM, maxM]. 'forced' pool indexes are taken first and
     *  ignore the window and the caps: a class or name that requires an affix. */
    void pickAffixes(int minM, int maxM, int worstPenalty, int maxAffixes, int maxNegatives,
                     const std::set<int> &forced);
    bool conflicts(const Candidate &c, const std::map<int, int> &picked) const;

    /** Affects, flags and materials of the sections both generators share.
     *  False when the section is not a shared one. */
    bool applyShared(const Candidate &c, int count);
    void applyPack(const Json::Value &affects, int count);
    /** Put the collected affects on the item. Bit and scoped affects never merge
     *  (affect_enhance merges by location and type only, and would fold two of
     *  them into one); plain stats do. */
    void flushAffects();

    /** Pick one prefix and one suffix, the dearer the likelier. */
    void pickNameParts(std::vector<NamePart> &prefixes, std::vector<NamePart> &suffixes,
                       const Json::Value *&pre, int &adjIndex,
                       const Json::Value *&suf, int &nounIndex) const;

    int rolls() const;
    int modifier(const Json::Value &affix, int count) const;
    void remember(Affect &af);

    /** Generator-specific filters and prices. */
    virtual bool candidateAllowed(const Json::Value &section, const Json::Value &affix, int floor) const;
    virtual bool valueAllowed(const DLString &secName, const DLString &value) const;
    virtual int candidatePrice(const DLString &secName, const Json::Value &affix, bool &allowed) const;
    virtual int candidateWeight(const DLString &secName, const Json::Value &affix) const;

    Object *obj;
    PCharacter *pch;
    int tier;
    DLString kind;
    DLString slot;
    bool isCaster;
    int align;

    std::set<int> playerGroups;
    std::vector<Candidate> pool;
    std::map<int, int> chosen;          // pool index -> stack count
    int chosenTotal;                    // centi-M actually spent

    Flags extraFlags;
    std::list<Affect> affects;
    DLString materialName;
    StringSet affixNames;
};

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
class ArmorGenerator : public ItemAffixRoller {
public:
    ArmorGenerator(Object *obj, PCharacter *pch, int tier, const DLString &slot);

    ArmorGenerator & caster(bool caster) { this->isCaster = caster; return *this; }
    ArmorGenerator & alignment(int align) { this->align = align; return *this; }

    /** Roll everything. False when the configuration can't serve this slot. */
    bool run();

protected:
    virtual bool valueAllowed(const DLString &secName, const DLString &value) const;

private:
    bool pickNoun();
    void applyAffixes();
    void applyOne(const Candidate &c, int count);
    void assignAC();
    void assignNames();
    void assignFlags();

    bool materialAllowed(const DLString &name) const;
    DLString defaultMaterial() const;

    Json::Value nounConfig;
    DLString wornBuff;
    Json::Value procs;
};

#endif
