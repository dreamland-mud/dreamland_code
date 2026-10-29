#include <algorithm>
#include <cmath>
#include <map>
#include <set>

#include "magicitemgenerator.h"
#include "weapongenerator.h"
#include "weapontier.h"
#include "itemmodel.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "core/object.h"
#include "character.h"
#include "room.h"
#include "area.h"
#include "itemevents.h"
#include "npcharacter.h"
#include "occupations.h"
#include "chargedprops.h"
#include "configurable.h"
#include "date.h"
#include "dl_math.h"
#include "merc.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * Configs
 *---------------------------------------------------------------------------*/
static Json::Value magic_items;
// Every read goes through the const view: a non-const operator[] would insert
// null members for missing keys into the shared config.
static const Json::Value &cfg = magic_items;
static std::set<DLString> autocastBlocked;

CONFIGURABLE_LOADED(fight, magic_items)
{
    magic_items = value;
    autocastBlocked.clear();

    const char *keys[] = { "blocked", "autocast_blocked" };
    for (const char *key: keys) {
        const Json::Value &list = value[key];
        if (!list.isArray( ))
            continue;

        for (const auto &name: list)
            if (name.isString( ))
                autocastBlocked.insert( name.asString( ) );
    }
}

// Only what the generator needs from the spell price table: one charge in M,
// and Kit's per-spell exclusion from magic items.
static std::map<DLString, double> spellCharge;
static std::set<DLString> spellExcluded;

CONFIGURABLE_LOADED(fight, spell_value)
{
    spellCharge.clear();
    spellExcluded.clear();

    const Json::Value &spells = value["spells"];
    if (!spells.isObject( ))
        return;

    for (auto i = spells.begin( ); i != spells.end( ); ++i) {
        DLString name = i.key( ).asString( );
        const Json::Value &s = *i;
        if (s["charge"].isNumeric( ))
            spellCharge[name] = s["charge"].asDouble( );
        if (s["excluded"].isBool( ) && s["excluded"].asBool( ))
            spellExcluded.insert( name );
    }
}

const Json::Value & magic_items_config()
{
    return magic_items;
}

bool magic_item_autocast_blocked(const DLString &spell)
{
    return autocastBlocked.count( spell ) > 0;
}

double magic_spell_charge(const DLString &spell)
{
    auto i = spellCharge.find( spell );
    return i == spellCharge.end( ) ? -1 : i->second;
}

// A family stands for its kinds: "scroll" is scroll_buff or scroll_combat.
static DLString resolve_kind(const DLString &kind)
{
    const Json::Value &kinds = cfg["kinds"];

    if (kinds.isMember( kind ))
        return kind;

    StringList family;
    for (auto const &k: kinds.getMemberNames( ))
        if (kinds[k]["item_type"].asString( ) == kind)
            family.push_back( k );

    if (family.empty( ))
        return DLString::emptyString;

    auto it = family.begin( );
    std::advance( it, number_range( 0, family.size( ) - 1 ) );
    return *it;
}

bool magic_item_kind_exists(const DLString &kind)
{
    try {
        return !resolve_kind( kind ).empty( );
    }
    catch (const std::exception &e) {
        return false;
    }
}

/*----------------------------------------------------------------------------
 * MagicItemGenerator
 *---------------------------------------------------------------------------*/
// Name ladder and max_spells run common..epic; legendary shares the top step.
static int tier_step(int tier)
{
    return URANGE(0, WORST_TIER - tier, 3);
}

static bool kind_is_charged(const Json::Value &kc)
{
    return kc["item_type"].asString( ) == "wand";
}

MagicItemGenerator::MagicItemGenerator(Object *obj, Character *ch, int tier, const DLString &kind)
    : obj(obj), ch(ch), tier(tier), kind(kind),
      align(0), hasAlign(false), chosenTotal(0), kindConfig(0), tierConfig(0)
{
}

MagicItemGenerator & MagicItemGenerator::level(int level)
{
    obj->level = URANGE(1, level, MAX_LEVEL);
    return *this;
}

MagicItemGenerator & MagicItemGenerator::alignment(int align)
{
    this->align = align;
    hasAlign = true;
    return *this;
}

bool MagicItemGenerator::collect(std::vector<Pick> &cands) const
{
    const Json::Value &kc = *kindConfig;
    const Json::Value &reach = cfg["level_reach"];
    int level = obj->level;
    int maxLevel = level + (level <= reach["low_level_max"].asInt( )
                                ? reach["low"].asInt( ) : reach["high"].asInt( ));

    DLString aoe = kc["aoe"].asString( );
    std::set<DLString> groupOk;
    for (auto const &s: cfg["group_ok"])
        groupOk.insert( s.asString( ) );

    // Top spells: one per item, only from the kind's min_tier up, and of the
    // sanctuary family only the one that suits the owner's alignment.
    const Json::Value &top = cfg["top_spells"];
    std::set<DLString> topSpells, alignSpells;
    for (auto const &s: top["spells"])
        topSpells.insert( s.asString( ) );
    for (auto const &a: top["by_align"].getMemberNames( ))
        alignSpells.insert( top["by_align"][a].asString( ) );

    DLString alignKey = "neutral";
    int a = hasAlign ? align : (ch ? (int)ch->alignment : 0);
    if (a >= 350)
        alignKey = "good";
    else if (a <= -350)
        alignKey = "evil";
    DLString mySanctuary = top["by_align"][alignKey].asString( );

    int topTier = 0;
    DLString minTier = top["min_tier"][kind].asString( );
    for (auto const &t: cfg["tiers"].getMemberNames( ))
        if (cfg["tiers"][t]["name"].asString( ) == minTier)
            topTier = atoi( t.c_str( ) );

    for (auto const &p: kc["pool"]) {
        DLString name = p["spell"].asString( );

        if (p["level"].asInt( ) > maxLevel)
            continue;
        if (spellExcluded.count( name ))
            continue;

        if (p["aoe"].asBool( )) {
            if (aoe == "never")
                continue;
            if (aoe == "group_only" && !groupOk.count( name ))
                continue;
            if (aoe == "rare+" && tier > 3)
                continue;
        }

        if (topSpells.count( name )) {
            if (topTier == 0 || tier > topTier)
                continue;
            if (alignSpells.count( name ) && name != mySanctuary)
                continue;
        }

        double price = magic_spell_charge( name );
        if (price <= 0)
            continue;

        Skill *skill = skillManager->findExisting( name );
        if (!skill || !skill->getSpell( ))
            continue;

        cands.push_back( Pick{ name, skill->getIndex( ), price } );
    }

    return !cands.empty( );
}

static bool is_top(const DLString &spell)
{
    for (auto const &s: cfg["top_spells"]["spells"])
        if (s.asString( ) == spell)
            return true;
    return false;
}

template <typename Elem>
static void shuffle_picks(std::vector<Elem> &v)
{
    for (int i = (int)v.size( ) - 1; i > 0; i--)
        std::swap( v[i], v[number_range( 0, i )] );
}

// Distance of a sum from the [lo, hi] window, 0 inside it.
static double window_miss(double sum, double lo, double hi)
{
    return sum < lo ? lo - sum : (sum > hi ? sum - hi : 0);
}

/** Up to max_spells distinct spells whose prices sum into the window. A tier the
 *  pool cannot reach keeps the closest fill found. */
bool MagicItemGenerator::pickConsumable(const std::vector<Pick> &cands, std::vector<Pick> &picks) const
{
    const Json::Value &tc = *tierConfig;
    double lo = tc["min_m"].asDouble( );
    double hi = tc["max_m"].asDouble( ) * (100 + cfg["spread_cap_pct"].asInt( )) / 100.0;
    int maxSpells = (*kindConfig)["max_spells"][tier_step( tier )].asInt( );
    double bestMiss = -1;

    for (int attempt = 0; attempt < 50; attempt++) {
        std::vector<Pick> pool = cands, fill;
        shuffle_picks( pool );

        // A random target inside the window, so tiers roll both one strong
        // spell and several weak ones.
        double target = lo + (hi - lo) * number_range( 0, 100 ) / 100.0;
        double sum = 0;
        bool top = false;

        for (auto &p: pool) {
            if ((int)fill.size( ) >= maxSpells || sum >= target)
                break;
            if (sum + p.price > hi)
                continue;
            if (is_top( p.spell )) {
                if (top)
                    continue;
                top = true;
            }
            fill.push_back( p );
            sum += p.price;
        }

        if (fill.empty( ))
            continue;

        double miss = window_miss( sum, lo, hi );
        if (bestMiss < 0 || miss < bestMiss) {
            bestMiss = miss;
            picks = fill;
        }

        if (miss == 0)
            break;
    }

    return !picks.empty( );
}

/** One spell and a charge count that puts charge x charges into the window,
 *  inside the tier's wand_charges range. */
bool MagicItemGenerator::pickCharged(const std::vector<Pick> &cands, Pick &pick, int &charges) const
{
    const Json::Value &tc = *tierConfig;
    double lo = tc["min_m"].asDouble( );
    double hi = tc["max_m"].asDouble( ) * (100 + cfg["spread_cap_pct"].asInt( )) / 100.0;
    int cmin = tc["wand_charges"][0].asInt( );
    int cmax = std::min( tc["wand_charges"][1].asInt( ), cfg["max_charges"].asInt( ) );
    double bestMiss = -1;

    std::vector<Pick> pool = cands;
    shuffle_picks( pool );

    for (auto &p: pool) {
        int fit_lo = std::max( cmin, (int)ceil( lo / p.price ) );
        int fit_hi = std::min( cmax, (int)floor( hi / p.price ) );

        if (fit_lo <= fit_hi) {
            pick = p;
            charges = number_range( fit_lo, fit_hi );
            return true;
        }

        int c = URANGE(cmin, (int)round( lo / p.price ), cmax);
        double miss = window_miss( p.price * c, lo, hi );
        if (bestMiss < 0 || miss < bestMiss) {
            bestMiss = miss;
            pick = p;
            charges = c;
        }
    }

    return bestMiss >= 0;
}

bool MagicItemGenerator::run()
{
    // Generator data is hand-editable: a wrong-typed field throws from jsoncpp,
    // and on the reset path an uncaught throw would take the server down.
    try {
        return roll( );
    }
    catch (const std::exception &e) {
        LogStream::sendError( ) << "MagicItemGenerator: kind '" << kind << "': " << e.what( ) << endl;
        return false;
    }
}

bool MagicItemGenerator::roll()
{
    kind = resolve_kind( kind );
    if (kind.empty( ))
        return false;

    kindConfig = &cfg["kinds"][kind];
    DLString itemType = (*kindConfig)["item_type"].asString( );

    // Staves are weapons: the weapon generator rolls them (P2b).
    if (itemType == "staff")
        return false;

    // Legacy wands and staves cast from their values, not from props.
    if (obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF)
        return false;

    // Names are data: refuse before touching the item rather than leave it unnamed.
    const Json::Value &ladder = cfg["names"][itemType];
    if (!ladder.isArray( ) || ladder.size( ) == 0 || !ladder[0]["short"].isString( ))
        return false;

    bool charged = kind_is_charged( *kindConfig );
    int rolled = URANGE(BEST_TIER, tier, WORST_TIER);

    // There are no legendary consumables: the best one is epic.
    if (!charged && rolled == BEST_TIER)
        rolled = BEST_TIER + 1;

    if (obj->level < 1)
        obj->level = 1;

    // A pool that cannot fill the rolled tier's window rolls again a tier lower,
    // under that tier's own rules (top spells, AoE, spell count), so the colour
    // and the name never promise more than the item holds.
    std::vector<Pick> chosen;
    int charges = 0;
    double sum = 0;
    int foundTier = 0;
    const Json::Value *foundConfig = 0;

    for (int t = rolled; t <= WORST_TIER && t <= (int)weapon_tier_table.size( ); t++) {
        const Json::Value &tc = cfg["tiers"][DLString( t ).c_str( )];
        if (!tc.isObject( ))
            continue;

        tier = t;
        tierConfig = &tc;

        std::vector<Pick> cands, fill;
        int c = 0;
        double s = 0;

        if (!collect( cands ))
            continue;

        if (charged) {
            Pick pick;
            if (!pickCharged( cands, pick, c ))
                continue;
            fill.push_back( pick );
            s = pick.price * c;
        }
        else {
            if (!pickConsumable( cands, fill ))
                continue;
            for (auto &p: fill)
                s += p.price;
        }

        chosen = fill;
        charges = c;
        sum = s;
        foundTier = t;
        foundConfig = &tc;

        if (sum >= tc["min_m"].asDouble( ))
            break;
    }

    if (!foundConfig)
        return false;

    tier = foundTier;
    tierConfig = foundConfig;

    int castLevel = std::max( 1, obj->level + (*tierConfig)["cast_offset"].asInt( ) );

    if (charged) {
        Json::Value c;
        c["spell"] = chosen.front( ).spell;
        c["level"] = castLevel;
        c["charges"] = charges;
        c["max"] = charges;
        c["where"] = "carried";
        c["crumble"] = true;
        obj->props["charged"] = c;
    }
    else {
        if (itemType == "pill")
            obj->item_type = ITEM_PILL;
        else if (itemType == "potion")
            obj->item_type = ITEM_POTION;
        else
            obj->item_type = ITEM_SCROLL;

        // A consumable casts from its values; a charged copy would steal 'use'.
        if (obj->props.isObject( ))
            obj->props.removeMember( "charged" );

        // Empty slots hold the "none" skill (0), like every prototype.
        obj->value0( castLevel );
        for (int i = 1; i <= 4; i++)
            obj->valueByIndex( i, i <= (int)chosen.size( ) ? chosen[i - 1].sn : 0 );
    }

    chosenTotal = (int)round( sum * 100 );

    // The strongest spell names the item.
    std::stable_sort( chosen.begin( ), chosen.end( ),
                      [](const Pick &a, const Pick &b) { return a.price > b.price; } );
    assignNames( chosen );
    assignFlags( );
    return true;
}

void MagicItemGenerator::assignNames(const std::vector<Pick> &picks)
{
    DLString family = (*kindConfig)["item_type"].asString( );
    const Json::Value &ladder = cfg["names"][family];
    const Json::Value &noun = ladder[std::min( tier_step( tier ), (int)ladder.size( ) - 1 )];
    const Json::Value &gen = cfg["nouns"][picks.front( ).spell];

    obj->gram_gender = MultiGender( noun["gender"].asString( ).c_str( ) );

    obj->setShortDescr( random_item_compose_short(
            DLString::emptyString, noun["short"].asString( ), gen["ru"].asString( ) ), LANG_RU );

    DLString en = noun["en"].asString( );
    if (gen.isMember( "en" ))
        en += " of " + gen["en"].asString( );
    obj->setShortDescr( en, LANG_EN );

    obj->setShortDescr( random_item_compose_short(
            DLString::emptyString, noun["short_ua"].asString( ), gen["ua"].asString( ) ), LANG_UA );

    obj->setDescription( noun["long"].asString( ).c_str( ), LANG_RU );
    obj->setDescription( noun["long_en"].asString( ).c_str( ), LANG_EN );
    obj->setDescription( noun["long_ua"].asString( ).c_str( ), LANG_UA );
    obj->setKeyword( noun["name"].asString( ).c_str( ) );

    DLString colour = weapon_tier_table[tier - 1].colour;
    if (!colour.empty( ))
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            DLString s = obj->getShortDescr( (lang_t)l );
            obj->setShortDescr( "{" + colour + s.colourStrip( ) + "{x", (lang_t)l );
        }
}

void MagicItemGenerator::assignFlags()
{
    obj->setProperty( "tier", tier );
    obj->setProperty( "measure_m", chosenTotal );

    Json::Value gen;
    gen["kind"] = kind;
    gen["rev"] = 1;
    obj->props["magic_gen"] = gen;

    SET_BIT( obj->extra_flags, weapon_tier_table[tier - 1].extra.getValue( ) );

    int days = (*tierConfig)["days"].asInt( );
    if (days > 0)
        obj->timer = days * Date::SECOND_IN_DAY / Date::SECOND_IN_MINUTE;

    obj->cost = item_model_cost( chosenTotal, obj->level );
}

/*----------------------------------------------------------------------------
 * MagicItemRandomizer: reset stubs
 *---------------------------------------------------------------------------*/
MagicItemRandomizer * MagicItemRandomizer::thisClass = 0;

MagicItemRandomizer::MagicItemRandomizer()
{
    checkDuplicate( thisClass );
    thisClass = this;
}

MagicItemRandomizer::~MagicItemRandomizer()
{
    thisClass = 0;
}

void MagicItemRandomizer::initialization()
{
    eventBus->subscribe( typeid(ItemResetEvent), Pointer(this) );
}

void MagicItemRandomizer::destruction()
{
    eventBus->unsubscribe( typeid(ItemResetEvent), Pointer(this) );
}

void MagicItemRandomizer::handleEvent(const type_index &eventType, const Event &event) const
{
    if (eventType == typeid(ItemResetEvent))
        eventItemReset( static_cast<const ItemResetEvent &>(event) );
}

static const char *RANDOM_MAGIC = "random magic";

// A shopkeeper's stock, like WeaponRandomizer's: any level, no alignment.
static bool item_is_sold(Object *obj)
{
    Character *carrier = obj->getCarrier( );
    return carrier
            && carrier->is_npc( )
            && mob_has_occupation( carrier->getNPC( ), OCC_SHOPPER )
            && IS_OBJ_STAT(obj, ITEM_INVENTORY);
}

// A carried item takes its carrier's level (capped like death drops), a chest
// or floor item the area's level window rather than the whole 1..100.
static int reset_level(Object *obj)
{
    if (item_is_sold( obj ))
        return number_range( 1, LEVEL_MORTAL );

    Character *carrier = obj->getCarrier( );
    if (carrier)
        return std::min( (int)carrier->getRealLevel( ), LEVEL_MORTAL + 3 );

    Room *room = obj->getRoom( );
    if (room && room->areaIndex( )) {
        AreaIndexData *area = room->areaIndex( );
        if (area->low_range > 0 && area->high_range >= area->low_range)
            return number_range( area->low_range, std::min( area->high_range, LEVEL_MORTAL ) );
    }

    return number_range( 1, LEVEL_MORTAL );
}

void MagicItemRandomizer::eventItemReset(const ItemResetEvent &event) const
{
    Object *obj = event.obj;

    if (!obj->pIndexData || !obj->pIndexData->props.isMember( RANDOM_MAGIC ))
        return;

    if (!obj->getProperty( "tier" ).empty( ))
        return;

    const Json::Value &k = obj->pIndexData->props[RANDOM_MAGIC];
    DLString kind = k.isString( ) ? DLString( k.asString( ) ) : DLString::emptyString;

    int best = event.pReset && event.pReset->bestTier > 0 ? event.pReset->bestTier : DEFAULT_TIER;
    int tier = random_weapon_tier( URANGE(BEST_TIER, best, WORST_TIER) );

    Character *carrier = item_is_sold( obj ) ? 0 : obj->getCarrier( );
    MagicItemGenerator gen( obj, carrier, tier, kind );
    gen.level( reset_level( obj ) );
    if (carrier)
        gen.alignment( carrier->alignment );

    if (gen.run( ))
        return;

    // Once per prototype: a stub that cannot roll would warn on every reset.
    static std::set<int> warned;
    if (warned.insert( obj->pIndexData->vnum ).second)
        LogStream::sendWarning( ) << "random magic: obj " << obj->pIndexData->vnum
                                  << " kind '" << kind << "' did not roll" << endl;
}
