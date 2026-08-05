#include <algorithm>

#include "weapongenerator.h"
#include "weaponcalculator.h"
#include "weapontier.h"
#include "weaponaffixes.h"

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

static int get_random_skillgroup(PCharacter *pch);

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
    retainChance = 50;
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
    wclass = allClasses[random_index];
    wclassConfig = weapon_classes[wclass];
    obj->value0(weapon_class.value(wclass));

    // Keep some extra flags (e.g. for shops) but clean everything else.
    obj->extra_flags &= ITEM_INVENTORY;

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignValues() const
{    
    WeaponCalculator calc(valTier, obj->level, obj->value0(), aveIndexBonus);
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
        if (!noMetal || !name_is_metal_only(configs[i]))
            allowed.push_back(i);

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

WeaponGenerator & WeaponGenerator::randomAffixes()
{
    affix_generator gen(valTier);

    // Set requirements and restrictions assigned directly to the generator.
    for (auto const &reqName: required)
        gen.addRequired(reqName);
    for (auto const &fbdName: forbidden)
        gen.addForbidden(fbdName);

    // Set exclusions or requirements based on chosen names and weapon flags.
    for (auto const &affixName: wclassConfig["forbids"])
        gen.addForbidden(affixName.asString());

    for (auto const &affixName: wclassConfig["requires"])
        gen.addRequired(affixName.asString());

    for (auto const &affixName: wclassConfig["prefers"])
        gen.addPreference(affixName.asString());

    for (auto const &affixName: nameConfig["forbids"])
        gen.addForbidden(affixName.asString());

    for (auto const &affixName: nameConfig["requires"])
        gen.addRequired(affixName.asString());

    for (auto const &affixName: nameConfig["prefers"])
        gen.addPreference(affixName.asString());

    // Material affixes that turn the weapon into a metal one are pointless
    // for a player who would never be able to wield it.
    if (rejectsMetal()) {
        gen.addForbidden("platinum");
        gen.addForbidden("titanium");
    }

    // Exclude hr/dr affixes that don't have non-zero values at this level and tier.
    if (maxHitroll() <= 0) {
        gen.addForbidden("hr");
        gen.addForbidden("-hr");
    }

    if (maxDamroll() <= 0) {
        gen.addForbidden("dr");
        gen.addForbidden("-dr");
    }

    gen.setPlayer(pch);
    gen.setAlign(align);
    gen.setRetainChance(retainChance);

    // Generate all combinations of affixes.
    gen.run();
//    LogStream::sendNotice() << gen.dump();

    if (gen.getResultSize() == 0) {
        warn("Weapon generator: no affixes found for tier %d.", valTier);
        return *this;
    }    

    // Collect all configurations mandated by given set of affixes: flags, material, affects.
    auto result = gen.getSingleResult();
    int minPrice = result.front().price;
    int maxPrice = result.back().price;
    StringSet affixNames;

    for (auto &pinfo: result) {
        const Json::Value &affix = pinfo.affix;
        const DLString &section = pinfo.section;
        affixNames.insert(affix["value"].asCString());

        extraFlags.setBits(affix["extra"].asString());

        if (section == "flag") {
            weaponFlags.setBits(pinfo.affixName);

        } else if (section == "extra") {
            extraFlags.setBits(pinfo.affixName);

        } else if (section == "material") {
            materialName = pinfo.affixName;

        } else if (section == "affects_by_tier") {
            float bonus = affix["step"].asFloat() * pinfo.stack;
            DLString aname = pinfo.normalizedName();

            if (aname == "hr")
                hrIndexBonus += bonus;
            else if (aname == "dr")
                drIndexBonus += bonus;
            else if (aname == "ave") 
                aveIndexBonus += bonus;

        } else if (section == "affects_by_level") {
            Affect af;
            af.modifier = calcAffectModifier(affix, pinfo);
            af.location = apply_flags.value(pinfo.normalizedName());
            rememberAffect(af);

        } else if (section == "affects_with_bits") {
            Affect af;
            af.bitvector.setTable(&affect_flags);
            af.bitvector.setBits(pinfo.affixName);
            rememberAffect(af);

        } else if (section == "skill_group") {
            Affect af;
            af.global.setRegistry(skillGroupManager);
            af.global.fromString(pinfo.affixName);
            af.modifier = calcAffectModifier(affix, pinfo);
            rememberAffect(af);

        } else if (section == "player") {
            if (pinfo.affixName == "skillgroup") {
                Affect af;
                af.global.setRegistry(skillGroupManager);
                af.global.set(get_random_skillgroup(pch));
                af.modifier = calcAffectModifier(affix, pinfo);
                rememberAffect(af);
            }

        } else if (section == "affect_packs") {
            for (auto const &affect: affix["affects"]) {
                Affect af;

                if (affect.isMember("apply")) {
                    af.modifier = calcAffectModifier(affect, pinfo);
                    af.location = apply_flags.value(affect["apply"].asString());
                    rememberAffect(af);

                } else if (affect.isMember("table")) {
                    af.bitvector.setTable(FlagTableRegistry::getTable(affect["table"].asString()));
                    af.bitvector.setBits(affect["bits"].asString());
                    rememberAffect(af);
                }
            }
        }

        // Each adjective or noun has a chance to be chosen, but the most expensive get an advantage.
        // Push the parallel EN/UA forms under the SAME roll so the vectors stay index-aligned.
        const Json::Value &adjEn = affix["adjectives_en"], &adjUa = affix["adjectives_ua"];
        for (Json::ArrayIndex k = 0; k < affix["adjectives"].size(); k++)
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price) {
                adjectives.push_back(affix["adjectives"][k].asString());
                adjectives_en.push_back(k < adjEn.size() ? DLString(adjEn[k].asString()) : DLString::emptyString);
                adjectives_ua.push_back(k < adjUa.size() ? DLString(adjUa[k].asString()) : DLString::emptyString);
            }

        const Json::Value &nounEn = affix["nouns_en"], &nounUa = affix["nouns_ua"];
        for (Json::ArrayIndex k = 0; k < affix["nouns"].size(); k++)
            if (pinfo.price >= 0 && number_range(minPrice - 10, maxPrice) <= pinfo.price) {
                nouns.push_back(affix["nouns"][k].asString());
                nouns_en.push_back(k < nounEn.size() ? DLString(nounEn[k].asString()) : DLString::emptyString);
                nouns_ua.push_back(k < nounUa.size() ? DLString(nounUa[k].asString()) : DLString::emptyString);
            }
    }

    // Additional flags configured for weapon class. 
    for (auto const &flag: wclassConfig["flags"].getMemberNames()) {
        int prob = wclassConfig["flags"][flag].asInt();
        if (chance(prob))
            weaponFlags.setBits(flag);
    }

    // Improve ave for two-handed weapons.
    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
        aveIndexBonus++;

    // Remember affixes choice on the item.
    obj->setProperty("affixes", affixNames.toString());

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
    randomWeaponClass()
        .randomNames()
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

/** Guess affect modifier from json config as (mult * level * stack + mod). */
int WeaponGenerator::calcAffectModifier(const Json::Value &afConfig, const affix_info &info) const
{
    float mult = afConfig.isMember("mult") ? afConfig["mult"].asFloat() : 0;
    int mod = afConfig.isMember("mod") ? afConfig["mod"].asInt() : 0;
    int result = mult * info.stack * obj->level + mod;

    if (result != 0)
        return result;
    else
        return signum(mult) * 1; // return a minimum of +1/-1 when level is too small
}

void WeaponGenerator::setName() const
{
    StringList mynames(nameConfig["name"].asString());
    mynames.addUnique(wclass);
    mynames.addUnique(weapon_class.message(obj->value0()));
    obj->setKeyword(mynames.join(" ").c_str());
}

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
    DLString myshort;
    if (a >= 0)
        myshort += Morphology::adjective(adjectives[a], obj->gram_gender) + " "; // леденящий
    myshort += nameConfig["short"].asString(); // буздыган
    if (n >= 0)
        myshort += " " + nouns[n]; // боли

    obj->setShortDescr(myshort, LANG_RU);
    obj->setProperty("eqName", nameConfig["short"].asString()); // 'буздыган' in sheath wearloc

    // --- English: plain per-language forms if authored, else mirror RU ---
    if (nameConfig.isMember("short_en")) {
        DLString en;
        if (a >= 0 && a < (int)adjectives_en.size() && !adjectives_en[a].empty())
            en += adjectives_en[a] + " ";
        en += nameConfig["short_en"].asString();
        if (n >= 0 && n < (int)nouns_en.size() && !nouns_en[n].empty())
            en += " " + nouns_en[n];
        obj->setShortDescr(en, LANG_EN);
    } else {
        obj->setShortDescr(myshort, LANG_EN);
    }

    // --- Ukrainian: decline nominative forms via the sidecar, else mirror RU ---
    if (nameConfig.isMember("short_ua")) {
        DLString g = nameConfig["gender"].asString();
        DLString gtag = g == "m" ? "masc" : g == "f" ? "femn" : g == "n" ? "neut" : "-";

        DLString ua;
        if (a >= 0 && a < (int)adjectives_ua.size() && !adjectives_ua[a].empty())
            ua += Morphology::declineUa(adjectives_ua[a], "ADJF", gtag) + " ";
        ua += Morphology::declineUa(nameConfig["short_ua"].asString(), "NOUN", gtag);
        // Suffix nouns ("... of pain") are a fixed genitive -- appended as-is,
        // like RU, so they stay put when the weapon name declines by case.
        if (n >= 0 && n < (int)nouns_ua.size() && !nouns_ua[n].empty())
            ua += " " + nouns_ua[n];
        obj->setShortDescr(ua, LANG_UA);
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
            obj->setShortDescr("{" + colour + s.colourStrip() + "{x", (lang_t)l);
        }

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignAffects() const
{
    for (auto &af: affects) {
        affect_enhance(obj, &af);
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

// Helper function to get most popular/learned skill group for a player.
static int get_random_skillgroup(PCharacter *pch)
{
    GlobalArray mygroups(skillGroupManager);
    set<int> totalGroups;
    int totalWeight = 0;

    for (int sn = 0; sn < skillManager->size(); sn++) {
        PCSkillData &myskill = pch->getSkillData(sn);

        if (myskill.learned <= 1)
            continue;
        if (myskill.isTemporary())
            continue;

        Skill *skill = skillManager->find(sn);
        vector<int> groups = skill->getGroups().toArray();
        for (auto g: groups) {
            mygroups[g]++;
            totalGroups.insert(g);
        }

        totalWeight++;
    }

    int currentWeight = 0;
    int dice = number_range(0, totalWeight - 1);
    for (auto &group: totalGroups) {
        currentWeight += mygroups[group];
        if (currentWeight > dice)
            return group;
    }
        
    return -1;
}
