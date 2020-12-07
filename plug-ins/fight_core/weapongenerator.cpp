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
#include "material-table.h"
#include "attacks.h"
#include "loadsave.h"
#include "dl_math.h"
#include "merc.h"
#include "def.h"

GSN(none);
WEARLOC(wield);
WEARLOC(second_wield);

static int get_random_skillgroup(PCharacter *pch);

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
WeaponGenerator::WeaponGenerator(bool debug)
        : extraFlags(0, &extra_flags),
          weaponFlags(0, &weapon_type2),
          debug(debug)
{
    pch = 0;
    sn = gsn_none;
    valTier = hrTier = drTier = 5;
    hrCoef = drCoef = 0;
    hrMinValue = drMinValue = 0;
    hrIndexBonus = drIndexBonus = aveIndexBonus = 0;
    align = ALIGN_NONE;
}

WeaponGenerator::~WeaponGenerator()
{

}

WeaponGenerator & WeaponGenerator::item(Object *obj)
{ 
    this->obj = obj; 
    wclass = weapon_class.name(obj->value0());
    wclassConfig = weapon_classes[wclass];
    if (wclass.empty())
        warn("Weapon generator: no configuration defined for weapon class %s.", wclass.c_str());
     return *this; 
}

WeaponGenerator & WeaponGenerator::tier(int tier)
{
    valTier = hrTier = drTier = tier;
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
    affix_generator gen(valTier);

    // Set requirements assigned directly to the generator (most likely from the tests).
    for (auto const &reqName: required)
        gen.addRequired(reqName);

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

    gen.setPlayer(pch);
    gen.setAlign(align);
    gen.setRetainChance(50);

    // Generate all combinations of affixes.
    gen.run();
    //LogStream::sendNotice() << gen.dump();

    if (gen.getResultSize() == 0) {
        warn("Weapon generator: no affixes found for tier %d.", valTier);
        return *this;
    }    

    // Collect all configurations mandated by given set of affixes: flags, material, affects.
    auto result = gen.getSingleResult();
    int minPrice = result.front().price;
    int maxPrice = result.back().price;

    for (auto &pinfo: result) {
        const Json::Value &affix = pinfo.affix;
        const DLString &section = pinfo.section;
        if (debug) obj->carried_by->pecho("{DAffix %s [%d]", affix["value"].asCString(), pinfo.price);

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
            float mult = affix.isMember("mult") ? affix["mult"].asFloat() : 0;
            int mod = affix.isMember("mod") ? affix["mod"].asInt() : 0;
            af.modifier = mult * pinfo.stack * obj->level + mod;
            af.location = apply_flags.value(pinfo.normalizedName());
            af.type = gsn_none;
            af.duration = -1;
            af.level = obj->level;
            affects.push_back(af);

        } else if (section == "affects_with_bits") {
            Affect af;
            af.bitvector.setTable(&affect_flags);
            af.bitvector.setBits(pinfo.affixName);
            af.type = gsn_none;
            af.duration = -1;
            af.level = obj->level;
            affects.push_back(af);

        } else if (section == "skill_group") {
            Affect af;
            af.global.setRegistry(skillGroupManager);
            af.global.fromString(pinfo.affixName);
            af.modifier = affix["mod"].asInt();
            af.type = gsn_none;
            af.duration = -1;
            af.level = obj->level;
            affects.push_back(af);

        } else if (section == "player") {
            if (pinfo.affixName == "skillgroup") {
                Affect af;
                af.global.setRegistry(skillGroupManager);
                af.global.set(get_random_skillgroup(pch));
                af.modifier = affix["mod"].asInt();
                af.type = gsn_none;
                af.duration = -1;
                af.level = obj->level;
                affects.push_back(af);
            }

        } else if (section == "affect_packs") {
            for (auto const &affect: affix["affects"]) {
                Affect af;
                af.type = gsn_none;
                af.duration = -1;
                af.level = obj->level;

                if (affect.isMember("apply")) {
                    float mult = affect.isMember("mult") ? affect["mult"].asFloat() : 0;
                    int mod = affect.isMember("mod") ? affect["mod"].asInt() : 0;
                    af.modifier = mult * pinfo.stack * obj->level + mod;
                    af.location = apply_flags.value(affect["apply"].asString());
                    affects.push_back(af);

                } else if (affect.isMember("table")) {
                    af.bitvector.setTable(FlagTableRegistry::getTable(affect["table"].asString()));
                    af.bitvector.setBits(affect["bits"].asString());
                    affects.push_back(af);
                }
            }
        }

        // Each adjective or noun has a chance to be chosen, but the most expensive get an advantage.
        for (auto &adj: affix["adjectives"])
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price)
                adjectives.push_back(adj.asString());

        for (auto &noun: affix["nouns"])
            if (pinfo.price >= 0 && number_range(minPrice - 10, maxPrice) <= pinfo.price)
                nouns.push_back(noun.asString());
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

    if (debug) obj->carried_by->pecho("{DExtras %s, weapon flags %s, material %s{x", 
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
    for (auto &af: affects) {
        affect_to_obj(obj, &af);
    }
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignFlags() const
{
    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, weapon_tier_table[valTier-1].extra.getValue());
    obj->value4(obj->value4() | weaponFlags.getValue());

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
        mygroups[skill->getGroup()]++;
        totalWeight++;
        totalGroups.insert(skill->getGroup());
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