/* $Id$
 *
 * ruffina, 2004
 */
#ifndef STATS_APPLY_H
#define STATS_APPLY_H

class Character;

/*
 * Attribute bonus structures.
 */
struct        str_app_type
{
    int        hit;
    int        missile;
    int        carry;
    int        wield;
    int web;
    int damage;
};

struct        int_app_type
{
    int        learn;
    int slevel;
};

struct        wis_app_type
{
    int        practice;
    int learn;
};

struct        dex_app_type
{
    int        defensive;
};

struct        con_app_type
{
//    int        hitp;
    int        shock;
};

/*
 * Character parameters macros and utils
 */
const struct str_app_type & get_str_app( Character * );
const struct int_app_type & get_int_app( Character * );
const struct wis_app_type & get_wis_app( Character * );
const struct dex_app_type & get_dex_app( Character * );

#define GET_AC(ch,type)                             \
           ((ch)->armor[type]                            \
            + (IS_AWAKE(ch) ? get_dex_app(ch).defensive : 0))

#endif
