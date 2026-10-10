/*
 * Item economy: weight and price models for every object, the boot pass that
 * applies them to prototypes and the one-shot migration of saved objects.
 * docs/plans/weight-cost-sweep.md; the offline port is scripts/weight-cost-census.py,
 * keep the two in step.
 */
#include <algorithm>
#include <cmath>
#include <jsoncpp/json/json.h>

#include "itemeconomy.h"
#include "itemmodel.h"
#include "itemvalue.h"
#include "material-table.h"
#include "damageflags.h"
#include "configurable.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "plugininitializer.h"
#include "logstream.h"
#include "exception.h"
#include "character.h"
#include "wrapperbase.h"
#include "stringset.h"
#include "core/object.h"
#include "money_utils.h"
#include "loadsave.h"
#include "save.h"
#include "merc.h"
#include "def.h"

using std::min;
using std::max;

static Json::Value itemWeightConfig;
static void econ_config_loaded( );

CONFIGURABLE_LOADED(fight, item_weight)
{
    itemWeightConfig = value;
    econ_config_loaded( );
}

/** Read-only view: the non-const operator[] would insert missing keys, and a
 *  file that is not an object reads as empty instead of throwing. */
static const Json::Value & IW( )
{
    static const Json::Value none;
    return itemWeightConfig.isObject( ) ? itemWeightConfig : none;
}

/** Any wrong-typed config or builder data throws a std::exception (jsoncpp's
 *  LogicError is not a ::Exception). Every entry point catches it, logs the
 *  vnum and keeps the area-file value: a typo must not crash the boot. */
static void econ_error( const char *what, int vnum, const std::exception &e )
{
    LogStream::sendError( ) << "Item economy: " << what << " of obj " << vnum << ": " << e.what( ) << endl;
}

static ItemProtoPointsFn protoPointsFn = 0;

void item_set_proto_points_fn( ItemProtoPointsFn fn )
{
    protoPointsFn = fn;
}

/*--------------------------------------------------------------------------
 * JSON helpers
 *-------------------------------------------------------------------------*/
static double json_num( const Json::Value &v, const char *key, double def )
{
    if (!v.isObject( ) || !v.isMember( key ) || !v[key].isNumeric( ))
        return def;
    return v[key].asDouble( );
}

static bool json_true( const Json::Value &v, const char *key )
{
    if (!v.isObject( ) || !v.isMember( key ))
        return false;
    const Json::Value &x = v[key];
    return x.isBool( ) ? x.asBool( ) : (x.isNumeric( ) && x.asDouble( ) != 0);
}

static DLString json_str( const Json::Value &v, const char *key )
{
    if (!v.isObject( ) || !v.isMember( key ) || !v[key].isString( ))
        return DLString::emptyString;
    return v[key].asString( );
}

/** kg spec: a number, or {family: kg, default: kg}. */
static double kg_of( const Json::Value &spec, const DLString &family, double def )
{
    if (spec.isNumeric( ))
        return spec.asDouble( );
    if (spec.isObject( )) {
        if (!family.empty( ) && spec.isMember( family.c_str( ) ) && spec[family.c_str( )].isNumeric( ))
            return spec[family.c_str( )].asDouble( );
        if (spec.isMember( "default" ) && spec["default"].isNumeric( ))
            return spec["default"].asDouble( );
    }
    return def;
}

/** The family a material belongs to: the first entry whose "match" types the
 *  material has all of. An unknown material (or none) falls to the catch-all. */
static const Json::Value & econ_family( const material_t *mat )
{
    static const Json::Value nullValue;
    const Json::Value &families = IW( )["families"];

    if (!families.isArray( ))
        return nullValue;

    bitstring_t mtype = mat ? mat->type.getValue( ) : 0;

    for (unsigned int i = 0; i < families.size( ); i++) {
        const Json::Value &f = families[i];
        if (!f.isObject( ))
            continue;
        DLString match = json_str( f, "match" );
        bitstring_t need = 0;

        if (!match.empty( )) {
            need = material_types.bitstring( match, true );
            // An unknown type name answers NO_FLAG, a mask with bit 0 set: skip it.
            if (need == NO_FLAG || need <= 0)
                continue;
        }

        if ((mtype & need) == need)
            return f;
    }

    return nullValue;
}

/*--------------------------------------------------------------------------
 * Weight model
 *-------------------------------------------------------------------------*/
struct EconShape {
    int itemType;
    int wearFlags;
    int value[5];
    DLString material;
    DLString heft;
    int coverage;
};

/** The slots entry of the first wear flag the item has, or null. */
static const Json::Value & econ_slot_entry( int wearFlags, bool withHold )
{
    static const Json::Value nullValue;
    const Json::Value &slots = IW( )["slots"];

    if (!slots.isArray( ))
        return nullValue;

    for (unsigned int i = 0; i < slots.size( ); i++) {
        DLString wear = json_str( slots[i], "wear" );
        if (wear.empty( ) || (!withHold && wear == "hold"))
            continue;
        bitstring_t bit = wear_flags.bitstring( wear, true );
        if (bit == NO_FLAG || bit <= 0)
            continue;
        if (IS_SET( wearFlags, bit ))
            return slots[i];
    }

    return nullValue;
}

/** The table entry for this shape and the fallback kg when it has none. */
static const Json::Value & econ_entry( const EconShape &s, double &def )
{
    static const Json::Value nullValue;
    DLString typeName = item_table.name( s.itemType );
    const Json::Value &typeEntry = IW( )["types"].isObject( ) && IW( )["types"].isMember( typeName.c_str( ) )
                                   ? IW( )["types"][typeName.c_str( )] : nullValue;

    // A worn item of such a type (boots of water walking) weighs by its slot.
    if (s.itemType != ITEM_ARMOR && s.itemType != ITEM_CLOTHING
        && json_true( typeEntry, "worn_uses_slot" )) {
        const Json::Value &slot = econ_slot_entry( s.wearFlags, false );
        if (!slot.isNull( )) {
            def = json_num( IW( ), "slot_default", 0.5 );
            return slot;
        }
    }

    if (s.itemType == ITEM_ARMOR || s.itemType == ITEM_CLOTHING) {
        def = json_num( IW( ), "slot_default", 0.5 );
        const Json::Value &slot = econ_slot_entry( s.wearFlags, true );
        if (!slot.isNull( ))
            return slot;
        const Json::Value &types = IW( )["types"];
        if (types.isObject( ) && types.isMember( typeName.c_str( ) ))
            return types[typeName.c_str( )];
        return nullValue;
    }

    if (s.itemType == ITEM_WEAPON) {
        def = json_num( IW( ), "weapon_default", 1.5 );
        DLString wclass = weapon_class.name( s.value[0] );
        const Json::Value &weapons = IW( )["weapons"];
        if (!wclass.empty( ) && weapons.isObject( ) && weapons.isMember( wclass.c_str( ) ))
            return weapons[wclass.c_str( )];
        return nullValue;
    }

    def = json_num( IW( ), "type_default", 0.5 );
    const Json::Value &types = IW( )["types"];
    if (types.isObject( ) && types.isMember( typeName.c_str( ) ))
        return types[typeName.c_str( )];
    return nullValue;
}

static int econ_clamp_units( double units )
{
    int lo = (int)json_num( IW( ), "min_units", 1 );
    int hi = (int)json_num( IW( ), "max_units", 100000 );
    if (units > hi)
        return hi;
    return max( lo, (int)(units + 0.5) );
}

static int econ_weight( const EconShape &s )
{
    if (s.itemType == ITEM_MONEY)
        return Money::weight( s.value[1], s.value[0] );

    double unitGrams = json_num( IW( ), "unit_grams", 45.36 );
    if (unitGrams <= 0)
        unitGrams = 45.36;

    DLString materials = s.material.empty( ) ? DLString( "none" ) : s.material;
    std::list<DLString> mats = materials.split( ", " );
    if (mats.empty( ))
        mats.push_back( "none" );

    int n = min( (int)mats.size( ), 16 );
    double den = (double)((1 << n) - 1);
    DLString typeName = item_table.name( s.itemType );

    // Solid lumps: a block of l x w x th mm, times each material's density.
    const Json::Value &blocks = IW( )["blocks"];
    if (blocks.isObject( ) && blocks.isMember( typeName.c_str( ) )) {
        const Json::Value &b = blocks[typeName.c_str( )];
        if (b.isArray( ) && b.size( ) == 3
            && b[0].isNumeric( ) && b[1].isNumeric( ) && b[2].isNumeric( )) {
            double cm3 = b[0].asDouble( ) * b[1].asDouble( ) * b[2].asDouble( ) / 1000.0;
            double grams = 0;
            int i = 0;
            for (auto &m: mats) {
                if (i >= n)
                    break;
                const material_t *mat = material_by_name( m );
                double rho = mat ? mat->rho : 0;
                grams += cm3 * rho / 1000.0 * (double)(1 << (n - 1 - i)) / den;
                i++;
            }
            return econ_clamp_units( grams / unitGrams );
        }
    }

    double def = 0.5;
    const Json::Value &entry = econ_entry( s, def );
    bool twoHands = s.itemType == ITEM_WEAPON && IS_SET( s.value[4], WEAPON_TWO_HANDS );
    bool own2h = entry.isObject( ) && entry.isMember( "kg_2h" );
    Json::Value spec = entry.isObject( ) ? entry["kg"] : Json::Value( );
    if (twoHands && own2h)
        spec = entry["kg_2h"];

    // Food weighs by the hours it feeds.
    if (s.itemType == ITEM_FOOD && s.value[0] > 0 && entry.isObject( ) && entry.isMember( "kg_per_hour" ))
        spec = Json::Value( max( kg_of( spec, DLString::emptyString, def ),
                                 json_num( entry, "kg_per_hour", 0 ) * s.value[0] ) );

    double haft = json_num( entry, "haft", 0 );
    double rmin = json_num( IW( )["ratio"], "min", 0.25 );
    double rmax = json_num( IW( )["ratio"], "max", 3.0 );
    double kg = 0;
    int i = 0;

    for (auto &m: mats) {
        if (i >= n)
            break;
        const material_t *mat = material_by_name( m );
        const Json::Value &fam = econ_family( mat );
        DLString famName = json_str( fam, "name" );
        double famRho = json_num( fam, "rho", 0 );
        double base = kg_of( spec, famName, def );
        double ratio = 1.0;

        if (mat && famRho > 0 && mat->rho > 0)
            ratio = mat->rho / famRho;
        ratio = max( rmin, min( rmax, ratio ) );

        // A wooden haft is the same whatever the head is made of.
        double eff = (1.0 - haft) * ratio + haft;
        kg += base * eff * (double)(1 << (n - 1 - i)) / den;
        i++;
    }

    // Heft: how much of the body a worn piece covers. A coverage prop wins.
    if (s.itemType == ITEM_ARMOR || s.itemType == ITEM_CLOTHING) {
        if (s.coverage > 0) {
            double ref = json_num( IW( ), "coverage_ref", 65 );
            if (ref > 0)
                kg *= min( s.coverage, 100 ) / ref;
        }
        else {
            DLString heft = s.heft.empty( ) ? DLString( "medium" ) : s.heft;
            kg *= json_num( IW( )["heft"], heft.c_str( ), 1.0 );
        }
    }

    if (twoHands && !own2h && !json_true( entry, "inherent_2h" ))
        kg *= json_num( IW( ), "two_hands", 2.0 );

    if (entry.isObject( ) && entry.isMember( "max_kg" ))
        kg = min( kg, json_num( entry, "max_kg", kg ) );

    int units = econ_clamp_units( kg * 1000.0 / unitGrams );

    if (s.itemType == ITEM_DRINK_CON)
        units += (int)json_num( entry, "liquid_units", 1 ) * max( 0, s.value[1] );

    return units;
}

static int prop_number( const DLString &v )
{
    // toInt() throws on overflow: only short plain numbers.
    return v.size( ) <= 9 && v.isNumber( ) ? v.toInt( ) : 0;
}

int item_weight( Object *obj, const DLString &heft )
{
    try {
        EconShape s;
        s.itemType = obj->item_type;
        s.wearFlags = obj->wear_flags;
        for (int i = 0; i < 5; i++)
            s.value[i] = obj->valueByIndex( i );
        s.material = obj->getMaterial( );
        s.heft = heft.empty( ) ? obj->getProperty( "heft" ) : heft;
        s.coverage = prop_number( obj->getProperty( "coverage" ) );
        return econ_weight( s );
    }
    catch (const std::exception &e) {
        econ_error( "weight", obj->pIndexData ? obj->pIndexData->vnum : 0, e );
        return obj->weight;
    }
}

int item_weight( obj_index_data *pObj, const DLString &heft )
{
    try {
        EconShape s;
        s.itemType = pObj->item_type;
        s.wearFlags = pObj->wear_flags;
        for (int i = 0; i < 5; i++)
            s.value[i] = pObj->value[i];
        s.material = pObj->material;
        s.heft = heft.empty( ) ? pObj->getProperty( "heft" ) : heft;
        s.coverage = prop_number( pObj->getProperty( "coverage" ) );
        return econ_weight( s );
    }
    catch (const std::exception &e) {
        econ_error( "weight", pObj->vnum, e );
        return pObj->xml_weight > 0 ? pObj->xml_weight : pObj->weight;
    }
}

int item_proto_weight( obj_index_data *pObj )
{
    if (pObj->xml_weight > 0)
        return pObj->xml_weight;
    return item_weight( pObj );
}

/** The instance is still its prototype's shape, so it weighs what the prototype does. */
static bool econ_same_shape( Object *obj )
{
    obj_index_data *p = obj->pIndexData;

    if (obj->item_type != p->item_type)
        return false;
    if (DLString( obj->getMaterial( ) ) != p->material)
        return false;
    if (obj->props.isObject( ) && (obj->props.isMember( "heft" ) || obj->props.isMember( "coverage" )))
        return false;
    if (obj->item_type == ITEM_WEAPON
        && (obj->value0( ) != p->value[0]
            || IS_SET( obj->value4( ), WEAPON_TWO_HANDS ) != IS_SET( p->value[4], WEAPON_TWO_HANDS )))
        return false;
    return true;
}

int item_instance_weight( Object *obj )
{
    if (obj->item_type == ITEM_MONEY)
        return Money::weight( obj->value1( ), obj->value0( ) );
    if (econ_same_shape( obj ))
        return obj->pIndexData->weight;
    return item_weight( obj );
}

/*--------------------------------------------------------------------------
 * Price model
 *-------------------------------------------------------------------------*/
int item_cost_cap( )
{
    int cap = (int)item_value( "measure", "cost_cap", 50000 );
    return cap > 0 ? cap : 50000;
}

static bool json_list_has_str( const Json::Value &list, const DLString &name )
{
    if (!list.isArray( ))
        return false;
    for (unsigned int i = 0; i < list.size( ); i++)
        if (list[i].isString( ) && name == list[i].asString( ))
            return true;
    return false;
}

int item_base_cost( obj_index_data *pObj )
{
    const int *v = pObj->value;
    int level = max( 0, pObj->level );
    DLString typeName = item_table.name( pObj->item_type );
    double base = 0;

    switch (pObj->item_type) {
    case ITEM_POTION:
    case ITEM_PILL:
    case ITEM_SCROLL: {
        int spells = 0;
        for (int i = 1; i < 5; i++)
            if (v[i] > 0)
                spells++;
        base = max( 1, v[0] ) * max( 1, spells ) * item_value( "cost", "spell_per_level", 50 );
        break;
    }
    case ITEM_WAND:
    case ITEM_STAFF:
        base = max( 1, v[0] ) * max( 1, v[1] ) * item_value( "cost", "charge_per_level", 15 );
        break;
    case ITEM_FOOD:
        base = min( max( 0, v[0] ), (int)item_value( "cost", "food_max_hours", 100 ) )
               * item_value( "cost", "food_per_hour", 5 );
        break;
    case ITEM_DRINK_CON:
        base = max( 0, v[0] ) * item_value( "cost", "drink_per_unit", 2 );
        break;
    case ITEM_LIGHT:
        if (v[2] < 0 || v[2] >= 999)
            base = item_value( "cost", "light_infinite", 1000 );
        else
            base = v[2] * item_value( "cost", "light_per_hour", 3 );
        break;
    case ITEM_CONTAINER: {
        int mult = v[4] > 0 ? v[4] : 100;
        base = max( 0, v[0] ) * item_value( "cost", "container_per_lb", 3 )
               + max( 0, 100 - mult ) * item_value( "cost", "container_mult_premium", 50 );
        break;
    }
    case ITEM_RECIPE:
        base = max( 1, v[2] ) * item_value( "cost", "recipe_per_complexity", 1500 );
        break;
    default: {
        int worn = pObj->wear_flags;
        REMOVE_BIT( worn, ITEM_TAKE | ITEM_WEAR_TATTOO );
        if (worn != 0 && json_list_has_str( item_value_object( "cost", "worn_types" ), typeName ))
            base = level * item_value( "cost", "worn_per_level", 12 );
        break;
    }
    }

    if (json_list_has_str( item_value_object( "cost", "valuable_types" ), typeName ))
        base += level * item_value( "cost", "valuable_per_level", 40 );

    const Json::Value &floors = item_value_object( "cost", "type_floor" );
    if (floors.isObject( ) && floors.isMember( typeName.c_str( ) ) && floors[typeName.c_str( )].isNumeric( ))
        base += floors[typeName.c_str( )].asDouble( );
    else
        base += item_value( "cost", "type_floor_default", 1 );

    // Precious materials: price per kg x weight, each material by its share.
    double unitGrams = json_num( IW( ), "unit_grams", 45.36 );
    double kg = item_proto_weight( pObj ) * unitGrams / 1000.0;
    DLString materials = pObj->material.empty( ) ? DLString( "none" ) : pObj->material;
    std::list<DLString> mats = materials.split( ", " );
    int n = min( (int)mats.size( ), 16 );
    double den = (double)((1 << n) - 1);
    int i = 0;
    for (auto &m: mats) {
        if (i >= n)
            break;
        const material_t *mat = material_by_name( m );
        if (mat && mat->price > 0)
            base += mat->price * kg * (double)(1 << (n - 1 - i)) / den;
        i++;
    }

    // Everything above is double: an absurd weight or price can not wrap an int.
    return (int)min( (double)item_cost_cap( ), max( 0.0, base ) );
}

/** The measure slot of a prototype (item_value.json measure.slot_factor keys). */
static DLString econ_measure_slot( obj_index_data *pObj )
{
    static const struct { int bit; const char *slot; } slots[] = {
        { ITEM_WEAR_SHIELD, "shield" }, { ITEM_WEAR_HEAD, "head" }, { ITEM_WEAR_BODY, "body" },
        { ITEM_WEAR_WAIST, "waist" }, { ITEM_WEAR_NECK, "neck" }, { ITEM_WEAR_FINGER, "finger" },
        { ITEM_WEAR_WRIST, "wrist" }, { ITEM_WEAR_EARS, "ears" }, { ITEM_WEAR_FLOAT, "float" },
    };

    for (auto &s: slots)
        if (IS_SET( pObj->wear_flags, s.bit ))
            return s.slot;
    if (pObj->item_type == ITEM_LIGHT)
        return "light";
    return DLString::emptyString;
}

int item_measure_cm( obj_index_data *pObj )
{
    if (!protoPointsFn || pObj->level <= 0)
        return 0;

    // Stats are what M prices: gear always, anything else only with affects.
    DLString typeName = item_table.name( pObj->item_type );
    if (!json_list_has_str( item_value_object( "cost", "worn_types" ), typeName )
        && pObj->affected.empty( ))
        return 0;

    DLString slot = econ_measure_slot( pObj );
    double best = 0;

    for (int c = 0; c < 2; c++) {
        bool caster = c == 1;
        double om = item_one_m( pObj->level, caster, slot );
        if (om <= 0)
            continue;
        best = max( best, protoPointsFn( pObj, caster ) / om );
    }

    return (int)min( 1000000.0, best * 100 + 0.5 );
}

/** Takeable and not coins: the only prototypes the scorer is asked about. */
static bool econ_priced( obj_index_data *pObj )
{
    return IS_SET( pObj->wear_flags, ITEM_TAKE ) && pObj->item_type != ITEM_MONEY;
}

/** The auto price with the measure already known (the scorer runs once). */
static int econ_auto_cost( obj_index_data *pObj, int measureCm )
{
    int cap = item_cost_cap( );

    if (!IS_SET( pObj->wear_flags, ITEM_TAKE ))
        return 0;

    // Coins are worth their face value.
    if (pObj->item_type == ITEM_MONEY)
        return min( (long long)cap, max( 0LL, (long long)pObj->value[0] + 100LL * pObj->value[1] ) );

    long long base = item_base_cost( pObj );
    long long magic = item_model_cost( measureCm, max( 0, pObj->level ) );
    return (int)min( (long long)cap, base + magic );
}

int item_auto_cost( obj_index_data *pObj )
{
    try {
        return econ_auto_cost( pObj, econ_priced( pObj ) ? item_measure_cm( pObj ) : 0 );
    }
    catch (const std::exception &e) {
        // The cap makes min(auto, area cost) fall back to the area value.
        econ_error( "auto cost", pObj->vnum, e );
        return item_cost_cap( );
    }
}

bool item_scripted( obj_index_data *pObj )
{
    if (pObj->behavior || !pObj->behaviors.empty( ))
        return true;

    // An OLC copy has no wrapper of its own: ask the real prototype.
    Scripting::Object *wrapper = pObj->wrapper;
    if (wrapper == 0) {
        obj_index_data *orig = get_obj_index( pObj->vnum );
        if (orig != 0)
            wrapper = orig->wrapper;
    }

    WrapperBase *w = get_wrapper( wrapper );
    if (w == 0)
        return false;

    StringSet triggers, misc;
    w->collectTriggers( triggers, misc );
    return !triggers.empty( );
}

int item_proto_cost( obj_index_data *pObj )
{
    int cap = item_cost_cap( );

    try {
        int measure = econ_priced( pObj ) ? item_measure_cm( pObj ) : 0;

        // The model can not see what a script or a behavior does (a surprise egg
        // is a container with no capacity): a scripted item it finds no stats in
        // keeps the builder's price.
        if (pObj->item_type != ITEM_MONEY && pObj->xml_cost > 0
            && measure == 0 && item_scripted( pObj ))
            return min( pObj->xml_cost, cap );

        // The area value is the most a prototype may cost: 0 stays 0 (D12 clamp).
        int autoCost = econ_auto_cost( pObj, measure );
        return min( min( autoCost, max( 0, pObj->xml_cost ) ), cap );
    }
    catch (const std::exception &e) {
        econ_error( "cost", pObj->vnum, e );
        return min( max( 0, pObj->xml_cost ), cap );
    }
}

void item_economy_proto( obj_index_data *pObj )
{
    try {
        // Weight first: the material part of the price reads it.
        pObj->weight = item_proto_weight( pObj );
        pObj->cost = item_proto_cost( pObj );
    }
    catch (const std::exception &e) {
        // Keep the area-file values: a bad prototype must not stop the boot.
        econ_error( "weight and cost", pObj->vnum, e );
        if (pObj->xml_weight > 0)
            pObj->weight = pObj->xml_weight;
        pObj->cost = min( max( 0, pObj->xml_cost ), item_cost_cap( ) );
    }
}

/*--------------------------------------------------------------------------
 * Boot pass: every prototype, once all areas are loaded and fixed
 * (SCDP_BOOT + 5, + 6) and before players, saved rooms (+ 10, + 20) and the
 * first reset (SCDP_INITIAL).
 *-------------------------------------------------------------------------*/
static void item_economy_boot( )
{
    int total = 0, autoW = 0, autoC = 0, cappedC = 0, changedW = 0, changedC = 0;

    for (int h = 0; h < MAX_KEY_HASH; h++)
        for (obj_index_data *p = obj_index_hash[h]; p; p = p->next) {
            int oldW = p->weight, oldC = p->cost;

            try {
                item_economy_proto( p );
            }
            catch (const std::exception &e) {
                econ_error( "boot pass", p->vnum, e );
                p->weight = oldW;
                p->cost = oldC;
            }

            total++;
            if (p->xml_weight <= 0)
                autoW++;
            if (p->xml_cost <= 0)
                autoC++;
            else if (p->cost < p->xml_cost)
                cappedC++;
            if (p->weight != oldW)
                changedW++;
            if (p->cost != oldC)
                changedC++;
        }

    LogStream::sendNotice( )
        << "Item economy: " << total << " prototypes; weight auto " << autoW
        << ", changed " << changedW << "; cost auto " << autoC
        << ", area cost above the model " << cappedC << ", changed " << changedC << endl;
}

class ItemEconomyBootTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<ItemEconomyBootTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis( )->getCurrentTick( ) == 0)
            item_economy_boot( );
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 7;
    }
};

PluginInitializer<ItemEconomyBootTask> initItemEconomyBoot;

/*--------------------------------------------------------------------------
 * Migration of saved objects (weight-cost-sweep D14), called from fread_obj.
 *-------------------------------------------------------------------------*/
static DLString econ_migration_mode( )
{
    const Json::Value &m = IW( )["migration"];
    if (!m.isObject( ) || !m["mode"].isString( ))
        return "off";
    return m["mode"].asString( );
}

static void econ_config_loaded( )
{
    const Json::Value &m = IW( )["migration"];
    int rev = m.isObject( ) ? (int)json_num( m, "rev", 0 ) : 0;
    obj_econ_rev = econ_migration_mode( ) == "off" ? 0 : max( 0, rev );
}

static bool econ_vnum_exempt( int vnum )
{
    const Json::Value &m = IW( )["migration"];
    if (!m.isObject( ))
        return false;
    const Json::Value &list = m["weight_exempt_vnums"];
    if (!list.isArray( ))
        return false;
    for (unsigned int i = 0; i < list.size( ); i++)
        if (list[i].isNumeric( ) && list[i].asInt( ) == vnum)
            return true;
    return false;
}

/** Weights set by the engine or by Fenia, not by the model. */
static bool econ_weight_exempt( Object *obj )
{
    if (!IS_SET( obj->wear_flags, ITEM_TAKE ))
        return true;

    switch (obj->item_type) {
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_BOAT:          // crafted boats weigh by the carpenter's skill
        return true;
    }

    return econ_vnum_exempt( obj->pIndexData->vnum );
}

/** Costs that are not prices. */
static bool econ_cost_exempt( Object *obj )
{
    if (obj->cost <= 0)                     // devalued at the pit
        return true;
    if (!IS_SET( obj->wear_flags, ITEM_TAKE ))
        return true;

    switch (obj->item_type) {
    case ITEM_MONEY:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
        return true;
    case ITEM_WEAPON:
        if (IS_SET( obj->value4( ), WEAPON_KATANA ))   // cost counts the katana's hits
            return true;
        break;
    }

    // A personal shop keeps the owner's price, the real one waits in psCost.
    WrapperBase *w = get_wrapper( obj->wrapper );
    if (w && w->hasField( "psCost" ))
        return true;

    // On display at a personal shop (its mob carries the Fenia "shop" map):
    // items put out before psCost existed (fenia 19e2aa01) have none.
    Character *carrier = obj->getCarrier( );
    if (carrier && carrier->is_npc( )) {
        WrapperBase *cw = get_wrapper( carrier->wrapper );
        if (cw && cw->hasField( "shop" ))
            return true;
    }

    return false;
}

static bool item_econ_migrate_body( Object *obj, int savedRev, int stage )
{
    DLString mode = econ_migration_mode( );
    bool apply = mode == "apply";

    if (!apply && mode != "dry")
        return false;

    if (stage == 0) {
        if (econ_weight_exempt( obj ))
            return apply;

        int w = item_instance_weight( obj );
        if (w != obj->weight) {
            LogStream::sendNotice( )
                << "econ " << mode << ": obj " << obj->pIndexData->vnum << " id " << obj->getID( )
                << " rev " << savedRev << " weight " << obj->weight << " -> " << w << endl;
            if (apply)
                obj->weight = w;      // not placed yet: no carry weight to fix
        }
        return apply;
    }

    if (econ_cost_exempt( obj ))
        return apply;

    int c = obj->cost;
    DLString measure;
    if (obj->props.isObject( ) && obj->props.isMember( "measure_m" ))
        measure = obj->getProperty( "measure_m" );

    if (!measure.empty( ) && measure.size( ) <= 9 && measure.isNumber( ))
        // Generated (random loot, crafts): the model's price, never above its own.
        // At least 1: cost 0 reads as "devalued at the pit".
        c = min( c, max( 1, item_model_cost( measure.toInt( ), obj->level ) ) );
    else if (!(obj->props.isObject( ) && obj->props.isMember( "tier" )))
        // Hand-made: never above the prototype's new price.
        c = min( c, max( 1, obj->pIndexData->cost ) );

    c = min( c, item_cost_cap( ) );

    if (c != obj->cost) {
        LogStream::sendNotice( )
            << "econ " << mode << ": obj " << obj->pIndexData->vnum << " id " << obj->getID( )
            << " rev " << savedRev << " cost " << obj->cost << " -> " << c << endl;
        if (apply)
            obj->cost = c;
    }

    return apply;
}

/** fread_obj has no use for an exception (it catches FileFormatException only):
 *  log it, leave the object as saved. */
static bool item_econ_migrate( Object *obj, int savedRev, int stage )
{
    try {
        return item_econ_migrate_body( obj, savedRev, stage );
    }
    catch (const std::exception &e) {
        LogStream::sendError( ) << "Item economy: migrating obj " << obj->pIndexData->vnum
                                << " id " << obj->getID( ) << ": " << e.what( ) << endl;
        return false;
    }
}

/** Hands the migration to fread_obj (loadsave); cleared on unload so loadsave
 *  never calls into an unloaded plugin. */
class ItemEconomyHooks : public Plugin {
public:
    virtual void initialization( )
    {
        obj_econ_migrate_fn = &item_econ_migrate;
        econ_config_loaded( );
    }
    virtual void destruction( )
    {
        obj_econ_migrate_fn = 0;
    }
};

PluginInitializer<ItemEconomyHooks> initItemEconomyHooks;
