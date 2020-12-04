#include "jsoncpp/json/json.h"

#include "olc.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"
#include "argparser.h"
#include "core/object.h"
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

list<list<string>> random_weapon_affixes(int tier, int count, int align);

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
        int count = 50;

        auto allNames = random_weapon_affixes(tier, count, align);
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
        .alignment(align)
        .hitrollTier(tier)
        .damrollTier(tier)
        .valueTier(tier)
        .randomNames()
        .randomAffixes()
        .assignHitroll()
        .assignDamroll()
        .assignValues()
        .assignNames()
        .assignFlags()
        .assignAffects()
        .assignDamageType();

    interpret_fmt(ch, "stat obj %lld", obj->getID());
}

