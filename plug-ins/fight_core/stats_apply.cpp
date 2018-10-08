/* $Id$
 *
 * ruffina, 2004
 */
#include "stats_apply.h"

#include "character.h"
#include "profession.h"
#include "pcrace.h"
#include "merc.h"

#ifndef FIGHT_STUB
/*
 * Attribute bonus tables.
 */
const struct        str_app_type        str_app                []                =
{
/*   hit% missile% carry wield web damage% */
    { -35,  -40,     0,    0,   5,  -40     },  /* 0  */
    { -30,  -40,     3,    1,   5,  -40     },  /* 1  */
    { -20,  -20,     3,    2,   5,  -30     },
    { -20,  -10,    10,    3,   5,  -30     },  /* 3  */
    { -15,  -10,    25,    4,   5,  -30     },
    { -15,  -10,    55,    5,   5,  -30     },  /* 5  */
    { -10,    0,    80,    6,   5,  -25     },
    {  -5,    0,    90,    7,   5,  -25     },
    {   0,    0,   100,    8,   5,  -25     },
    {   0,    0,   100,    9,   5,  -25     },
    {   0,    0,   115,   10,   5,  -25     }, /* 10  */
    {   0,    0,   115,   11,   5,  -20     },
    {   0,    0,   130,   12,   5,  -20     },
    {   0,    0,   130,   13,   5,  -20     }, /* 13  */
    {   2,   10,   140,   14,   6,  -17     },
    {   2,   10,   150,   15,   7,  -15     }, /* 15  */
    {   3,   20,   165,   16,   8,  -13     },
    {   3,   30,   180,   22,  12,  -10     },
    {   3,   30,   200,   25,  13,   -7     }, /* 18  */
    {   4,   40,   225,   30,  16,   -5     },
    {   4,   50,   250,   35,  20,    0     }, /* 20  */
    {   5,   60,   300,   40,  24,    3     },
    {   5,   60,   350,   45,  25,    6     },
    {   6,   70,   400,   50,  28,   10     },
    {   6,   80,   450,   55,  32,   13     },
    {   7,   90,   500,   60,  36,   15     }, /* 25   */
    {   8,   95,   550,   65,  37,   20     }  /* 25+ */
};           


const struct        int_app_type        int_app                []                =
{
/* prac    slevel */
    {  3,   -5 },        /*  0 */
    {  5,   -5 },        /*  1 */
    {  7,   -5 },
    {  8,   -4 },        /*  3 */
    {  9,   -4 },
    { 10,   -4 },        /*  5 */
    { 11,   -4 },
    { 12,   -4 },
    { 13,   -3 },
    { 15,   -3 },
    { 17,   -3 },        /* 10 */
    { 19,   -3 },
    { 22,   -3 },
    { 25,   -2 },
    { 28,   -2 },
    { 31,   -2 },        /* 15 */
    { 34,   -2 },
    { 37,   -1 },
    { 40,   -1 },        /* 18 */
    { 44,   -1 },
    { 49,    0 },        /* 20 */
    { 55,    0 },
    { 60,    1 },
    { 70,    2 },
    { 80,    3 },
    { 85,    4 },        /* 25  */
    { 90,    5 }        /* 25+ */
};



const struct        wis_app_type        wis_app                []                =
{
/* prac learn */
    { 0, 0, },        /*  0 */
    { 0, 0, },        /*  1 */
    { 0, 0, },
    { 0, 0, },        /*  3 */
    { 0, 0, },
    { 1, 0, },        /*  5 */
    { 1, 0, },
    { 1, 0, },
    { 1, 0, },
    { 1, 0, },
    { 1, 0, },        /* 10 */
    { 1, 0, },
    { 1, 0, },
    { 1, 1, },
    { 1, 1, },
    { 2, 1, },        /* 15 */
    { 2, 1, },
    { 2, 1, },
    { 3, 1, },        /* 18 */
    { 3, 1, },
    { 3, 2, },        /* 20 */
    { 3, 2, },
    { 4, 2, },
    { 4, 3, },
    { 4, 3, },
    { 5, 4, }  /* 25 */
};



const struct        dex_app_type        dex_app                []                =
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const struct        con_app_type        con_app                []                =
{
    { 20 },   /*  0 */
    { 25 },   /*  1 */
    { 30 },
    { 35 },          /*  3 */
    { 40 },
    { 45 },   /*  5 */
    { 50 },
    { 55 },
    { 60 },
    { 65 },
    { 70 },   /* 10 */
    { 75 },
    { 80 },
    { 85 },
    { 88 },
    { 90 },   /* 15 */
    { 95 },
    { 97 },
    { 99 },   /* 18 */
    { 99 },
    { 99 },   /* 20 */
    { 99 },
    { 99 },
    { 99 },
    { 99 },
    { 99 }    /* 25 */
};    

/*
 * Attribute helper functions
 */
static int get_curr_stat_extra( Character *ch, int stat )
{
    int value;

    value = ch->getCurrStat( stat );

    if (value == MAX_STAT 
        && !ch->is_npc( )
        && ch->getTrueProfession( )->getStat( stat ) > 0
        && ch->getRace( )->getPC( )->getStats( )[stat] >= MAX_STAT - BASE_STAT)
        value++;

    return value;
}

const struct str_app_type & get_str_app( Character *ch )
{
    return str_app[get_curr_stat_extra( ch, STAT_STR )];
}

const struct int_app_type & get_int_app( Character *ch )
{
    return int_app[get_curr_stat_extra( ch, STAT_INT )];
}

const struct wis_app_type & get_wis_app( Character *ch )
{
    return wis_app[get_curr_stat_extra( ch, STAT_WIS )];
}

const struct dex_app_type & get_dex_app( Character *ch )
{
    return dex_app[get_curr_stat_extra( ch, STAT_DEX )];
}

#else
static const struct str_app_type zero_str_app = { 4, 50, 250, 35,  20,    0     }; /* 20  */
static const struct int_app_type zero_int_app = { 49, 0 };        /* 20 */
static const struct wis_app_type zero_wis_app = { 3, 2, };        /* 20 */
static const struct dex_app_type zero_dex_app = { - 50 };   /* 20 */
const struct str_app_type & get_str_app( Character * ) { return zero_str_app; }
const struct int_app_type & get_int_app( Character * ) { return zero_int_app; }
const struct wis_app_type & get_wis_app( Character * ) { return zero_wis_app; }
const struct dex_app_type & get_dex_app( Character * ) { return zero_dex_app; }
#endif
