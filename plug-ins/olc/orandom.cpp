#include "olc.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"
#include "argparser.h"
#include "core/object.h"
#include "weapons.h"
#include "math_utils.h"
#include "comm.h"
#include "interp.h"
#include "loadsave.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace TAO_PEGTL_NAMESPACE::mud {
    struct MyArgs : public args_level, public args_tier, public args_wclass {};

    struct level   : seq< one<'l'>, level_value > {};
    struct tier    : seq< one<'t'>, tier_value > {};
    struct anything: sor< level, tier, wclass > {};
    struct grammar : must< list_must< anything, spaces >, eof > {};
}

CMD(orandom, 50, "орандом", POS_DEAD, 103, LOG_ALWAYS, 
        "Random weapon generator.")
{
    vector<bitnumber_t> myclasses = {
        WEAPON_EXOTIC, WEAPON_SWORD, WEAPON_DAGGER, WEAPON_SPEAR, WEAPON_MACE,
        WEAPON_AXE, WEAPON_FLAIL, WEAPON_WHIP, WEAPON_POLEARM };

    pegtl::mud::MyArgs myargs = { -1, -1, -1 };

    if (!parse_input<pegtl::mud::grammar, pegtl::mud::MyArgs>(ch, argument, myargs)) {
        ch->println("Формат: {Worandom{x <weapon class> {Wl{x<level> {Wt{x<tier>");
        return;
    }

    // Assign random level, tier and weapon class unless specified in parameters.
    int level = myargs.level == -1 ? number_range(1, LEVEL_MORTAL) : myargs.level;
    int tier = myargs.tier == -1  ? number_range(1, 5) : myargs.tier;
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
        .hitrollTier(tier)
        .damrollTier(tier)
        .valueTier(tier)
        .assignHitroll()
        .assignDamroll()
        .assignValues()
        .assignNames()
        .assignDamageType();

    interpret_fmt(ch, "stat obj %lld", obj->getID());
}

