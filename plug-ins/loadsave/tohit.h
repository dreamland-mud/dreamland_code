/*
 * The armor model (fight/to_hit.json): hit chance from accuracy against evasion,
 * armor as graded damage reduction that hitroll pierces. The formulas here and in
 * the callers must stay equal to the design oracle, reviews/2026-10-30-ac-hitroll-data
 * c2.py and c4.py.
 *
 * Lives in loadsave, not fight_core, because the armor recompute runs from
 * PCharacter::load and every plugin that shows or spends armor links loadsave.
 *
 * With no to_hit.json the model is 0 and nothing here changes the game: every
 * caller gates on to_hit_model() == 1.
 */
#ifndef TOHIT_H
#define TOHIT_H

#include <set>
#include <string>

class Character;
class PCharacter;
class Object;
class Wearlocation;

struct ToHitConfig {
    int model = 0;
    int armor_version = 0;       // 0 = no recompute at login
    double hit_pow = 2.5;        // exponent of the accuracy/evasion roll
    double base_w = 0.33;        // weight of the class base in accuracy
    double ev_level = 0.7;       // evasion per victim level
    double ev_dex = 0.33;        // evasion per dex defense point, scaled by level/100
    double p_floor = 5, p_cap = 95;
    double dr_level = 4;         // armor half point per victim level ...
    double dr_pierce = 4;        // ... and per point of attacker effective hitroll
    double dr_pow = 1.5;
    double dr_cap = 60;
    double spell_factor = 0.7;   // share of the exotic reduction an elemental hit takes
    double spell_pierce_level = 1.2;  // a caster's effective hitroll per level
    double armor_use_bonus = 3;  // armor per victim level on an armor use success
    double blind_factor = 1.25;  // evasion multiplier against a blind attacker
    // Physical: full reduction, hitroll pierces. Energy has no column of its own
    // and reads the exotic one. Exotic: spell factor, caster level pierces.
    std::set<std::string> physical_types = { "bash", "pierce", "slash", "energy" };
    std::set<std::string> exotic_types = { "fire", "cold", "lightning", "acid", "light" };
};

const ToHitConfig & to_hit_config( );

/** Active model: the runtime override if one is set, else the config. */
int to_hit_model( );
/** Runtime override: 0 or 1, -1 follows the config again. */
void to_hit_model_override( int model );

/** The attacker's hitroll as it reaches the roll: skill% and strength applied. */
double to_hit_hitroll_eff( Character *ch, int skillPercent );
/** thacCoeff was points on a d20, hence the 20. A skill's bonus aims and pierces. */
double to_hit_skill_eff( double hitrollEff, int thacCoeff, int skillEffective );
double to_hit_accuracy( int classBase, double hitrollEff );
/** dexDefensive is the dex_app value: negative is good. */
double to_hit_evasion( int victimLevel, int dexDefensive );
/** Chance to hit, 0..1, between p_floor and p_cap. */
double to_hit_chance( double accuracy, double evasion );
/** Share of damage armor stops, 0..dr_cap. armor is positive: more is better. */
double to_hit_reduction( double armor, int victimLevel, double hitrollEff );
/** The same for an elemental hit: exotic column, spell factor, caster-level pierce. */
double to_hit_reduction_spell( double armor, int victimLevel, int casterLevel );

/** Armor as the current model shows it: GET_DEFENSE under model 1, else GET_AC. */
int to_hit_shown_armor( Character *ch, int type );

/** armorCoef lives on DefaultWearlocation, in a plugin loadsave cannot link.
 *  Installed by the wearlocation plugin, cleared on its unload. */
typedef int (*WearlocArmorCoefFn)( Wearlocation * );
typedef int (*WearFlagsArmorCoefFn)( int wearFlags );
extern WearlocArmorCoefFn wearloc_armor_coef_fn;
extern WearFlagsArmorCoefFn wear_flags_armor_coef_fn;

/** armorCoef of the best stat-giving slot these wear flags fit, 1 if unknown. */
int armor_coef_for_flags( int wearFlags );

/** What armor[] should be from the worn gear and the affects on ch, written into
 *  columns[4]. False while the wearlocation plugin is away. */
bool armor_recompute( PCharacter *ch, int columns[4] );

/** What hitroll and damroll should be from the worn gear and the affects on ch. */
void roll_recompute( PCharacter *ch, int &hitroll, int &damroll );

/** Log hitroll or damroll that drifted from roll_recompute, with what is worn. */
void roll_drift_log( PCharacter *ch );

/** Rebuild armor[] once per fight/to_hit.json armor_version, at load. */
void recalc_armor_if_stale( PCharacter *ch );

/** The version stamped on this character, 0 if none. */
int armor_version_of( PCharacter *ch );
void armor_version_stamp( PCharacter *ch, int version );

#endif
