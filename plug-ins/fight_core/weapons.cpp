/* $Id$
 *
 * ruffina, 2004
 */
#include <math.h>
#include "jsoncpp/json/json.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillreference.h"
#include "logstream.h"
#include "core/object.h"
#include "character.h"
#include "room.h"
#include "affect.h"
#include "configurable.h"

#include "weapons.h"
#include "math_utils.h"
#include "material.h"
#include "material-table.h"
#include "attacks.h"
#include "itemflags.h"
#include "affectflags.h"
#include "damageflags.h"
#include "loadsave.h"
#include "dl_math.h"
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
WeaponCalculator:: WeaponCalculator(int tier, int level, bitnumber_t wclass) : tier(tier), level(level), wclass(wclass) 
{
    calcValue2Range();
    calcAve();
    calcValues();
    calcDamroll();
}

int WeaponCalculator::getTierIndex() const
{
    int index = level / 5;
    int penalty = weapon_level_penalty[wclass].asInt();
    index = max(0, index + penalty);
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
{
    sn = gsn_none;
    valTier = hrTier = drTier = 5;
    hrCoef = drCoef = 0;
    hrMinValue = drMinValue = 0;
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
    return WeaponCalculator(drTier, obj->level, obj->value0()).getDamroll();
}

int WeaponGenerator::maxHitroll() const
{
    return WeaponCalculator(hrTier, obj->level, obj->value0()).getDamroll();
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

const WeaponGenerator & WeaponGenerator::assignNames() const
{
    DLString wclass = weapon_class.name(obj->value0());
    Json::Value &names = weapon_names[wclass];

    if (names.empty()) {
        warn("Weapon generator: no names defined for type %s.", wclass.c_str());
        return *this;
    }

    int index = number_range(0, names.size() - 1);
    Json::Value &entry = names[index];

    // Config item names and gram gender. 
    StringList mynames(entry["name"].asString());
    mynames.addUnique(wclass);
    mynames.addUnique(weapon_class.message(obj->value0()));
    obj->setName(mynames.join(" ").c_str());
    obj->setShortDescr(entry["short"].asCString());
    obj->setDescription(entry["long"].asCString());
    obj->gram_gender = MultiGender(entry["gender"].asCString());

    // Set up 'two handed' weapon flag: 2 - two hands, 1 - one hand, 0 - either.
    DLString twohand = entry["twohand"].asString();
    if (twohand == "2" || (twohand.empty() && chance(10))) {
        obj->value4(obj->value4() | WEAPON_TWO_HANDS);
    }

    // Set weight: 0.4 kg by default in OLC, 2kg for two hand.
    // TODO: Weight is very approximate, doesn't depend on weapon type.
    if (IS_SET(obj->value4(), WEAPON_TWO_HANDS))
        obj->weight *= 5;

    // Set up provided material or default.
    obj->setMaterial(findMaterial(entry).c_str());

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamageType() const
{
    DLString wclass = weapon_class.name(obj->value0());
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
DLString WeaponGenerator::findMaterial(Json::Value &entry) const
{
    // Find by exact name, e.g. "fish".
    DLString mname = entry["material"].asString();
    const material_t *material = material_by_name(mname);
    if (material)
        return material->name;

    // Find a random material name for each of requested types.
    StringList materials;
    for (auto &mtype: entry["mtypes"]) {
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

