#include "tohit.h"

#include <algorithm>
#include <cmath>
#include <jsoncpp/json/json.h>

#include "configurable.h"
#include "logstream.h"
#include "commonattributes.h"
#include "character.h"
#include "pcharacter.h"
#include "object.h"
#include "affect.h"
#include "affectflags.h"
#include "wearlocation.h"
#include "stats_apply.h"
#include "merc.h"
#include "def.h"

using std::max;
using std::min;

static const char *ARMOR_VERSION_ATTR = "armor_version";

static ToHitConfig config;
static int modelOverride = -1;

WearlocArmorCoefFn wearloc_armor_coef_fn = 0;
WearFlagsArmorCoefFn wear_flags_armor_coef_fn = 0;

static double json_num( const Json::Value &value, const char *key, double def )
{
    return value.isMember( key ) && value[key].isNumeric( ) ? value[key].asDouble( ) : def;
}

static void json_names( const Json::Value &value, const char *key, std::set<std::string> &names )
{
    if (!value.isMember( key ) || !value[key].isArray( ))
        return;

    names.clear( );
    for (const auto &name: value[key])
        names.insert( name.asString( ) );
}

CONFIGURABLE_LOADED(fight, to_hit)
{
    // Absent file, or a broken edit: keep what we run on, compiled defaults at boot.
    if (!value.isObject( ))
        return;

    const Json::Value &v = value;
    ToHitConfig c;
    c.model              = (int)json_num( v, "model", 0 ) == 1 ? 1 : 0;
    c.armor_version      = (int)json_num( v, "armor_version", 0 );
    c.hit_pow            = json_num( v, "hit_pow", c.hit_pow );
    c.base_w             = json_num( v, "base_w", c.base_w );
    c.ev_level           = json_num( v, "ev_level", c.ev_level );
    c.ev_dex             = json_num( v, "ev_dex", c.ev_dex );
    c.p_floor            = json_num( v, "p_floor", c.p_floor );
    c.p_cap              = json_num( v, "p_cap", c.p_cap );
    c.dr_level           = json_num( v, "dr_level", c.dr_level );
    c.dr_pierce          = json_num( v, "dr_pierce", c.dr_pierce );
    c.dr_pow             = json_num( v, "dr_pow", c.dr_pow );
    c.dr_cap             = json_num( v, "dr_cap", c.dr_cap );
    c.spell_factor       = json_num( v, "spell_factor", c.spell_factor );
    c.spell_pierce_level = json_num( v, "spell_pierce_level", c.spell_pierce_level );
    c.armor_use_bonus    = json_num( v, "armor_use_bonus", c.armor_use_bonus );
    c.blind_factor       = json_num( v, "blind_factor", c.blind_factor );
    json_names( v, "physical_types", c.physical_types );
    json_names( v, "exotic_types", c.exotic_types );
    config = c;
}

const ToHitConfig & to_hit_config( )
{
    return config;
}

int to_hit_model( )
{
    return modelOverride >= 0 ? modelOverride : config.model;
}

void to_hit_model_override( int model )
{
    modelOverride = model;
}

double to_hit_hitroll_eff( Character *ch, int skillPercent )
{
    return max( 0, ch->hitroll.getValue( ) ) * skillPercent / 100.0
           * (1 + get_str_app( ch ).hit / 100.0);
}

double to_hit_skill_eff( double hitrollEff, int thacCoeff, int skillEffective )
{
    return hitrollEff * (1 + thacCoeff * skillEffective / 100.0 / 20.0);
}

double to_hit_accuracy( int classBase, double hitrollEff )
{
    return config.base_w * classBase + hitrollEff;
}

double to_hit_evasion( int victimLevel, int dexDefensive )
{
    return config.ev_level * victimLevel
           + config.ev_dex * -dexDefensive * min( 1.0, victimLevel / 100.0 );
}

double to_hit_chance( double accuracy, double evasion )
{
    // A low level clumsy victim can compute to nothing: a hair above 0 hits at the cap.
    double a = pow( max( 0.01, accuracy ), config.hit_pow );
    double e = pow( max( 0.01, evasion ), config.hit_pow );
    return min( config.p_cap / 100.0, max( config.p_floor / 100.0, a / (a + e) ) );
}

double to_hit_reduction( double armor, int victimLevel, double hitrollEff )
{
    double k = config.dr_level * victimLevel + config.dr_pierce * hitrollEff;

    if (armor <= 0 || k <= 0)
        return 0;

    double x = pow( armor / k, config.dr_pow );
    return min( config.dr_cap / 100.0, x / (1 + x) );
}

double to_hit_reduction_spell( double armor, int victimLevel, int casterLevel )
{
    return config.spell_factor
           * to_hit_reduction( armor, victimLevel, casterLevel * config.spell_pierce_level );
}

int to_hit_shown_armor( Character *ch, int type )
{
    return to_hit_model( ) == 1 ? GET_DEFENSE( ch, type ) : GET_AC( ch, type );
}

int armor_coef_for_flags( int wearFlags )
{
    return wear_flags_armor_coef_fn ? wear_flags_armor_coef_fn( wearFlags ) : 1;
}

bool armor_recompute( PCharacter *ch, int columns[4] )
{
    if (!wearloc_armor_coef_fn)
        return false;

    int acMod = 0;

    for (int i = 0; i < 4; i++)
        columns[i] = 100;

    for (auto &paf: ch->affected)
        if (paf->location == APPLY_AC)
            acMod += paf->modifier;

    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content) {
        if (obj->item_type == ITEM_ARMOR) {
            int coef = wearloc_armor_coef_fn( obj->wear_loc.getElement( ) );
            for (int i = 0; i < 4; i++)
                columns[i] -= coef * obj->valueByIndex( i );
        }

        // The flavor slots and the backpack give nothing: same gate as equip.
        if (!obj->wear_loc->givesAffects( ))
            continue;

        for (auto &paf: obj->pIndexData->affected)
            if (paf->location == APPLY_AC)
                acMod += paf->modifier;

        for (auto &paf: obj->affected)
            if (paf->location == APPLY_AC)
                acMod += paf->modifier;
    }

    for (int i = 0; i < 4; i++)
        columns[i] += acMod;

    return true;
}

int armor_version_of( PCharacter *ch )
{
    XMLStringAttribute::Pointer attr = ch->getAttributes( ).findAttr<XMLStringAttribute>( ARMOR_VERSION_ATTR );

    if (!attr || !attr->getValue( ).isNumber( ) || attr->getValue( ).length( ) > 7)
        return 0;

    return attr->getValue( ).toInt( );
}

void armor_version_stamp( PCharacter *ch, int version )
{
    ch->getAttributes( ).getAttr<XMLStringAttribute>( ARMOR_VERSION_ATTR )->setValue( DLString( version ) );
}

void recalc_armor_if_stale( PCharacter *ch )
{
    int want = config.armor_version;

    if (want <= 0 || armor_version_of( ch ) >= want)
        return;

    int columns[4];
    if (!armor_recompute( ch, columns ))
        return;

    for (int i = 0; i < 4; i++) {
        if (ch->armor[i] != columns[i])
            LogStream::sendNotice( ) << "Armor recompute " << ch->getName( ) << " column " << i
                                     << ": " << ch->armor[i] << " -> " << columns[i] << endl;
        ch->armor[i] = columns[i];
    }

    armor_version_stamp( ch, want );
}
