#include <algorithm>
#include <cmath>

#include "weapongenerator.h"
#include "weaponcalculator.h"
#include "weapontier.h"
#include "weaponaffixes.h"
#include "armorgenerator.h"
#include "itemvalue.h"
#include "itemmodel.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillreference.h"
#include "core/object.h"
#include "pcharacter.h"

#include "damageflags.h"
#include "morphology.h"
#include "material.h"
#include "material-table.h"
#include "attacks.h"
#include "loadsave.h"
#include "dl_math.h"
#include "math_utils.h"
#include "merc.h"
#include "def.h"

GSN(none);
WEARLOC(wield);
WEARLOC(second_wield);

/** A weapon name is metal-only when its configuration leaves no other option:
 *  either a metallic material by name, or 'mtypes' listing nothing but metal.
 *  Names without any material configured are free to take a non-metal default.
 */
static bool name_is_metal_only(const Json::Value &nameConfig)
{
    const material_t *material = material_by_name(nameConfig["material"].asString());
    if (material)
        return IS_SET(material->type, MAT_METAL);

    const Json::Value &mtypes = nameConfig["mtypes"];
    if (mtypes.empty())
        return false;

    for (auto const &mtype: mtypes)
        if (!IS_SET(material_types.bitstring(mtype.asString()), MAT_METAL))
            return false;

    return true;
}

static bool json_list_has(const Json::Value &list, const char *name)
{
    for (auto const &v: list)
        if (v.asString() == name)
            return true;
    return false;
}

Json::Value weapon_classes;
CONFIGURABLE_LOADED(fight, weapon_classes)
{
    weapon_classes = value;
}

Json::Value weapon_names;
CONFIGURABLE_LOADED(fight, weapon_names)
{
    weapon_names = value;
}

/*--------------------------------------------------------------------------
 * WeaponGenerator
 *-------------------------------------------------------------------------*/
WeaponGenerator::WeaponGenerator()
        : extraFlags(0, &extra_flags),
          weaponFlags(0, &weapon_type2)
{
    pch = 0;
    sn = gsn_none;
    valTier = hrTier = drTier = 5;
    hrCoef = drCoef = 0;
    hrMinValue = drMinValue = 0;
    hrIndexBonus = drIndexBonus = aveIndexBonus = 0;
    align = ALIGN_NONE;
    isCaster = false;
    aveMult = damrollMult = 1;
    twoHands = twoHandsDecided = false;
    retainChance = 50;
    wclassFixed = false;
    // Every assign*/random* method dereferences obj; item() is what sets it. Start
    // it null so a chain built in the wrong order crashes readably instead of
    // running off an indeterminate pointer.
    obj = 0;
}

bool weapon_class_exists(const DLString &name)
{
    return !name.empty() && weapon_classes.isMember(name);
}

DLString best_weapon_class(PCharacter *pch)
{
    if (!pch)
        return DLString::emptyString;

    vector<DLString> best;
    int bestPercent = 0;

    for (auto const &name: weapon_classes.getMemberNames()) {
        // Same availability filter randomWeaponClass() uses: a class the player
        // cannot use at all is not a reward, it is a paperweight.
        Skill *skill = skillManager->find(name);
        if (!skill || !skill->available(pch))
            continue;

        // A skill reporting nothing learned can't win: the base Skill and an
        // unusable GenericSkill both report 0, and a zero is not a preference.
        // (The 'arrow' entry never reaches here at all -- it is a BasicSkill, and
        // those are unavailable by definition, so the filter above drops it.)
        int percent = skill->getEffective(pch);
        if (percent < 1 || percent < bestPercent)
            continue;

        if (percent > bestPercent) {
            bestPercent = percent;
            best.clear();
        }

        best.push_back(name);
    }

    if (best.empty())
        return DLString::emptyString;

    // Ties are broken at random on purpose: a fresh character sits at 1% across
    // every available class, and there is no principled winner among them.
    return best[number_range(0, best.size() - 1)];
}

WeaponGenerator::~WeaponGenerator()
{

}

WeaponGenerator & WeaponGenerator::item(Object *obj)
{ 
    this->obj = obj; 
    wclass = weapon_class.name(obj->value0());
    
    if (weapon_classes.isMember(wclass))
        wclassConfig = weapon_classes[wclass];
    else
        warn("Weapon generator: no configuration defined for weapon class %s.", wclass.c_str());

     return *this; 
}

WeaponGenerator & WeaponGenerator::tier(int tier)
{
    valTier = hrTier = drTier = tier;
    return *this;
}

// Pick target tier according to each tier's chances, but no better than provided bestTier.
WeaponGenerator & WeaponGenerator::randomTier(int bestTier, int legendaryPerMille)
{
    tier(random_weapon_tier(bestTier, legendaryPerMille));
    return *this;
}

// Assign random weapon class available to a player, or any class configured.
WeaponGenerator & WeaponGenerator::randomWeaponClass()
{
    Json::Value::Members allClasses =  weapon_classes.getMemberNames();

    if (pch)
        allClasses.erase( // Remove all weapon classes n/a to the player.
            remove_if(
                allClasses.begin(), allClasses.end(), [this](const string &c) {
                    Skill *skill = skillManager->find(c);
                    return !skill || !skill->available(pch);
                }), 
            allClasses.end());

    if (allClasses.empty())
        return *this;

    unsigned int random_index = number_range(0, allClasses.size() - 1);
    applyWeaponClass(allClasses[random_index]);

    return *this;
}

// Assign a caller-chosen weapon class and keep randomizeAll() from rolling over it.
WeaponGenerator & WeaponGenerator::weaponClass(const DLString &name)
{
    // No class requested: stay chainable and let randomizeAll() roll one.
    if (name.empty())
        return *this;

    if (!obj) {
        warn("Weapon generator: weaponClass(%s) called before item(); ignored.", name.c_str());
        return *this;
    }

    if (!weapon_class_exists(name)) {
        warn("Weapon generator: unknown weapon class %s requested, rolling one instead.", name.c_str());
        return *this;
    }

    applyWeaponClass(name);
    wclassFixed = true;
    return *this;
}

void WeaponGenerator::applyWeaponClass(const DLString &name)
{
    wclass = name;
    wclassConfig = weapon_classes[wclass];
    obj->value0(weapon_class.value(wclass));

    // Keep some extra flags (e.g. for shops) but clean everything else.
    obj->extra_flags &= ITEM_INVENTORY;
}

const WeaponGenerator & WeaponGenerator::assignValues() const
{    
    WeaponCalculator calc(valTier, obj->level, obj->value0(), aveIndexBonus, aveMult);
    obj->value1(calc.getValue1());
    obj->value2(calc.getValue2());
    return *this;
}

int WeaponGenerator::maxDamroll() const
{
    return (int)(WeaponCalculator(drTier, obj->level, obj->value0(), drIndexBonus).getDamroll() * damrollMult);
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
        // Remove old affects from paf_hr.
        if (obj->carried_by)
            obj->wear_loc->affectsOnUnequip(obj->carried_by, obj);

        int oldMod = paf_hr->modifier;
        int min_hr = minHitroll();
        int max_hr = maxHitroll();
        paf_hr->modifier = URANGE( min_hr, oldMod + 1, max_hr );

        // Restore affects with updated hitroll.
        if (obj->carried_by)
            obj->wear_loc->affectsOnEquip(obj->carried_by, obj);
    }

    return *this;
}

const WeaponGenerator & WeaponGenerator::incrementDamroll() const
{
    Affect *paf_dr = obj->affected.find( sn, APPLY_DAMROLL );
    if (paf_dr) {
        // Remove old affects from paf_dr.        
        if (obj->carried_by)
            obj->wear_loc->affectsOnUnequip(obj->carried_by, obj);

        int oldMod = paf_dr->modifier;
        int min_dr = minDamroll();
        int max_dr = maxDamroll();
        paf_dr->modifier = URANGE( min_dr, oldMod + 1, max_dr );

        // Restore affects with updated damroll.
        if (obj->carried_by)
            obj->wear_loc->affectsOnEquip(obj->carried_by, obj);
    }
    
    return *this;
}

void WeaponGenerator::setAffect(int location, int modifier) const
{
    if (modifier == 0)
        return;

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

    // Don't offer names that can only be forged of metal to a player who can't wield it.
    bool noMetal = rejectsMetal();
    vector<Json::ArrayIndex> allowed;
    for (Json::ArrayIndex i = 0; i < configs.size(); i++)
        if ((!noMetal || !name_is_metal_only(configs[i])) && nameFitsHands(configs[i]))
            allowed.push_back(i);

    // A two-hander takes a two-handed name when the class has one, else any
    // name that does not forbid two hands.
    if (twoHandsDecided && twoHands) {
        vector<Json::ArrayIndex> twoHanded;
        for (auto i: allowed)
            if (json_list_has(configs[i]["requires"], "two_hands"))
                twoHanded.push_back(i);
        if (!twoHanded.empty())
            allowed = twoHanded;
    }

    if (allowed.empty()) {
        warn("Weapon generator: all names for type %s are metal-only.", wclass.c_str());
        for (Json::ArrayIndex i = 0; i < configs.size(); i++)
            allowed.push_back(i);
    }

    int index = number_range(0, allowed.size() - 1);
    nameConfig = configs[allowed.at(index)];
    return *this;
}

/** True when the player this weapon is generated for can never wield metal, e.g. a druid. */
bool WeaponGenerator::rejectsMetal() const
{
    return pch && IS_SET(material_types_forbidden(pch), MAT_METAL);
}

/*--------------------------------------------------------------------------
 * Weapons on the M budget
 *-------------------------------------------------------------------------*/
static const Json::Value & weapon_m_config()
{
    return item_affixes_config()["_weapons"];
}


/** Two-handedness is a property of the weapon, decided before the name and the
 *  affixes: the caller may require or forbid it (re-statting a weapon keeps its
 *  hands), a class that requires two hands always has them, a class with
 *  two_hand_chance has them that often, every other class never. */
void WeaponGenerator::decideTwoHands()
{
    twoHandsDecided = true;

    if (required.count("two_hands"))
        twoHands = true;
    else if (forbidden.count("two_hands"))
        twoHands = false;
    else if (json_list_has(wclassConfig["requires"], "two_hands"))
        twoHands = true;
    else
        twoHands = chance(wclassConfig["two_hand_chance"].asInt());
}

/** A name that requires two hands only for a two-hander, one that forbids them
 *  only for a one-hander. Anything goes until the hands are decided. */
bool WeaponGenerator::nameFitsHands(const Json::Value &config) const
{
    if (!twoHandsDecided)
        return true;
    if (twoHands)
        return !json_list_has(config["forbids"], "two_hands");
    return !json_list_has(config["requires"], "two_hands");
}

namespace {

/** The weapon side of the item affix pool. */
class WeaponAffixRoller : public ItemAffixRoller {
public:
    WeaponAffixRoller(Object *obj, PCharacter *pch, int tier, float share)
        : ItemAffixRoller(obj, pch, tier, "weapon", "wield"),
          share(share), weaponFlags(0, &weapon_type2),
          hrBonus(0), drBonus(0), aveBonus(0)
    {
    }

    void setCaster(bool caster) { isCaster = caster; }
    void setAlign(int a) { align = a; }

    void run(const std::set<DLString> &requiredNames, int maxAffixesBonus)
    {
        const weapon_tier_t &t = weapon_tier_table[tier - 1];

        learnPlayerGroups();
        collectCandidates();

        std::set<int> forced;
        for (int i = 0; i < (int)pool.size(); i++)
            if (requiredNames.count(pool[i].value))
                forced.insert(i);

        double curve = windowCurve();
        pickAffixes((int)(t.min_m * share * curve), (int)(t.max_m * share * curve), t.worst_penalty_m,
                    t.max_affixes_m > 0 ? t.max_affixes_m + maxAffixesBonus : 0,
                    t.max_negatives_m, forced);

        for (auto const &p: chosen)
            applyOne(pool[p.first], p.second);
    }

    /** One prefix and one suffix, the two-handed adjective among the prefixes. */
    void names(const Json::Value *twoHands, const Json::Value *&pre, int &a,
               const Json::Value *&suf, int &n) const
    {
        vector<NamePart> prefixes, suffixes;
        if (twoHands)
            prefixes.push_back({max(1, chosenTotal / 2), twoHands});
        pickNameParts(prefixes, suffixes, pre, a, suf, n);
    }

    const std::list<Affect> & getAffects() const { return affects; }
    const Flags & getExtraFlags() const { return extraFlags; }
    const DLString & getMaterial() const { return materialName; }
    const StringSet & getAffixNames() const { return affixNames; }
    int getTotal() const { return chosenTotal; }

    std::set<DLString> forbidden;
    std::set<DLString> preferred;
    float share;
    Flags weaponFlags;
    float hrBonus, drBonus, aveBonus;

protected:
    virtual bool valueAllowed(const DLString &secName, const DLString &value) const
    {
        return forbidden.count(value) == 0;
    }

    /** weapon_tier: a floor for weapons only (damroll/hitroll stats from rare up). */
    virtual bool candidateAllowed(const Json::Value &section, const Json::Value &affix, int floor) const
    {
        if (affix.isMember("weapon_tier"))
            floor = min(floor, affix["weapon_tier"].asInt());
        return ItemAffixRoller::candidateAllowed(section, affix, floor);
    }

    virtual int candidateWeight(const DLString &secName, const Json::Value &affix) const
    {
        int weight = ItemAffixRoller::candidateWeight(secName, affix);
        if (preferred.count(affix["value"].asString()))
            weight *= 3;
        return weight;
    }

    virtual int candidatePrice(const DLString &secName, const Json::Value &affix, bool &allowed) const
    {
        if (secName == "flag")
            return flagPrice(affix, allowed);

        if (affix["dynamic_price"].asBool()) {
            int price = stepPrice(affix);
            allowed = (price != 0);
            return price;
        }

        return ItemAffixRoller::candidatePrice(secName, affix, allowed);
    }

private:
    /** price_by_level interpolated at the item level, clamped to the end points. */
    int flagPrice(const Json::Value &affix, bool &allowed) const
    {
        const Json::Value &table = affix["price_by_level"];
        std::map<int, int> points;
        for (auto const &lvl: table.getMemberNames())
            points[DLString(lvl).toInt()] = table[lvl].asInt();

        allowed = false;
        if (points.empty())
            return 0;

        int level = obj->level, price;
        double curve = windowCurve();
        if (level <= points.begin()->first)
            price = points.begin()->second;
        else if (level >= points.rbegin()->first)
            price = points.rbegin()->second;
        else {
            auto hi = points.upper_bound(level);
            auto lo = std::prev(hi);
            price = lo->second + (hi->second - lo->second) * (level - lo->first) / (hi->first - lo->first);
        }

        // The flag's points at the item level (combat-effect model).
        if (affix.isMember("points_by_level"))
            price = modelPrice(item_points_by_level(affix["points_by_level"], level));

        // Allowed on the cheapest tier whose window covers the price, and on every
        // better one; dearer than the legendary window, never.
        int gate = 0;
        for (int t = WORST_TIER; t >= BEST_TIER; t--)
            if (price <= weapon_tier_table[t - 1].max_m * share * curve) {
                gate = t;
                break;
            }

        if (affix.isMember("min_tier"))
            gate = min(gate, affix["min_tier"].asInt());

        allowed = (gate > 0 && tier <= gate);
        return price;
    }

    /** A step of the base table (ave, damroll, hitroll index) priced at the item
     *  level by the item_value weights the gear sage uses: one M is one level's
     *  worth (item_value "level") at the reference level, scaled by measure rolls. */
    int stepPrice(const Json::Value &affix) const
    {
        DLString value = affix["value"].asString();
        DLString norm = (!value.empty() && (value.at(0) == '-' || value.at(0) == '+')) ? value.substr(1) : value;
        float step = affix["step"].asFloat();
        int wclass = obj->value0();
        const char *profile = isCaster ? "caster" : "melee";

        double points;
        if (norm == "ave") {
            int now = WeaponCalculator(tier, obj->level, wclass, 0, share).getAve();
            int then = WeaponCalculator(tier, obj->level, wclass, step, share).getAve();
            points = (then - now) * item_value(profile, "weapon_weight", 12);
        } else if (norm == "dr") {
            int now = WeaponCalculator(tier, obj->level, wclass, 0).getDamroll();
            int then = WeaponCalculator(tier, obj->level, wclass, step).getDamroll();
            points = (then - now) * share * item_value(profile, "damroll", 12);
        } else if (norm == "hr") {
            int now = WeaponCalculator(tier, obj->level, wclass, 0).getDamroll();
            int then = WeaponCalculator(tier, obj->level, wclass, step).getDamroll();
            points = (then - now) * item_value(profile, "hitroll", 6);
        } else {
            return 0;
        }

        double oneM = item_one_m(obj->level, isCaster);
        if (oneM <= 0)
            return 0;

        return (int)std::round(100 * points / oneM);
    }

    void applyOne(const Candidate &c, int count)
    {
        const Json::Value &affix = *c.affix;
        const DLString &sec = c.section;

        if (sec == "flag") {
            affixNames.insert(c.value);
            extraFlags.setBits(affix["extra"].asString());
            weaponFlags.setBits(c.value);

        } else if (sec == "weapon_material") {
            affixNames.insert(c.value);
            extraFlags.setBits(affix["extra"].asString());
            materialName = c.value;

        } else if (sec == "affects_by_tier") {
            affixNames.insert(c.value);
            extraFlags.setBits(affix["extra"].asString());
            float bonus = affix["step"].asFloat() * count;
            if (c.norm == "hr")
                hrBonus += bonus;
            else if (c.norm == "dr")
                drBonus += bonus;
            else if (c.norm == "ave")
                aveBonus += bonus;

        } else if (!applyShared(c, count)) {
            warn("Weapon generator: affix %s in unknown section %s.", c.value.c_str(), sec.c_str());
        }
    }
};

}

WeaponGenerator & WeaponGenerator::randomAffixes()
{
    const Json::Value &config = weapon_m_config();
    if (!twoHandsDecided)
        decideTwoHands();
    float share = twoHands ? config["two_hand_k"].asFloat() : 1;
    if (share < 1)
        share = 1;

    WeaponAffixRoller roller(obj, pch, valTier, share);
    roller.setCaster(isCaster);
    roller.setAlign(align);

    // Same exclusions the points generator honours: the caller's, the class's and
    // the name's, metal for a druid, hr/dr steps that round to nothing here.
    set<DLString> requiredNames = required;
    auto addAll = [](const Json::Value &list, set<DLString> &to) {
        for (auto const &v: list)
            to.insert(v.asString());
    };
    roller.forbidden = forbidden;
    addAll(wclassConfig["forbids"], roller.forbidden);
    addAll(nameConfig["forbids"], roller.forbidden);
    addAll(wclassConfig["requires"], requiredNames);
    addAll(nameConfig["requires"], requiredNames);
    addAll(wclassConfig["prefers"], roller.preferred);
    addAll(nameConfig["prefers"], roller.preferred);

    if (rejectsMetal()) {
        roller.forbidden.insert("platinum");
        roller.forbidden.insert("titanium");
    }
    if (maxHitroll() <= 0) {
        roller.forbidden.insert("hr");
        roller.forbidden.insert("-hr");
    }
    if (maxDamroll() <= 0) {
        roller.forbidden.insert("dr");
        roller.forbidden.insert("-dr");
    }

    roller.run(requiredNames, twoHands ? 1 : 0);

    weaponFlags.setBit(roller.weaponFlags.getValue());
    if (twoHands) {
        weaponFlags.setBit(WEAPON_TWO_HANDS);
        extraFlags.setBits(config["two_hands"]["extra"].asString());
        aveMult = damrollMult = share;
    }

    // Additional flags configured for weapon class, as the points generator does.
    for (auto const &flag: wclassConfig["flags"].getMemberNames())
        if (chance(wclassConfig["flags"][flag].asInt()))
            weaponFlags.setBits(flag);

    extraFlags.setBit(roller.getExtraFlags().getValue());
    if (!roller.getMaterial().empty())
        materialName = roller.getMaterial();
    hrIndexBonus += roller.hrBonus;
    drIndexBonus += roller.drBonus;
    aveIndexBonus += roller.aveBonus;

    for (auto af: roller.getAffects())
        affects.push_back(af);

    // One adjective and one noun, so setShortDescr has exactly one of each to take.
    const Json::Value *pre, *suf;
    int a, n;
    roller.names(twoHands ? &config["two_hands"] : 0, pre, a, suf, n);

    auto form = [](const Json::Value *affix, const char *field, int idx) -> DLString {
        if (!affix || idx < 0)
            return DLString::emptyString;
        const Json::Value &forms = (*affix)[field];
        return idx < (int)forms.size() ? DLString(forms[idx].asString()) : DLString::emptyString;
    };

    if (pre) {
        adjectives.push_back(form(pre, "adjectives", a));
        adjectives_en.push_back(form(pre, "adjectives_en", a));
        adjectives_ua.push_back(form(pre, "adjectives_ua", a));
    }
    if (suf) {
        nouns.push_back(form(suf, "nouns", n));
        nouns_en.push_back(form(suf, "nouns_en", n));
        nouns_ua.push_back(form(suf, "nouns_ua", n));
    }

    StringSet names = roller.getAffixNames();
    if (twoHands)
        names.insert("two_hands");
    obj->setProperty("affixes", names.toString());
    obj->setProperty("measure_m", roller.getTotal());

    return *this;
}

WeaponGenerator& WeaponGenerator::randomizeStats()
{    
    randomAffixes()
    .assignHitroll()
    .assignDamroll()
    .assignFlags()
    .assignValues()
    .assignAffects()
    .assignTimers()
    .assignColours();

    notice("rand_stat: created item %s [%d] [%lld] tier %s affixes [%s]",
            obj->getShortDescr('1', LANG_DEFAULT).c_str(),
            obj->pIndexData->vnum, obj->getID(), 
            obj->getProperty("tier").c_str(),
            obj->getProperty("affixes").c_str());

    return *this;
}

WeaponGenerator& WeaponGenerator::randomizeAll()
{
    // weaponClass() has already pinned and applied the class; don't roll over it.
    if (!wclassFixed)
        randomWeaponClass();

    // The hands come first, so the name can match them.
    decideTwoHands();

    randomNames()
        .randomAffixes()
        .assignHitroll()
        .assignDamroll()
        .assignFlags()
        .assignValues()
        .assignAffects()
        .assignTimers()
        .assignNames()
        .assignDamageType()
        .assignColours();

    notice("rand_all: created item %s [%d] [%lld] tier %s affixes [%s] level %d",
            obj->getShortDescr('1', LANG_DEFAULT).c_str(),
            obj->pIndexData->vnum, obj->getID(), 
            obj->getProperty("tier").c_str(), 
            obj->getProperty("affixes").c_str(),
            obj->level);

    return *this;        
}

/** Add obj affect to the storage to be applied later. */
void WeaponGenerator::rememberAffect(Affect &af)
{
    af.type = gsn_none;
    af.duration = -1;
    af.level = obj->level;

    affects.push_back(af);
}

void WeaponGenerator::setName() const
{
    StringList mynames(nameConfig["name"].asString());
    mynames.addUnique(wclass);
    mynames.addUnique(weapon_class.message(obj->value0()));
    obj->setKeyword(mynames.join(" ").c_str());
}

/** Glue one language's weapon name together: "леденящий буздыган боли".
 *  Shared by generation and by the repair pass below, so the two can never
 *  disagree about spacing or field order. Empty parts drop out. */
static DLString compose_short(const DLString &adjective, const DLString &base, const DLString &noun)
{
    DLString result;

    if (!adjective.empty())
        result += adjective + " ";
    result += base;
    if (!noun.empty())
        result += " " + noun;

    return result;
}

/** pymorphy3 gender tag for the one-letter 'gender' of a weapon_names entry. */
static DLString gender_tag(const DLString &gender)
{
    if (gender == "m") return "masc";
    if (gender == "f") return "femn";
    if (gender == "n") return "neut";
    if (gender == "p") return "plur";
    return "-";
}

// Defined further down (with the lazy-repair path). Forward-declared so
// setShortDescr can share the same authored-pad override at generation time.
static bool decline_ua(const DLString &word, const DLString &pos, const DLString &gtag, DLString &result);

void WeaponGenerator::setShortDescr() const
{
    obj->gram_gender = MultiGender(nameConfig["gender"].asCString());

    // Pick one affix adjective + noun (same index across languages).
    int a = -1, n = -1;
    if (!adjectives.empty())
        a = number_range(0, adjectives.size() - 1);
    if (!nouns.empty())
        n = number_range(0, nouns.size() - 1);

    // --- Russian (unchanged) ---
    DLString myshort = compose_short(
        a >= 0 ? Morphology::adjective(adjectives[a], obj->gram_gender) : DLString::emptyString, // леденящий
        nameConfig["short"].asString(),                                                          // буздыган
        n >= 0 ? nouns[n] : DLString::emptyString);                                              // боли

    obj->setShortDescr(myshort, LANG_RU);
    obj->setProperty("eqName", nameConfig["short"].asString()); // 'буздыган' in sheath wearloc

    // --- English: plain per-language forms if authored, else mirror RU ---
    // eqName stays the Russian base noun (find_name_config keys on it); the sheath
    // wearloc reads eqName_en / eqName_ua for non-Russian viewers. Left empty when
    // the form isn't authored, so the sheath falls back to the viewer's full name.
    obj->setProperty("eqName_en", nameConfig.isMember("short_en") ? DLString(nameConfig["short_en"].asString()) : DLString::emptyString);
    obj->setProperty("eqName_ua", DLString::emptyString);

    if (nameConfig.isMember("short_en")) {
        obj->setShortDescr(compose_short(
            a >= 0 && a < (int)adjectives_en.size() ? adjectives_en[a] : DLString::emptyString,
            nameConfig["short_en"].asString(),
            n >= 0 && n < (int)nouns_en.size() ? nouns_en[n] : DLString::emptyString), LANG_EN);
    } else {
        obj->setShortDescr(myshort, LANG_EN);
    }

    // --- Ukrainian: decline nominative forms via the sidecar, else mirror RU ---
    if (nameConfig.isMember("short_ua")) {
        DLString gtag = gender_tag(nameConfig["gender"].asString());
        DLString adjUa = a >= 0 && a < (int)adjectives_ua.size() ? adjectives_ua[a] : DLString::emptyString;

        // Route through decline_ua (not Morphology::declineUa directly) so an
        // authored pad in short_ua overrides pymorphy for the words it declines
        // wrong. The bool return is irrelevant here: at generation time a sidecar
        // miss still writes what it managed, exactly as the direct call did.
        DLString baseUa, adjUaDeclined;
        decline_ua(nameConfig["short_ua"].asString(), "NOUN", gtag, baseUa);
        obj->setProperty("eqName_ua", baseUa); // declined pad, read by the sheath wearloc
        if (!adjUa.empty())
            decline_ua(adjUa, "ADJF", gtag, adjUaDeclined);

        obj->setShortDescr(compose_short(
            adjUaDeclined,
            baseUa,
            // Suffix nouns ("... of pain") are a fixed genitive -- appended as-is,
            // like RU, so they stay put when the weapon name declines by case.
            n >= 0 && n < (int)nouns_ua.size() ? nouns_ua[n] : DLString::emptyString), LANG_UA);
    } else {
        obj->setShortDescr(myshort, LANG_UA);
    }
}

const WeaponGenerator & WeaponGenerator::assignNames() const
{
    // Config item names and gram gender. 
    setName();
    setShortDescr();
    obj->setDescription(nameConfig["long"].asCString(), LANG_RU);
    obj->setDescription(nameConfig.isMember("long_en") ? nameConfig["long_en"].asCString() : nameConfig["long"].asCString(), LANG_EN);
    obj->setDescription(nameConfig.isMember("long_ua") ? nameConfig["long_ua"].asCString() : nameConfig["long"].asCString(), LANG_UA);

    // Set up provided material or default.
    obj->setMaterial(findMaterial());
    return *this;
}

/*-----------------------------------------------------------------------------
 * Repairing weapons generated before the generator spoke all three languages
 *----------------------------------------------------------------------------*/
// Object::getShortDescr(lang) is strict per language: own slot, then PROTOTYPE
// slot, then nothing. It never falls back to Russian. So a weapon carrying a
// generated Russian name and empty English/Ukrainian slots does not read as
// "untranslated" to those players -- it reads as the un-randomized prototype,
// which for the limbo blank (vnum 104) is the debug stub "[dummy random weapon]".
// Recover the missing languages from the very config the generator used.

/** Locate the weapon_names entry a generated weapon was built from. The 'short'
 *  values are unique across every weapon class, so the eqName the generator
 *  stored on the item identifies exactly one entry. */
static bool find_name_config(const DLString &eqName, Json::Value &result)
{
    for (auto &wclass: weapon_names.getMemberNames())
        for (auto const &config: weapon_names[wclass])
            if (config["short"].asString() == eqName) {
                result = config;
                return true;
            }

    return false;
}

/** Find the parallel EN/UA forms of one affix word out of a generated name.
 *  Adjectives are stored as a declension pattern, so every candidate is declined
 *  with this weapon's gender before comparing; nouns are appended raw and compare
 *  directly. The three arrays in weapon_affixes.json are authored index-aligned
 *  and no Russian form appears twice, which is what makes the recovery exact
 *  rather than a guess. Returns false when nothing matches. */
static bool find_affix_form(const DLString &field, const DLString &russian,
                            const MultiGender &gender, DLString &en, DLString &ua)
{
    bool isAdjective = (field == "adjectives");

    for (auto &section: weapon_affixes.getMemberNames())
        for (auto const &affix: weapon_affixes[section]["values"]) {
            const Json::Value &forms = affix[field.c_str()];

            for (Json::ArrayIndex k = 0; k < forms.size(); k++) {
                DLString candidate = forms[k].asString();
                if (isAdjective)
                    candidate = Morphology::adjective(candidate, gender);
                if (candidate != russian)
                    continue;

                const Json::Value &formsEn = affix[(field + "_en").c_str()];
                const Json::Value &formsUa = affix[(field + "_ua").c_str()];
                en = k < formsEn.size() ? DLString(formsEn[k].asString()) : DLString::emptyString;
                ua = k < formsUa.size() ? DLString(formsUa[k].asString()) : DLString::emptyString;
                return true;
            }
        }

    return false;
}

/** Decline one Ukrainian word, and say whether the answer is real.
 *
 *  Morphology::declineUa signals an unreachable pymorphy3 sidecar only by the
 *  shape of what it returns: "<word>|||||", five pipes, one cell short of a pad.
 *  A word that genuinely does not decline comes back as a full "<word>||||||"
 *  (six), so the two are told apart without asking the sidecar anything.
 *
 *  Checking each answer beats probing the sidecar once up front: successful
 *  lookups are cached forever, so a probe stops measuring the sidecar the moment
 *  its own word is in the cache -- and a sidecar that died after that would have
 *  its fallbacks written straight into a saved field, where the idempotency
 *  guard then blocks any repair from ever trying again. Failed lookups are never
 *  cached (morphology.cpp returns the fallback before it touches the cache), so
 *  refusing to write leaves the slot for a later read to fix. */
static bool decline_ua(const DLString &word, const DLString &pos,
                       const DLString &gtag, DLString &result)
{
    // An authored short_ua may already BE a full Flexer pad (it contains a
    // pipe) -- the override for the handful of words pymorphy declines wrong
    // (kukri, tanto, kamcha...). Take such a pad verbatim rather than feeding
    // an already-declined string back through the sidecar.
    if (word.find('|') != DLString::npos) {
        result = word;
        return true;
    }

    result = Morphology::declineUa(word, pos, gtag);
    return result != word + "|||||";
}

/** Tier colour the generator wrapped this weapon's name in, empty for the tiers
 *  that carry none. Read from the item so the repair does not need a live
 *  generator state to match what assignColours() did. */
static DLString repair_tier_colour(Object *obj)
{
    DLString tier = obj->getProperty("tier");
    if (!tier.isNumber())
        return DLString::emptyString;

    int num = tier.toInt();
    if (num < 1 || num > (int)weapon_tier_table.size())
        return DLString::emptyString;

    return weapon_tier_table[num - 1].colour;
}

/** Drop the Russian text a pre-trilingual binary pinned into the English and
 *  Ukrainian slots of a re-statted weapon. Those slots override a prototype that
 *  names itself perfectly well in both languages, so clearing them is the fix --
 *  but only ever for a slot that literally holds the prototype's own Russian, and
 *  only when the prototype has something to show in its place. Anything else is
 *  somebody's deliberate edit and is left alone. */
static bool clear_pinned_russian(Object *obj)
{
    const XMLMultiString &proto = obj->pIndexData->short_descr;
    DLString protoRu = proto.get(LANG_RU).colourStrip();
    bool changed = false;

    if (protoRu.empty())
        return false;

    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        if (lang == LANG_RU)
            continue;
        if (obj->getRealShortDescr(lang).colourStrip() != protoRu)
            continue;
        if (proto.get(lang).empty())
            continue;

        obj->setShortDescr(DLString::emptyString, lang);
        changed = true;
    }

    return changed;
}

bool weapon_repair_names(Object *obj)
{
    if (obj->getProperty("tier").empty())
        return false;

    // A re-statted weapon keeps the prototype's name and writes no slot of its
    // own. When only the Russian slot is empty, what the other two hold is the
    // pinned-Russian leak, not a generated name to complete.
    // Copied, not referenced: the writes further down touch the same field.
    DLString ownRu = obj->getRealShortDescr(LANG_RU);
    if (ownRu.empty())
        return clear_pinned_russian(obj);

    bool needShort = obj->getRealShortDescr(LANG_EN).empty()
                     || obj->getRealShortDescr(LANG_UA).empty();
    bool needDescr = obj->getRealDescription(LANG_EN).empty()
                     || obj->getRealDescription(LANG_UA).empty();
    if (!needShort && !needDescr)
        return false;

    Json::Value config;
    DLString eqName = obj->getProperty("eqName");
    if (eqName.empty() || !find_name_config(eqName, config))
        return false; // not a name this generator composed: nothing to decompose

    bool changed = false;

    if (needShort) {
        // "леденящий буздыган боли" splits at the base noun, which eqName names
        // exactly. What sits in front is the declined adjective, what trails is
        // the suffix noun; either may be absent.
        DLString bare = ownRu.colourStrip();
        DLString base = config["short"].asString();
        DLString::size_type at = bare.find(base);

        // Only colour the new languages if the Russian name is coloured at all:
        // a weapon made at a tier that carried no colour must not come back
        // wearing one. The shade itself still comes from today's tier table, so
        // a re-tuned tier leaves Russian on the old colour until it regenerates.
        DLString colour = (ownRu == bare) ? DLString::emptyString : repair_tier_colour(obj);

        if (at == DLString::npos) {
            warn("weapon repair: obj %d has eqName '%s' outside its own name '%s'.",
                 obj->pIndexData->vnum, eqName.c_str(), bare.c_str());
            needShort = false; // descriptions below are independent of the name
        }
        else {
            DLString adjRu(bare.substr(0, at));
            DLString nounRu(bare.substr(at + base.size()));
            adjRu.stripWhiteSpace();
            nounRu.stripWhiteSpace();

            // asString(), not asCString(): the latter throws Json::LogicError on
            // a non-string member, uncaught here, and this runs on every item
            // read rather than only at generation time.
            MultiGender gender(config["gender"].asString().c_str());
            DLString adjEn, adjUa, nounEn, nounUa;

            // A word we cannot decode must not be papered over by mirroring the
            // Russian: that writes the very leak this card exists to close, and
            // it would be permanent. Give up on the name instead -- with every
            // affix authored in all three languages and no duplicate Russian
            // form, this only fires if the affix config drops a word that
            // already went out on an item.
            if (!adjRu.empty() && !find_affix_form("adjectives", adjRu, gender, adjEn, adjUa)) {
                warn("weapon repair: obj %d uses unknown adjective '%s'.",
                     obj->pIndexData->vnum, adjRu.c_str());
                needShort = false;
            }
            else if (!nounRu.empty() && !find_affix_form("nouns", nounRu, gender, nounEn, nounUa)) {
                warn("weapon repair: obj %d uses unknown noun '%s'.",
                     obj->pIndexData->vnum, nounRu.c_str());
                needShort = false;
            }

            if (needShort && obj->getRealShortDescr(LANG_EN).empty() && config.isMember("short_en")) {
                DLString en = compose_short(adjEn, config["short_en"].asString(), nounEn);
                if (!colour.empty())
                    en = "{" + colour + en + "{x";
                obj->setShortDescr(en, LANG_EN);
                changed = true;
            }

            // Every declension is checked before any of it is written down, so a
            // sidecar that goes away mid-word leaves the slot empty for a later
            // read rather than half a name nothing will ever revisit.
            if (needShort && obj->getRealShortDescr(LANG_UA).empty() && config.isMember("short_ua")) {
                DLString gtag = gender_tag(config["gender"].asString());
                DLString baseUa, declinedAdj;
                bool declined = decline_ua(config["short_ua"].asString(), "NOUN", gtag, baseUa);

                if (declined && !adjUa.empty())
                    declined = decline_ua(adjUa, "ADJF", gtag, declinedAdj);

                if (declined) {
                    DLString ua = compose_short(declinedAdj, baseUa, nounUa);
                    if (!colour.empty())
                        ua = "{" + colour + ua + "{x";
                    obj->setShortDescr(ua, LANG_UA);
                    changed = true;
                }
            }
        }
    }

    // Long descriptions carry no affix parts and need no morphology at all --
    // they are whole authored sentences on the name entry, so an undecodable
    // name above does not stop them.
    if (obj->getRealDescription(LANG_EN).empty() && config.isMember("long_en")) {
        obj->setDescription(config["long_en"].asCString(), LANG_EN);
        changed = true;
    }

    if (obj->getRealDescription(LANG_UA).empty() && config.isMember("long_ua")) {
        obj->setDescription(config["long_ua"].asCString(), LANG_UA);
        changed = true;
    }

    return changed;
}

const WeaponGenerator & WeaponGenerator::assignColours() const
{
    DLString colour = weapon_tier_table[valTier-1].colour;

    if (obj->getProperty("eqName").empty())
        obj->setProperty("eqName", obj->getShortDescr(LANG_RU));

    // Colour each language's own name -- the tier colour wrap is language-agnostic.
    // Also covers the re-stat path, which doesn't regenerate the names.
    if (!colour.empty())
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            DLString s = obj->getShortDescr((lang_t)l);
            if (!s.empty())
                obj->setShortDescr("{" + colour + s.colourStrip() + "{x", (lang_t)l);
        }

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignAffects() const
{
    // affect_enhance merges by location and type only: two bit affects (haste and
    // protect evil, both location none) or two scoped skill-group affects would fold
    // into one and lose a bit or a scope. Only plain stats may merge.
    for (auto &af: affects) {
        Affect copy = af;
        if (copy.bitvector.getTable() != 0 || !copy.global.empty())
            affect_to_obj(obj, &copy);
        else
            affect_enhance(obj, &copy);
    }

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignTimers() const
{
    weapon_tier_t &tier = weapon_tier_table[valTier - 1];

    if (tier.weeks > 0)
        obj->timer = tier.weeks * Date::SECOND_IN_WEEK / Date::SECOND_IN_MINUTE;

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignFlags() const
{
    obj->setProperty("tier", valTier);

    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, weapon_tier_table[valTier-1].extra.getValue());
    obj->value4(weaponFlags.getValue());

    // Set weight: 0.4 kg by default in OLC, 2kg for two hand.
    // TODO: Weight is very approximate, doesn't depend on weapon type.
    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
        obj->weight = obj->pIndexData->weight * 5;

    // Set standardized cost in silver.
    if (obj->getProperty("measure_m").isNumber())
        obj->cost = item_model_cost(obj->getProperty("measure_m").toInt(), obj->level);
    else
        obj->cost = 5 * (WORST_TIER + 1 - valTier) * obj->level;
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamageType() const
{
    StringSet attacks = StringSet(wclassConfig["attacks"].asString()); // frbite, divine, etc
    StringSet damtypes = StringSet(wclassConfig["damtypes"].asString()); // bash, pierce, etc
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
    bool noMetal = rejectsMetal();

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

        // A single metal part makes the whole weapon metallic, e.g. "pine, steel".
        if (noMetal && IS_SET(type, MAT_METAL))
            continue;

        auto withType = materials_by_type(type);

        if (!withType.empty())
            materials.push_back(
                withType.at(number_range(0, withType.size() - 1))->name);
    }

    // Concatenate two or more material names, e.g. "pine, steel".
    if (!materials.empty())
        return materials.join(", ");

    if (noMetal)
        return nonMetalDefault();

    return "metal";
}

/** Default material for weapon classes with nothing configured, for players who
 *  can't wield metal: bone daggers and stone maces instead of a wooden knife.
 */
DLString WeaponGenerator::nonMetalDefault() const
{
    const Json::Value &nonmetal = wclassConfig["nonmetal"];

    if (!nonmetal.empty())
        return nonmetal[number_range(0, nonmetal.size() - 1)].asString();

    auto wooden = materials_by_type(MAT_WOOD);
    if (!wooden.empty())
        return wooden.at(number_range(0, wooden.size() - 1))->name;

    return "wood";
}

DLString random_item_compose_short(const DLString &adjective, const DLString &base, const DLString &noun)
{
    return compose_short(adjective, base, noun);
}

DLString random_item_gender_tag(const DLString &gender)
{
    return gender_tag(gender);
}

bool random_item_decline_ua(const DLString &word, const DLString &pos, const DLString &gtag, DLString &result)
{
    return decline_ua(word, pos, gtag, result);
}
