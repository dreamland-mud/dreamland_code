#include "jsoncpp/json/json.h"

#include "olc.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"
#include "argparser.h"
#include "core/object.h"
#include "affect.h"
#include "skill.h"
#include "skillreference.h"
#include "skillgroup.h"
#include "weapongenerator.h"
#include "weaponcalculator.h"
#include "weapontier.h"
#include "weaponaffixes.h"
#include "math_utils.h"
#include "comm.h"
#include "interp.h"
#include "loadsave.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

GSN(none);
GSN(bless);
GSN(calm);
GSN(frenzy);
GROUP(benedictions);
list<list<string>> random_weapon_affixes(int tier, int count, int align, int chance);

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace TAO_PEGTL_NAMESPACE::mud {
    struct MyArgs : 
        public args_level, public args_align, public args_tier, public args_wclass, public args_word {};

    struct level   : seq< one<'l'>, level_value > {};
    struct align   : seq< one<'a'>, align_value > {};
    struct tier    : seq< one<'t'>, tier_value > {};
    struct mode    : seq< one<'-'>, word_value > {};
    struct anything: sor< level, align, tier, wclass, mode > {};
    struct grammar : must< list_must< anything, spaces >, eof > {};
}

CMD(orandom, 50, "орандом", POS_DEAD, 103, LOG_ALWAYS, 
        "Random weapon generator.")
{
    vector<bitnumber_t> myclasses = {
        WEAPON_EXOTIC, WEAPON_SWORD, WEAPON_DAGGER, WEAPON_SPEAR, WEAPON_MACE,
        WEAPON_AXE, WEAPON_FLAIL, WEAPON_WHIP, WEAPON_POLEARM };

    pegtl::mud::MyArgs myargs = { -1, -1, -1, -1, "" };

    if (!parse_input<pegtl::mud::grammar, pegtl::mud::MyArgs>(ch, argument, myargs)) {
        ch->println("Формат: {Worandom{x <weapon class> {Wl{x<level> {Wt{x<tier> {Wa{x<align> [{W-{xtable|{W-{xaffix]");
        return;
    }

    // Table mode (-table argument): display all values.
    if (arg_oneof(myargs.word, "table")) {
        ostringstream buf;
        int minLevel = myargs.level == -1 ? 1 : myargs.level;
        int maxLevel = myargs.level == -1 ? MAX_LEVEL : myargs.level;
        int minTier  = myargs.tier == -1  ? BEST_TIER : myargs.tier;
        int maxTier  = myargs.tier == -1  ? WORST_TIER : myargs.tier;
        bitnumber_t minFlag  = myargs.wclass == -1 ? 0 : myargs.wclass;
        bitnumber_t maxFlag  = myargs.wclass == -1 ? WEAPON_MAX-1 : myargs.wclass;

        buf << dlprintf("{C%15s{x {WLVL  V1  V2  AVE  REAL  DR{x\r\n", "");
        for (bitnumber_t f = minFlag; f <= maxFlag; f++) {
            for (int t = minTier; t <= maxTier; t++) {
                buf << dlprintf("{CTier %1d: {y%7s{x  ", t, weapon_class.name(f).c_str());
                for (int l = minLevel; l <= maxLevel; l++) {
                    WeaponCalculator calc(t, l, f);

                    if (l != minLevel)
                        buf << dlprintf("%15s ", "");
                    buf << dlprintf("{C%3d{w  %2d  %2d  %3d   %3d  %2d\r\n", 
                        l, calc.getValue1(), calc.getValue2(), calc.getAve(), calc.getRealAve(), calc.getDamroll());
                }
            }
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    // Affix mode (-affix argument): display several possible flag combinations for given tier.
    if (arg_oneof(myargs.word, "affix", "prefix")) {
        ostringstream buf;
        int tier = myargs.tier == -1  ? number_range(BEST_TIER, WORST_TIER) : myargs.tier;
        int align = myargs.align == -1 ? ALIGN_NONE : myargs.align;
        int count = 10;
        int retainChance = 50;

        auto allNames = random_weapon_affixes(tier, count, align, retainChance);
        ch->printf("{W%d случайных комбинаций аффиксов для крутости %d и характера %d:\r\n", 
                    allNames.size(), tier, align);

        for (auto &names: allNames) {
            for (auto &name: names) {
                buf << dlprintf("%9s{x ", name.c_str());
            }
            buf << endl;
        }            

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    // Generate mode (default): create item in player inventory.

    // Assign random level, tier and weapon class unless specified in parameters.
    int level = myargs.level == -1 ? number_range(1, LEVEL_MORTAL) : myargs.level;
    int tier = myargs.tier == -1  ? number_range(BEST_TIER, WORST_TIER) : myargs.tier;
    int align = myargs.align == -1 ? ALIGN_NONE : myargs.align;
    bitnumber_t wclass = myargs.wclass == -1 ? 
        myclasses.at(number_range(0, myclasses.size() - 1)) : myargs.wclass;

    ch->printf("{WСоздаю оружие типа '%s' уровня %d и крутости %d.{x\r\n", 
        weapon_class.message(wclass).c_str(), level, tier);

    Object *obj = create_object(get_obj_index(104), 0);
    obj->value0(wclass);
    obj->level = level;
    obj->setShortDescr(str_empty); // pretend it's a restring, to allow value0-4 overrides.
    obj_to_char(obj, ch);

    WeaponGenerator()
        .item(obj)
        .player(ch)
        .tier(tier)
        .alignment(align)
        .randomNames()
        .randomAffixes()
        .assignHitroll()
        .assignDamroll()
        .assignValues()
        .assignNames()
        .assignColours()
        .assignFlags()
        .assignAffects()
        .assignDamageType();

    interpret_fmt(ch, "stat obj %lld", obj->getID());
}


/*--------------------------------------------------------------------------
 * Weapon generator test suite.
 *--------------------------------------------------------------------------*/

static bool assert_affix_excluded(const affix_generator &gen, const unordered_set<string> &affixNames)
{
    auto const &affixes = gen.getAffixes();
    auto it = find_if(affixes.begin(), affixes.end(), 
                        [affixNames](const affix_info &ai) { return affixNames.count(ai.affixName) > 0; });

    return it == affixes.end();
}

static int count_affix(const affix_generator &gen, const DLString &affixName)
{
    int cnt = 0;
    for (auto &ai: gen.getAffixes())
        if (ai.affixName == affixName)
            cnt++;

    return cnt;
}

static bool assert_affix_excluded(const affix_generator &gen, const DLString &affixName)
{
    unordered_set<string> names {affixName};
    return assert_affix_excluded(gen, names);
}

static bool assert_affix_included(const affix_generator &gen, const DLString &affixName)
{
    return !assert_affix_excluded(gen, affixName);
}

static bool assert_affix_included(const affix_generator &gen, const unordered_set<string> &affixNames)
{
    return !assert_affix_excluded(gen, affixNames);
}

static void show_title(Character *ch, const char *title)
{
    notice("TEST: %s", title);
    ptc(ch, " {C*{x %-52s    ", title);
}

static void show_result(Character *ch, bool success)
{
    if (success)
        stc("{GOK{x", ch);
    else
        stc("{RFAIL{x", ch);

    stc("\r\n", ch);
}

static void print_affixes(Character *ch, const affix_generator &gen)
{
    ptc(ch, "\r\n");
    for (const affix_info &ai: gen.getAffixes())
        ptc(ch, "...%13s [%d] x%d\r\n", ai.affixName.c_str(), ai.price, ai.stack);
}

static Object *item(Character *ch, bitnumber_t weaponClass, int level = -1)
{
    Object *obj = create_object(get_obj_index(104), 0);
    obj->value0(weaponClass);
    obj->level = level == -1 ? ch->getModifyLevel() : level;
    obj->setShortDescr(str_empty); 
    obj_to_char(obj, ch);
    return obj;
}

static bool has_affect(Object *obj, GlobalRegistryBase *registry, int regElementIndex)
{
    for (auto &af: obj->affected)
        if (af->global.getRegistry() == registry && af->global.isSet(regElementIndex))
            return true;
    return false;
}

CMD(trandom, 50, "трандом", POS_DEAD, 103, LOG_ALWAYS, 
        "Tests for the random weapon generator.")
{
    
    ch->println("Running a set of weapon generator tests:");

    {
        show_title(ch, "Players get skill group bonus from learned skills");
        PCharacter dummy;
        dummy.setLevel(100);
        dummy.setProfession("cleric");
        dummy.getSkillData(gsn_bless).learned = 100;
        dummy.getSkillData(gsn_frenzy).learned = 100;
        dummy.getSkillData(gsn_calm).learned = 100;

        Object *obj = item(ch, WEAPON_MACE);
        WeaponGenerator()
            .item(obj).tier(1).player(&dummy).addRequirement("skillgroup")
            .randomAffixes().assignAffects();
        show_result(ch, has_affect(obj, skillGroupManager, group_benedictions));
        extract_obj(obj);
    }

    {
        show_title(ch, "Polearm is always two-handed");
        Object *obj = item(ch, WEAPON_POLEARM);
        WeaponGenerator().item(obj).tier(1).randomAffixes().assignFlags();
        show_result(ch, obj->value4() | WEAPON_TWO_HANDS);
        extract_obj(obj);
    }

    {
        show_title(ch, "Dagger is never two-handed or vorpal");
        Object *obj = item(ch, WEAPON_DAGGER);
        WeaponGenerator().item(obj).tier(1).randomAffixes().assignFlags();
        show_result(ch, !IS_WEAPON_STAT(obj, WEAPON_VORPAL | WEAPON_TWO_HANDS));
        extract_obj(obj);
    }

    {
        show_title(ch, "Higher tier weapons cost more");
        Object *obj = item(ch, WEAPON_DAGGER);
        WeaponGenerator().item(obj).tier(1).randomAffixes().assignFlags();
        int cost_t1 = obj->cost;
        WeaponGenerator().item(obj).tier(3).randomAffixes().assignFlags();
        int cost_t3 = obj->cost;
        show_result(ch, cost_t1 > cost_t3);
        extract_obj(obj);
    }

    {
        show_title(ch, "AVE bonus of 1 pushes value1 to the next threshold");
        Object *obj = item(ch, WEAPON_DAGGER);
        WeaponGenerator().item(obj).valueTier(5).valueIndexBonus(1).assignValues();
        int v1 = obj->value1();
        WeaponGenerator().item(obj).valueTier(5).assignValues();
        show_result(ch, v1 > obj->value1());
        extract_obj(obj);
    }

    {
        show_title(ch, "AVE bonus of 0.5 gives value in between thresholds");
        Object *obj = item(ch, WEAPON_DAGGER, 50);
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(0.5).assignValues();
        int v1_05 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(1).assignValues();
        int v1_1 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).assignValues();
        show_result(ch, obj->value1() < v1_05 && v1_05 < v1_1);
        extract_obj(obj);
    }

    {
        show_title(ch, "AVE bonus of -0.5 gives value in between thresholds");
        Object *obj = item(ch, WEAPON_DAGGER, 40);
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(-0.5).assignValues();
        int v1_05 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(-1).assignValues();
        int v1_1 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).assignValues();
        show_result(ch, v1_1 < v1_05 && v1_05 < obj->value1());
        extract_obj(obj);
    }

    {
        show_title(ch, "AVE bonus of -1.5 gives value in between thresholds");
        Object *obj = item(ch, WEAPON_DAGGER, 50);
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(-1.5).assignValues();
        int v1_15 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).valueIndexBonus(-2).assignValues();
        int v1_2 = obj->value1();
        WeaponGenerator().item(obj).valueTier(1).assignValues();
        show_result(ch, v1_2 < v1_15 && v1_15 < obj->value1());
        extract_obj(obj);
    }

    {
        show_title(ch, "AVE bonus of -1 pulls value1 down to prev threshold");
        Object *obj = item(ch, WEAPON_DAGGER);
        WeaponGenerator().item(obj).valueTier(5).valueIndexBonus(-1).assignValues();
        int v1 = obj->value1();
        WeaponGenerator().item(obj).valueTier(5).assignValues();
        show_result(ch, v1 < obj->value1());
        extract_obj(obj);
    }

    {
        show_title(ch, "HR bonus of 3 still works on LVL 100");
        Object *obj = item(ch, WEAPON_DAGGER);
        WeaponGenerator().item(obj).tier(1).hitrollIndexBonus(3).assignHitroll();
        int hr3 = obj->affected.find(gsn_none, APPLY_HITROLL)->modifier;
        WeaponGenerator().item(obj).tier(1).assignHitroll();
        int hr = obj->affected.find(gsn_none, APPLY_HITROLL)->modifier;
        show_result(ch, hr3 > hr);
        extract_obj(obj);
    }

    {
        show_title(ch, "DR bonus of -1.5 gives value in between thresholds");
        Object *obj = item(ch, WEAPON_DAGGER, 80);
        WeaponGenerator().item(obj).damrollTier(1).damrollIndexBonus(-1.5).assignDamroll();
        int dr_15 = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        WeaponGenerator().item(obj).damrollTier(1).damrollIndexBonus(-2).assignDamroll();
        int dr_2 = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        WeaponGenerator().item(obj).damrollTier(1).assignDamroll();
        int dr = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        show_result(ch, dr_2 < dr_15 && dr_15 < dr);
        extract_obj(obj);
    }

    {
        show_title(ch, "DR bonus of 0.5 gives value in between thresholds");
        Object *obj = item(ch, WEAPON_DAGGER, 80);
        WeaponGenerator().item(obj).damrollTier(1).damrollIndexBonus(0.5).assignDamroll();
        int dr_05 = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        WeaponGenerator().item(obj).damrollTier(1).damrollIndexBonus(1).assignDamroll();
        int dr_1 = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        WeaponGenerator().item(obj).damrollTier(1).assignDamroll();
        int dr = obj->affected.find(gsn_none, APPLY_DAMROLL)->modifier;
        show_result(ch, dr < dr_05 && dr_05 < dr_1);
        extract_obj(obj);
    }

    ch->println("\r\nRunning a set of affix generator tests:");

    {
        show_title(ch, "Holy affix not selected for align < 350");
        affix_generator gen(1);
        gen.setAlign(349);
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "holy"));
    }

    {
        show_title(ch, "Holy affix selected for align >= 350");
        affix_generator gen(1);
        gen.setAlign(350);
        gen.setup();
        show_result(ch, assert_affix_included(gen, "holy"));
    }

    {
        show_title(ch, "Flaming conflicts with frost, fading, wood, ice");
        affix_generator gen(1);
        gen.addRequired("flaming");
        gen.setup();
        unordered_set<string> conflicts {"frost", "fading", "wood", "ice"};
        show_result(ch, assert_affix_excluded(gen, conflicts));
    }

    {
        show_title(ch, "Frost conflicts with flaming (indirect reference)");
        affix_generator gen(1);
        gen.addRequired("frost");
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "flaming"));
    }

    {
        show_title(ch, "Wood conflicts with all other materials");
        affix_generator gen(1);
        gen.addRequired("wood");
        gen.setup();
        unordered_set<string> conflicts {"platinum", "titanium", "ice"};
        show_result(ch, assert_affix_excluded(gen, conflicts));
    }

    {
        show_title(ch, "Shocking not selected for tier > 2");
        affix_generator gen(3);
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "shocking"));
    }

    {
        show_title(ch, "Shocking selected for tier <= 2");
        affix_generator gen(2);
        gen.setup();
        show_result(ch, assert_affix_included(gen, "shocking"));
    }

    {
        show_title(ch, "Align-restricted are kept for no align");
        affix_generator gen(1);
        gen.setup();
        unordered_set<string> withAlign {"vorpal", "fading", "holy", "vampiric", "evil"};
        show_result(ch, assert_affix_included(gen, withAlign));
    }

    {
        show_title(ch, "Forbidden affixes not included");
        affix_generator gen(5);
        gen.addForbidden("two_hands");
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "two_hands"));
    }

    {
        show_title(ch, "Preferred affixes are always kept");
        affix_generator gen(1);
        unordered_set<string> mine {"vorpal", "fading", "holy", "vampiric", "evil"};
        for (auto &affix: mine)
            gen.addPreference(affix);
        gen.setRetainChance(0);
        gen.setup();
        show_result(ch, assert_affix_included(gen, mine) && gen.getAffixes().size() == mine.size());
    }

    {
        show_title(ch, "Vorpal, holy are kept for good align");
        unordered_set<string> good {"vorpal", "holy"};
        unordered_set<string> evil {"vampiric", "evil"};
        affix_generator gen(1);
        gen.setRetainChance(0);
        gen.setAlign(1000);
        gen.setup();
        show_result(ch, assert_affix_included(gen, good) 
                        && assert_affix_excluded(gen, evil));
    }

    {
        show_title(ch, "Vorpal, holy are ignored for no align");
        unordered_set<string> good {"vorpal", "holy"};
        affix_generator gen(1);
        gen.setRetainChance(0);
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, good));
    }

    {
        show_title(ch, "Two_hands requirement is kept");
        affix_generator gen(1);
        gen.addRequired("two_hands");
        gen.addForbidden("flaming");
        gen.addPreference("vorpal");
        gen.addPreference("sharp");
        gen.setRetainChance(0);
        gen.setup();
        show_result(ch, assert_affix_included(gen, "two_hands"));
    }

    {
        show_title(ch, "HR included more than once");
        affix_generator gen(1);
        gen.setRetainChance(100);
        gen.setup();
        show_result(ch, count_affix(gen, "hr") == 3 && count_affix(gen, "-hr") == 3);
    }

    {
        show_title(ch, "Player-specific affixes not included by default");
        affix_generator gen(1);
        gen.setRetainChance(100);
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "skillgroup"));
    }

    {
        show_title(ch, "Player-specific affixes included for a player");
        PCharacter dummy;
        affix_generator gen(1);
        gen.setRetainChance(100);
        gen.setPlayer(&dummy);
        gen.setup();
        show_result(ch, assert_affix_included(gen, "skillgroup"));
    }

    {
        show_title(ch, "Required player affixes excluded by default");
        affix_generator gen(1);
        gen.addRequired("skillgroup");
        gen.setRetainChance(100);
        gen.setup();
        show_result(ch, assert_affix_excluded(gen, "skillgroup"));
    }

}
