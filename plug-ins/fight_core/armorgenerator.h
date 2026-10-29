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

/** The whole fight/item_affixes.json, for the weapon side of the pool. */
const Json::Value & item_affixes_config();

/** Section "charged" of item_affixes.json: how many charges a spell of this
 *  one-charge price (M) can take on an item of this tier and level. kind
 *  "armor" = jewelry and held items cast from anywhere, "weapon" = staves cast
 *  wielded. False when it does not fit. */
bool charged_charge_range(const DLString &kind, int tier, int level, bool caster, double price, int &lo, int &hi);

/** Writes props "charged"; level is the cast level before the tier's cast_offset. */
void charged_write(Object *obj, const DLString &kind, int tier, const DLString &spell, int charges, int level);

#define IMBUE_NO_SPELL  (-1)
#define IMBUE_TOO_WEAK  (-2)

/** Shalafi imbue: one chosen spell as charges, the most the charged affix
 *  rules allow at this tier. The spell must be in the item's pool (its level
 *  reach, AoE rule, alignment). Returns the charges, or IMBUE_NO_SPELL /
 *  IMBUE_TOO_WEAK; write false only measures. */
int magic_item_imbue(Object *obj, const DLString &spell, int tier, const DLString &kind, int level, int align, bool write = true);

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
        bool sigOnly;       // needs_boss: only a boss signature may pick it
        DLString spell;     // section "charged": the spell and its charges
        int charges;
    };

    /** One prefix or suffix candidate for the name: an affix entry and its weight. */
    struct NamePart {
        int weight;
        const Json::Value *affix;
    };

protected:
    void learnPlayerGroups();
    void collectCandidates();
    /** Weighted fill of [minM, maxM]. 'forced' pool indexes (index -> stacks) are
     *  taken first and ignore the window and the caps: a class or name that
     *  requires an affix, a crafted material's own affix, a refit replay. */
    void pickAffixes(int minM, int maxM, int worstPenalty, int minAffixes, int maxAffixes, int maxNegatives,
                     const std::map<int, int> &forced);
    /** "section:value[:stacks]" words, space separated, to pool indexes. Words
     *  that are not in this item's pool are dropped: the fill rolls in their place. */
    std::map<int, int> resolveForced(const DLString &spec) const;
    /** The picks as resolveForced reads them back. */
    DLString picksSpec() const;
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

    /** One item model (fight_core/itemmodel.h): the affix's base points at the
     *  item level, false for a section the model does not price (those keep their
     *  price_melee / price_caster). */
    bool modelPoints(const DLString &secName, const Json::Value &affix, double &points) const;
    /** Points -> centi-M at the item level for the roller's profile and slot. */
    int modelPrice(double points) const;
    /** Fit clause for the killer: no stat over its cap, counted on perm stats
     *  (worn gear changes, the item stays). A pack of nothing but primary stats
     *  is out when any of them is capped (Kit 2026-10-28). What the killer
     *  already has (resists, bits, buffs) still rolls (Kit 2026-10-26). */
    bool fitAllowed(const DLString &secName, const Json::Value &affix) const;
    /** One item model: the 'pick' weight multiplier for the killer's class or
     *  profile at the item level (affix first, then section); 1 when absent. */
    double pickMultiplier(const Json::Value &section, const Json::Value &affix) const;
    /** caster / hybrid / melee, from the killer's profession flags. */
    DLString pickProfile() const;
    /** Tier window multiplier by item level (decision 3); 1 when the model is off. */
    double windowCurve() const;
    void remember(Affect &af);

    /** Generator-specific filters and prices. */
    virtual bool candidateAllowed(const Json::Value &section, const Json::Value &affix, int floor) const;
    virtual bool valueAllowed(const DLString &secName, const DLString &value) const;
    virtual int candidatePrice(const DLString &secName, const Json::Value &affix, bool &allowed) const;
    virtual double candidateWeight(const DLString &secName, const Json::Value &affix) const;
    /** Section "charged": a spell from the affix's magic_items pool for this
     *  item kind and its charges, priced charge x charges (centi-M) inside the
     *  section's share of the tier window. False when nothing fits. */
    bool chargedPick(const Json::Value &section, const Json::Value &affix, Candidate &c) const;
    /** A learned ancient language (affix "skills") short of native, or -1. */
    int pickTongue(const Json::Value &affix) const;
    double learnedValueMult() const;

    /** A crafted piece (craft/armor rollCraft): no core picks and no stat floor,
     *  the crafter's fixed stats are already on the item. */
    bool crafted;
    /** "section:value" of the crafted forced picks. */
    std::set<DLString> forcedKeys;
    /** Measure rolls the caller counted (a jeweler's steeper factor), 0 = item_rolls. */
    int rollsOverride;

    Object *obj;
    PCharacter *pch;
    int tier;
    DLString kind;
    DLString slot;
    bool isCaster;
    int align;
    /** A boss signature is wanted: needs_boss affixes join the pool (sigOnly). */
    bool sigPool;

    std::set<int> playerGroups;
    std::map<int, int> groupCounts;     // group -> learned skills of the killer in it
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
    /** Hand-made armor ('random armor' behavior): keep the item's own names,
     *  gender, material and wear slot, roll only affixes, AC, tier and timer.
     *  The slot may be any wear slot (about, waist, neck...): it only picks the
     *  measure rolls and the affix slot weights. No worn buff: that needs the
     *  base vnums' onEquip. */
    ArmorGenerator & keepIdentity(bool keep) { this->keepName = keep; return *this; }
    /** A crafted piece: own names, AC and colour stay, the window is the
     *  caller's centi-M as given (no level curve), forced picks first, and the
     *  picks are written to the "craft_picks" property for a refit replay. */
    ArmorGenerator & craft(int minCm, int maxCm, const DLString &forcedSpec)
    {
        this->keepName = true;
        this->crafted = true;
        this->craftMin = minCm;
        this->craftMax = maxCm;
        this->forcedSpec = forcedSpec;
        return *this;
    }
    /** Measure rolls to scale the stats with, when the caller counts them its own way. */
    ArmorGenerator & measureRolls(int rolls) { this->rollsOverride = rolls; return *this; }
    /** centi-M the rolled affixes cost. */
    int total() const { return chosenTotal; }
    /** Boss signature (legendary only): the dead boss's damage type and the
     *  spells it casts. One of them goes on the item, 50/50, inside the budget. */
    ArmorGenerator & signature(const DLString &damtype, const std::vector<DLString> &spells)
    {
        this->sigDamtype = damtype;
        this->sigSpells = spells;
        this->sigPool = !damtype.empty() || !spells.empty();
        return *this;
    }
    /** The dead boss, for the signature piece's name ("Jher's malice"). */
    ArmorGenerator & signatureBoss(Character *boss);

    /** Roll everything. False when the configuration can't serve this slot. */
    bool run();

protected:
    virtual bool valueAllowed(const DLString &secName, const DLString &value) const;

private:
    bool pickNoun();
    /** Pool index of the boss signature affix, -1 when there is none. */
    int pickSignature() const;
    void applyAffixes();
    void applyOne(const Candidate &c, int count);
    void assignAC();
    void assignNames();
    /** A signature piece is named after its boss; false when it can't be. */
    bool assignSignatureNames();
    /** keepIdentity: the tier colour on the item's own names. */
    void assignColours();
    void assignFlags();

    bool materialAllowed(const DLString &name) const;
    DLString defaultMaterial() const;

    Json::Value nounConfig;
    DLString sigDamtype;
    std::vector<DLString> sigSpells;
    /** The boss's name: genitive in Russian and Ukrainian, nominative in English. */
    DLString sigBossRu, sigBossUa, sigBossEn;
    int sigIndex;
    DLString wornBuff;
    Json::Value procs;
    bool keepName;
    int craftMin, craftMax;
    DLString forcedSpec;
};

#endif
