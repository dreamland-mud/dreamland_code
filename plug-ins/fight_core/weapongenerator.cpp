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
    // weaponClass() has already pinned and applied the class; don't roll over it.
    if (!wclassFixed)
        randomWeaponClass();

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
    return "-";
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
    DLString myshort = compose_short(
        a >= 0 ? Morphology::adjective(adjectives[a], obj->gram_gender) : DLString::emptyString, // леденящий
        nameConfig["short"].asString(),                                                          // буздыган
        n >= 0 ? nouns[n] : DLString::emptyString);                                              // боли

    obj->setShortDescr(myshort, LANG_RU);
    obj->setProperty("eqName", nameConfig["short"].asString()); // 'буздыган' in sheath wearloc

    // --- English: plain per-language forms if authored, else mirror RU ---
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

        obj->setShortDescr(compose_short(
            adjUa.empty() ? adjUa : Morphology::declineUa(adjUa, "ADJF", gtag),
            Morphology::declineUa(nameConfig["short_ua"].asString(), "NOUN", gtag),
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

/** Morphology::declineUa answers "<word>|||||" -- a pad carrying no case endings
 *  at all -- whenever the pymorphy3 sidecar is unreachable, and the repair below
 *  writes whatever it is handed into a saved field. A weapon repaired while the
 *  sidecar is down would keep that undeclined name forever, so probe with a word
 *  known to decline and leave Ukrainian alone until the sidecar answers. Failed
 *  lookups are never cached (morphology.cpp returns the fallback before it
 *  touches the cache), so a later read picks the repair up by itself. */
static bool ua_morphology_answers()
{
    DLString probe = "меч";
    return Morphology::declineUa(probe, "NOUN", "masc") != probe + "|||||";
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

        // Take the tier colour from what the Russian name actually wears, not
        // from the table: a tier re-tuned since this weapon was made would
        // otherwise paint the new languages a colour the Russian does not have.
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

            MultiGender gender(config["gender"].asCString());
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

            // Probed only here, where a wrong answer would be written down.
            if (needShort && obj->getRealShortDescr(LANG_UA).empty()
                          && config.isMember("short_ua") && ua_morphology_answers()) {
                DLString gtag = gender_tag(config["gender"].asString());
                DLString ua = compose_short(
                    adjUa.empty() ? adjUa : Morphology::declineUa(adjUa, "ADJF", gtag),
                    Morphology::declineUa(config["short_ua"].asString(), "NOUN", gtag),
                    nounUa);
                if (!colour.empty())
                    ua = "{" + colour + ua + "{x";
                obj->setShortDescr(ua, LANG_UA);
                changed = true;
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
