/* $Id$
 *
 * ruffina, 2004
 */
#ifndef STATS_APPLY_H
#define STATS_APPLY_H

#include <vector>

class Character;
class Object;
namespace Json { class Value; }

/*
 * Attribute bonus structures.
 */
struct        str_app_type
{
    int        hit;
    int        carry;
    int        wield;
    int web;
    int damage;

    void fromJson(const Json::Value &value);
};

struct        int_app_type
{
    int        learn;
    int slevel;
    void fromJson(const Json::Value &value);    
};

struct        wis_app_type
{
    int        practice;
    int learn;
    int slevel;
    void fromJson(const Json::Value &value);    
};

struct        dex_app_type
{
    int        defensive;
    void fromJson(const Json::Value &value);    
};

/*
 * Character parameters macros and utils
 */
const struct str_app_type & get_str_app( Character * );
const struct int_app_type & get_int_app( Character * );
const struct wis_app_type & get_wis_app( Character * );
const struct dex_app_type & get_dex_app( Character * );

/*
 * Weapon weight vs strength: the primary hand holds up to str_app.wield*10,
 * the secondary hand half of that. Uncharmed NPCs are exempt for weapons
 * from their own zone unless their strength is debuffed.
 */
bool too_heavy_to_wield( Character *ch, Object *obj, bool secondary );
/* The weight cap too_heavy_to_wield tests against (before the NPC exemption). */
int wield_weight_cap( Character *ch, bool secondary );


#define GET_AC(ch,type)                             \
           ((ch)->armor[type]                            \
            + (IS_AWAKE(ch) ? get_dex_app(ch).defensive : 0))

#endif
