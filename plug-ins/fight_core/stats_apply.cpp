/* $Id$
 *
 * ruffina, 2004
 */
#include "stats_apply.h"
#include <vector>
#include <jsoncpp/json/json.h>

#include "configurable.h"
#include "character.h"
#include "profession.h"
#include "pcrace.h"
#include "merc.h"

/*
 * Attribute bonus tables.
 */
json_vector<str_app_type> str_app;
CONFIGURABLE_LOADED(fight, str_app)
{
    str_app.fromJson(value);
}

json_vector<int_app_type> int_app;
CONFIGURABLE_LOADED(fight, int_app)
{
    int_app.fromJson(value);
}

json_vector<wis_app_type> wis_app;
CONFIGURABLE_LOADED(fight, wis_app)
{
    wis_app.fromJson(value);
}

json_vector<dex_app_type> dex_app;
CONFIGURABLE_LOADED(fight, dex_app)
{
    dex_app.fromJson(value);
}

const struct str_app_type & get_str_app( Character *ch )
{
    return str_app[ch->getCurrStat( STAT_STR )];
}

const struct int_app_type & get_int_app( Character *ch )
{
    return int_app[ch->getCurrStat( STAT_INT )];
}

const struct wis_app_type & get_wis_app( Character *ch )
{
    return wis_app[ch->getCurrStat( STAT_WIS )];
}

const struct dex_app_type & get_dex_app( Character *ch )
{
    return dex_app[ch->getCurrStat( STAT_DEX )];
}

void str_app_type::fromJson(const Json::Value &value)
{
    hit = value["hit"].asInt();
    missile = value["missile"].asInt();
    carry = value["carry"].asInt();
    wield = value["wield"].asInt();
    web = value["web"].asInt();
    damage = value["damage"].asInt();    
}

void int_app_type::fromJson(const Json::Value &value)
{
    learn = value["learn"].asInt();
    slevel = value["slevel"].asInt();
}

void wis_app_type::fromJson(const Json::Value &value)
{
    practice = value["practice"].asInt();
    learn = value["learn"].asInt();
}

void dex_app_type::fromJson(const Json::Value &value)
{
    defensive = value["defensive"].asInt();
}
