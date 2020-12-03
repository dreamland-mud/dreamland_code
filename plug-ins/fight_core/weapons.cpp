/* $Id$
 *
 * ruffina, 2004
 */
#include <math.h>
#include <algorithm>
#include <random>
#include <bitset>
#include <unordered_set>

#include "profiler.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillreference.h"
#include "logstream.h"
#include "core/object.h"
#include "character.h"
#include "room.h"
#include "affect.h"

#include "weapons.h"
#include "weapon-generator.h"
#include "morphology.h"
#include "math_utils.h"
#include "material.h"
#include "material-table.h"
#include "attacks.h"
#include "itemflags.h"
#include "affectflags.h"
#include "damageflags.h"
#include "loadsave.h"
#include "dl_math.h"
#include "act.h"
#include "merc.h"
#include "def.h"

WEARLOC(wield);
WEARLOC(second_wield);

GSN(none);  GSN(exotic);      GSN(sword);        GSN(dagger);
GSN(spear); GSN(mace);        GSN(axe);          GSN(flail);
GSN(whip);  GSN(polearm);     GSN(bow);          GSN(arrow);
GSN(lance); GSN(throw_stone); GSN(hand_to_hand);

Skill * get_weapon_skill( Object *wield )
{
    switch (wield->value0())
    {
        default :               return &*gsn_none;
        case(WEAPON_EXOTIC):    return &*gsn_exotic;
        case(WEAPON_SWORD):     return &*gsn_sword;
        case(WEAPON_DAGGER):    return &*gsn_dagger;
        case(WEAPON_SPEAR):     return &*gsn_spear;
        case(WEAPON_MACE):      return &*gsn_mace;
        case(WEAPON_AXE):       return &*gsn_axe;
        case(WEAPON_FLAIL):     return &*gsn_flail;
        case(WEAPON_WHIP):      return &*gsn_whip;
        case(WEAPON_POLEARM):   return &*gsn_polearm;
        case(WEAPON_BOW):           return &*gsn_bow;
        case(WEAPON_ARROW):           return &*gsn_arrow;
        case(WEAPON_LANCE):           return &*gsn_lance;
        case(WEAPON_STONE):         return &*gsn_throw_stone;                                
   }
}

bitnumber_t get_weapon_for_skill(Skill *skill)
{
    int sn = skill->getIndex();
    
    if (sn == gsn_sword)
        return WEAPON_SWORD;
    else if (sn == gsn_dagger)
        return WEAPON_DAGGER; 
    else if (sn == gsn_spear)
        return WEAPON_SPEAR; 
    else if (sn == gsn_mace)
        return WEAPON_MACE; 
    else if (sn == gsn_axe)
        return WEAPON_AXE; 
    else if (sn == gsn_flail)
        return WEAPON_FLAIL; 
    else if (sn == gsn_polearm)
        return WEAPON_POLEARM; 
    else if (sn == gsn_bow)
        return WEAPON_BOW; 
    else
        return -1;
}
    
Object * get_wield( Character *ch, bool secondary )
{
    return secondary ? wear_second_wield->find( ch ) : wear_wield->find( ch );
}


int get_weapon_sn( Object *wield )
{
    int sn;

    if (wield == 0 || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else
        sn = get_weapon_skill( wield )->getIndex( );

   return sn;
}

    
int get_weapon_sn( Character *ch, bool secondary )
{
    return get_weapon_sn( get_wield( ch, secondary ) );
}

int weapon_ave(Object *wield)
{
    if (wield->item_type == ITEM_WEAPON)
        return dice_ave(wield->value1(), wield->value2());
    else
        return 0;
}

int weapon_ave(struct obj_index_data *pWield)
{
    if (pWield->item_type == ITEM_WEAPON)
        return dice_ave(pWield->value[1], pWield->value[2]);
    else
        return 0;
}

/*-----------------------------------------------------------------------------
 * Weapon tiers
 *-----------------------------------------------------------------------------*/
void weapon_tier_t::fromJson(const Json::Value &value)
{
    num = value["num"].asInt();
    rname = value["rname"].asString();
    aura = value["aura"].asString();
    colour = value["colour"].asString();
    extra.fromJson(value["extra"]);
    min_points = value["min_points"].asInt();
    max_points = value["min_points"].asInt();
}

json_vector<weapon_tier_t> weapon_tier_table;
CONFIGURABLE_LOADED(fight, weapon_tiers)
{
    weapon_tier_table.fromJson(value);
}


/*-----------------------------------------------------------------------------
 * Weapon prefixes
 *-----------------------------------------------------------------------------*/

Json::Value weapon_prefix;
CONFIGURABLE_LOADED(fight, weapon_prefix)
{
    weapon_prefix = value;
}

/** Helper structure to access prefix configuration. */
struct prefix_info {
    prefix_info(string name, int price, int stack, const Json::Value &entry) 
            : name(name), price(price), stack(stack), entry(entry)
    {        
    }

    string name;
    int price;
    int stack;
    const Json::Value &entry;
};

static bool sort_by_price(const prefix_info &p1, const prefix_info &p2)
{
    return p1.price <= p2.price;
}

/** This class can generate all combinations of prefixes that match given tier and its price range. */
struct prefix_generator {
    /** A mask that can hold boolean information about 100 affixes. */
    typedef bitset<100> bucket_mask_t;

    prefix_generator(int t) : tier(weapon_tier_table[t-1]) 
    {
        ProfilerBlock prof("generate prefixes", 10);

        collectPrefixesForTier();
        markExclusions();

        minPrice = tier.min_points;
        maxPrice = tier.max_points;
        requirements = 0L;
    }

    /** Mark a certain prefix as forbidden in all combinations. */
    void addExclusion(const DLString &name)
    {
        int index = getPrefixIndex(name);
        if (index >= 0)
            for (auto &ex: exclusions)
                ex.set(index);
    }

    /** Mark a certain prefix as required (always chosen). */
    void addRequirement(const DLString &name)
    {
        int index = getPrefixIndex(name);
        if (index >= 0)
            requirements.set(index);
    }

    void run() 
    {
        generateBuckets(0, 0, 0L);

        notice("Weapon generator: found %d result buckets for tier %d and %d prefixes", 
                buckets.size(), tier.num, prefixes.size());
    }

    /** Produces a single random prefix combination out of all generated ones. */
    list<prefix_info> getSingleResult() const
    {
        list<prefix_info> result;
        bucket_mask_t bucket = randomBucket();

        for (unsigned int i = 0; i < prefixes.size(); i++)
            if (bucket.test(i))
                result.push_back(prefixes[i]);

        return result;
    }

    int getResultSize() const
    {
        return buckets.size();
    }

private:

    int getPrefixIndex(const DLString &name)
    {
        for (unsigned int i = 0; i < prefixes.size(); i++)
            if (prefixes[i].entry["value"].asString() == name)
                return i;
        return -1;
    }

    list<int> getPrefixIndexes(const DLString &name)
    {
        list<int> result;
        for (unsigned int i = 0; i < prefixes.size(); i++)
            if (prefixes[i].entry["value"].asString() == name)
                result.push_back(i);
        return result;
    }

    /** Choose a random set element. */
    bucket_mask_t randomBucket() const 
    {
        vector<bucket_mask_t> random_sample;
        sample(buckets.begin(), buckets.end(), 
               back_inserter(random_sample), 1, mt19937{random_device{}()});
        return random_sample.front();
    }

    /** Recursively produce masks were 1 marks an included prefix, 0 marks an excluded prefix.
     *  Each mask denotes a combination of prefixes those total price matches prices for the tier. 
     */
    void generateBuckets(int currentTotal, long unsigned int index, bucket_mask_t currentMask) 
    {
        if (currentTotal >= minPrice && currentTotal <= maxPrice)
            // Good combo, remember it and continue.
            buckets.insert(currentMask);
        else if (currentTotal > maxPrice) 
            // Stop now: adding more prefixes will only make the price bigger.
            return;

        if (index >= prefixes.size())
            // Stop now: reached the end of prefixes vector.
            return;

        // Stop now: adding this or any subsequent price will still exceed maxPrice.
        if (currentTotal + prefixes[index].price > maxPrice)
            return;

        // First check whether current prefix doesn't conflict with any prefix chosen earlier.
        if ((currentMask & exclusions[index]).none()) {
            // Explore all further combinations that can happen if this prefix is included.
            currentMask.set(index);
            generateBuckets(currentTotal + prefixes[index].price, index + 1, currentMask);
        }

        // First check whether current prefix is required and cannot be excluded.
        if (!requirements.test(index)) {
            // Explore all further combinations that can happen if this prefix is excluded.
            currentMask.reset(index);
            generateBuckets(currentTotal, index + 1, currentMask);
        }
    }

    /** Creates a vector of all prefixes that are allowed for the tier, sorted by price in ascending order. */
    void collectPrefixesForTier()
    {
        list<prefix_info> sorted;

        if (!weapon_prefix.isObject()) {
            bug("weapon_prefix is not a well-formed json object.");
            return;
        }

        for (auto &section: weapon_prefix.getMemberNames()) {
            for (auto &entry: weapon_prefix[section]) {
                int threshold = entry.isMember("tier") ? entry["tier"].asInt() : WORST_TIER;
                if (tier.num > threshold)
                    continue;

                // See if negative counterpart has to be generated for this affix.
                bool both = entry.isMember("both") ? entry["both"].asBool() : false;

                // Decide how many times this affix has to be repeated, from 1 to 'stack'.
                int stack = entry.isMember("stack") ? entry["stack"].asInt() : 1;
                for (int s = 1; s <= stack; s++) {
                    int price = entry["price"].asInt();                
                    sorted.push_back(prefix_info(section, price * s, s, entry));
                    if (both)
                        sorted.push_back(prefix_info(section, -price * s, s, entry));
                }
            }
        }

        sorted.sort(sort_by_price);
        for (auto &p: sorted)
            prefixes.push_back(p);
    }

    /** Creates a NxN matrix of prefix flags, marking those that are mutually exclusive.
     */
    void markExclusions() 
    {
        exclusions.resize(prefixes.size());

        for (unsigned int i = 0; i < prefixes.size(); i++) {            
            bucket_mask_t &exclusion = exclusions[i];
            prefix_info &p = prefixes[i];

            if (p.name == "material")
                for (unsigned int j = 0; j < prefixes.size(); j++)
                    if (i != j && prefixes[j].name == p.name)
                        exclusion.set(j);

            for (auto &excl: p.entry["excl"]) {
                int index = getPrefixIndex(excl.asString());
                if (index >= 0) {
                    exclusion.set(index);
                    exclusions[index].set(i);
                }
            }
        }

        // Mark all stacked values such as +hr, -hr as mutually exclusive.
        for (unsigned int i = 0; i < prefixes.size(); i++) {
            const prefix_info &p = prefixes[i];
            for (auto &same: getPrefixIndexes(p.entry["value"].asString()))
                if (i != same)
                    exclusions[i].set(same);
        }
    }

    /** Keeps info about current tier. */
    weapon_tier_t &tier;

    /** Keeps all avaialble prefixes sorted by price. */
    vector<prefix_info> prefixes;

    /** Keeps all possible combination matching tier's price. If a bit M is set in a bucket mask, then prefix M is included. */
    unordered_set<bucket_mask_t> buckets;

    /** Marks prefixes that need to always be included in the result. */
    bucket_mask_t requirements;

    /** Keeps all exclusions for prefixes. If entry N has a bit M set, then prefixes N and M are mutually exclusive. */
    vector<bucket_mask_t> exclusions;

    int minPrice;
    int maxPrice;
    bitnumber_t align;
};

// Debug util: grab several good prefix combinations for the tier. 
list<list<string>> random_weapon_prefixes(int tier, int count)
{
    list<list<string>> allNames;
    prefix_generator gen(tier);
    gen.run();

    for (int i = 0; i < min(count, gen.getResultSize()); i++) {
        list<string> names;
        for (auto &pi: gen.getSingleResult())
            names.push_back(pi.entry["value"].asString());
        allNames.push_back(names);
    }

    return allNames;
}

/*-----------------------------------------------------------------------------
 * Other configuration tables for weapon generator.
 *-----------------------------------------------------------------------------*/

Json::Value weapon_value2_by_class;
CONFIGURABLE_LOADED(fight, weapon_value2)
{
    weapon_value2_by_class = value;
}

Json::Value weapon_damtype;
CONFIGURABLE_LOADED(fight, weapon_damtype)
{
    weapon_damtype = value;
}

Json::Value weapon_level_penalty;
CONFIGURABLE_LOADED(fight, weapon_level_penalty)
{
    weapon_level_penalty = value;
}

Json::Value weapon_ave_penalty;
CONFIGURABLE_LOADED(fight, weapon_ave_penalty)
{
    weapon_ave_penalty = value;
}

Json::Value weapon_ave_tiers;
CONFIGURABLE_LOADED(fight, weapon_ave_tiers)
{
    weapon_ave_tiers = value;
}

Json::Value weapon_damroll_tiers;
CONFIGURABLE_LOADED(fight, weapon_damroll_tiers)
{
    weapon_damroll_tiers = value;
}

Json::Value weapon_names;
CONFIGURABLE_LOADED(fight, weapon_names)
{
    weapon_names = value;
}

/** Helper struct to keep interim result of value1/value2 calculations. 
 *  A list of these structures will be sorted to find the one closest to requested ave.
 */
struct weapon_value_t {
    weapon_value_t(int v1, int v2, int ave) {
        this->v1 = v1;
        this->v2 = v2;
        this->ave = ave;
        real_ave = dice_ave(v1, v2);
    }

    int distance() const { 
        return abs(real_ave - ave); 
    }

    int v1;
    int v2;
    int real_ave;
    int ave;
};

static bool sort_by_ave_distance(const weapon_value_t &w1, const weapon_value_t &w2)
{
    return w1.distance() <= w2.distance();
}

/*--------------------------------------------------------------------------
 * WeaponCalculator
 *-------------------------------------------------------------------------*/
WeaponCalculator:: WeaponCalculator(int tier, int level, bitnumber_t wclass, int index_bonus) 
    : tier(tier), level(level), wclass(wclass), index_bonus(index_bonus)
{
    calcValue2Range();
    calcAve();
    calcValues();
    calcDamroll();
}

WeaponGenerator::~WeaponGenerator()
{

}

int WeaponCalculator::getTierIndex() const
{
    int index = level / 5;
    int penalty = weapon_level_penalty[wclass].asInt();
    index = max(0, index + penalty + index_bonus);
    return index;
}

/** Retrieve min and max value2 for a given weapon class. */
void WeaponCalculator::calcValue2Range()
{
    if (wclass < 0 || wclass >= (int)weapon_value2_by_class.size()) {
        bug("weapon_value2: invalid weapon class %d", wclass);
        return;
    }

    Json::Value &entry = weapon_value2_by_class[wclass];
    if (entry.isInt()) {
        v2_min = v2_max = entry.asInt();
        return;
    }

    if (entry.isArray() && entry.size() == 2) {
        v2_min = entry[0].asInt();
        v2_max = entry[1].asInt();
        return;
    }

    bug("weapon_value2: invalid values provided for class %d", wclass);
}

/** Retrieve desired ave damage for this tier and weapon class. */
void WeaponCalculator::calcAve() 
{
    if (tier <= 0 || tier > (int)weapon_ave_tiers.size()) {
        bug("weapon_ave: invalid tier %d for level %d", tier, level);
        return;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_ave: invalid level %d for tier %d", level, tier);
        return;
    }

    if (wclass < 0 || wclass >= (int)weapon_ave_penalty.size()) {
        bug("weapon_ave: invalid weapon class %d for penalty table", wclass);
        return;
    }

    Json::Value &one_tier = weapon_ave_tiers[tier-1];
    int index = getTierIndex();
    if (index >= (int)one_tier.size()) {
        bug("weapon_ave: tier %d of size %d doesn't have enough values for level %d", tier, one_tier.size(), level);
        return;
    }

    float multiplier = weapon_ave_penalty[wclass].asFloat();
    int tier_ave = one_tier[index].asInt();
    ave = (int)(multiplier * tier_ave);
}

/** Calculate value1 and resulting value2 (between min and max) for the requested ave damage. */
void WeaponCalculator::calcValues() 
{
    if (ave <= 0)
        return;

    // Calculate all possible v1 and v2, and their respective real average damage.
    list<weapon_value_t> weapon_value_candidates;
    for (int v2 = v2_min; v2 <= v2_max; v2++) {
        float value1_float = 2 * ave / (v2 + 1);
        int value1 = ceil(value1_float);

        weapon_value_candidates.push_back(weapon_value_t(value1, v2, ave));
        weapon_value_candidates.push_back(weapon_value_t(value1-1, v2, ave));
        weapon_value_candidates.push_back(weapon_value_t(value1+1, v2, ave));
    }

    // Find the best v1/v2 combo that gives us a dice closest to the ave from json tables.
    weapon_value_candidates.sort(sort_by_ave_distance);
    const weapon_value_t &winner = weapon_value_candidates.front();    
    value2 = winner.v2;
    value1 = winner.v1;
    real_ave = winner.real_ave;
}

/** Retrieve damroll for this tier and level. */
void WeaponCalculator::calcDamroll() 
{
    if (tier <= 0 || tier > (int)weapon_damroll_tiers.size()) {
        bug("weapon_damroll: invalid tier %d for level %d", tier, level);
        return;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_damroll: invalid level %d for tier %d", level, tier);
        return;
    }

    Json::Value &one_tier = weapon_damroll_tiers[tier-1];
    int index = getTierIndex();

    if (index >= (int)one_tier.size()) {
        bug("weapon_damroll: tier %d of size %d doesn't have enough values for level %d", tier, one_tier.size(), level);
        return;
    }

    damroll = one_tier[index].asInt();
}

/*--------------------------------------------------------------------------
 * WeaponGenerator
 *-------------------------------------------------------------------------*/
WeaponGenerator::WeaponGenerator()
        : extraFlags(0, &extra_flags),
          weaponFlags(0, &weapon_type2)
{
    sn = gsn_none;
    valTier = hrTier = drTier = 5;
    hrCoef = drCoef = 0;
    hrMinValue = drMinValue = 0;
    hrIndexBonus = drIndexBonus = 0;
}

WeaponGenerator & WeaponGenerator::item(Object *obj)
{ 
     this->obj = obj; 
     wclass = weapon_class.name(obj->value0());
     return *this; 
}

const WeaponGenerator & WeaponGenerator::assignValues() const
{    
    WeaponCalculator calc(valTier, obj->level, obj->value0());
    obj->value1(calc.getValue1());
    obj->value2(calc.getValue2());
    return *this;
}

int WeaponGenerator::maxDamroll() const
{
    return WeaponCalculator(drTier, obj->level, obj->value0(), drIndexBonus).getDamroll();
}

int WeaponGenerator::maxHitroll() const
{
    return WeaponCalculator(hrTier, obj->level, obj->value0(), hrIndexBonus).getDamroll();
}

int WeaponGenerator::minDamroll() const
{
    return max( drMinValue, (int)(drCoef * maxDamroll()));
}

int WeaponGenerator::minHitroll() const
{
    return max( hrMinValue, (int)(hrCoef * maxHitroll()));
}

const WeaponGenerator & WeaponGenerator::assignHitroll() const
{
    setAffect(APPLY_HITROLL, maxHitroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamroll() const
{
    setAffect(APPLY_DAMROLL, maxDamroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignStartingHitroll() const
{
    setAffect(APPLY_HITROLL, minHitroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignStartingDamroll() const
{
    setAffect(APPLY_DAMROLL, minDamroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::incrementHitroll() const
{
    Affect *paf_hr = obj->affected.find( sn, APPLY_HITROLL );
    if (paf_hr) {
        int oldMod = paf_hr->modifier;
        int min_hr = minHitroll();
        int max_hr = maxHitroll();
        paf_hr->modifier = URANGE( min_hr, oldMod + 1, max_hr );

        if (obj->carried_by && (obj->wear_loc == wear_wield || obj->wear_loc == wear_second_wield)) {
            obj->carried_by->hitroll += paf_hr->modifier - oldMod;
        }
    }

    return *this;
}

const WeaponGenerator & WeaponGenerator::incrementDamroll() const
{
    Affect *paf_dr = obj->affected.find( sn, APPLY_DAMROLL );
    if (paf_dr) {
        int oldMod = paf_dr->modifier;
        int min_dr = minDamroll();
        int max_dr = maxDamroll();
        paf_dr->modifier = URANGE( min_dr, oldMod + 1, max_dr );

        if (obj->carried_by && (obj->wear_loc == wear_wield || obj->wear_loc == wear_second_wield)) {
            obj->carried_by->damroll += paf_dr->modifier - oldMod;
        }
    }
    
    return *this;
}

void WeaponGenerator::setAffect(int location, int modifier) const
{
    int skill = sn < 0 ? gsn_none : sn;
    Affect *paf = obj->affected.find(sn, location);

    if (!paf) {
        Affect af;

        af.type = skill;
        af.level = obj->level;
        af.duration = -1;
        af.location = location;
        affect_to_obj(obj, &af);

        paf = obj->affected.front();
    }

    paf->modifier = modifier;
}

WeaponGenerator & WeaponGenerator::randomNames()
{
    const Json::Value &configs = weapon_names[wclass];

    if (configs.empty()) {
        warn("Weapon generator: no names defined for type %s.", wclass.c_str());
        return *this;
    }

    int index = number_range(0, configs.size() - 1);
    nameConfig = configs[index];
    return *this;
}

WeaponGenerator & WeaponGenerator::randomAffixes()
{
    prefix_generator gen(valTier);

    // Set exclusions or requirements based on chosen names and weapon flags.
    DLString twohand = nameConfig["twohand"].asString();
    if (twohand == "1")
        gen.addExclusion("two_hands");
    else if (twohand == "2")
        gen.addRequirement("two_hands");

    // Generate all combinations of affixes.
    gen.run();

    if (gen.getResultSize() == 0) {
        warn("Weapon generator: no prefixes found for tier %d.", valTier);
        return *this;
    }    

    // Collect all configurations mandated by given set of affixes: flags, material, affects.
    auto result = gen.getSingleResult();
    int minPrice = result.front().price;
    int maxPrice = result.back().price;

    for (auto &pinfo: result) {
        const Json::Value &prefix = pinfo.entry;
        const DLString &section = pinfo.name;
        obj->carried_by->pecho("{DAffix %s [%d]", prefix["value"].asCString(), pinfo.price);

        if (section == "flag") {
            extraFlags.setBits(prefix["extra"].asString());
            weaponFlags.setBits(prefix["value"].asString());
        } else if (section == "extra") {
            extraFlags.setBits(prefix["value"].asString());
        } else if (section == "material") {
            materialName = prefix["value"].asString();
        }

        // TODO collect data for suffixes, including hr/dr/ave tier bonuses.

        // Each adjective or noun has a chance to be chosen, but the most expensive get an advantage.
        for (auto &adj: prefix["adjectives"])
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price)
                adjectives.push_back(adj.asString());

        for (auto &noun: prefix["nouns"])
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price)
                nouns.push_back(noun.asString());
    }

    obj->carried_by->pecho("{DExtras %s, weapon flags %s, material %s{x", 
        extraFlags.names().c_str(), weaponFlags.names().c_str(), materialName.c_str());

    return *this;
}

void WeaponGenerator::setName() const
{
    StringList mynames(nameConfig["name"].asString());
    mynames.addUnique(wclass);
    mynames.addUnique(weapon_class.message(obj->value0()));
    obj->setName(mynames.join(" ").c_str());
}

void WeaponGenerator::setShortDescr() const
{
    DLString randomAdjective, randomNoun; 

    obj->gram_gender = MultiGender(nameConfig["gender"].asCString());

    if (!adjectives.empty()) {
        int a = number_range(0, adjectives.size() - 1);
        randomAdjective = adjectives[a];
    }

    if (!nouns.empty()) {
        int n = number_range(0, nouns.size() - 1);
        randomNoun = nouns[n];
    }

    DLString colour = weapon_tier_table[valTier-1].colour;
    DLString myshort;

    if (!colour.empty())
        myshort = "{" + colour;

    if (!randomAdjective.empty())
        myshort += Morphology::adjective(randomAdjective, obj->gram_gender) + " "; // леденящий

    myshort += nameConfig["short"].asString(); // буздыган

    if (!randomNoun.empty())
        myshort += " " + randomNoun; // боли

    if (!colour.empty())
        myshort += "{x";

    obj->setShortDescr(myshort.c_str());
}

const WeaponGenerator & WeaponGenerator::assignNames() const
{
    // Config item names and gram gender. 
    setName();
    setShortDescr();
    obj->setDescription(nameConfig["long"].asCString());

    // Set up provided material or default.
    obj->setMaterial(findMaterial().c_str());

    obj->properties["tier"] = valTier;
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignAffects() const
{
    // TODO applya affects from suffixes.
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignFlags() const
{
    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, weapon_tier_table[valTier-1].extra.getValue());
    obj->value4(obj->value4() | weaponFlags.getValue());

    // Set weight: 0.4 kg by default in OLC, 2kg for two hand.
    // TODO: Weight is very approximate, doesn't depend on weapon type.
    if (IS_SET(obj->value4(), WEAPON_TWO_HANDS))
        obj->weight *= 5;

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamageType() const
{
    Json::Value &entry = weapon_damtype[wclass];

    if (entry.empty()) {
        warn("Weapon generator: no damage types defined for type %s.", wclass.c_str());
        return *this;
    }

    StringSet attacks = StringSet(entry["attacks"].asString()); // frbite, divine, etc
    StringSet damtypes = StringSet(entry["damtypes"].asString()); // bash, pierce, etc
    bool any = damtypes.count("any") > 0;
    vector<int> result;

    for (int a = 0; attack_table[a].name != 0; a++) {
        const attack_type &attack = attack_table[a];
        if (any 
            || attacks.count(attack.name) > 0
            || damtypes.count(damage_table.name(attack.damage)) > 0)
        {
            result.push_back(a);
        }
    }

    if (result.empty()) {
        warn("Weapon generator: no matching damtype found for %s.", wclass.c_str());
        return *this;
    }

    obj->value3(
        result.at(number_range(0, result.size() - 1)));

    return *this;
}

/** Look up material based on suggested names or types. 
 *  Return 'metal' if nothing found.
 */
DLString WeaponGenerator::findMaterial() const
{
    // First analyze prefix requirements for material.
    if (!materialName.empty())
        return materialName;

    // Find by exact name, e.g. "fish".
    DLString mname = nameConfig["material"].asString();
    const material_t *material = material_by_name(mname);
    if (material)
        return material->name;

    // Find a random material name for each of requested types.
    StringList materials;
    for (auto &mtype: nameConfig["mtypes"]) {
        bitstring_t type = material_types.bitstring(mtype.asString());
        auto withType = materials_by_type(type);

        if (!withType.empty())
            materials.push_back(
                withType.at(number_range(0, withType.size() - 1))->name);
    }

    // Concatenate two or more material names, e.g. "pine, steel".
    if (!materials.empty())
        return materials.join(", ");

    return "metal";
}

