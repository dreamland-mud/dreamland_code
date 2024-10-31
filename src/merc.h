/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#ifndef _MERC_H_
#define _MERC_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "autoflags.h"
#include "dl_strings.h"
#include "dl_math.h"
#include "dl_ctype.h"
#include "logstream.h"

#include "mobilefactory.h"
#include "objectfactory.h"
#include "area.h"
#include "room.h"
#include "auction.h"

#define        MAX_KEY_HASH                 1024

extern char str_empty[1];

/* RT ASCII conversions -- used so we can have letters in this file */

#define IS_SET(flag, bit)        ((flag) & (bit))
#define SET_BIT(var, bit)        ((var) |= (bit))
#define REMOVE_BIT(var, bit)        ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))


/*
 * Per-class stuff.
 */
#define MAX_STAT 28
#define MAX_STAT_REMORT 25
#define MIN_STAT 3        
#define BASE_STAT 20        

/*
 * Game parameters.
 */
#define MAX_LEVEL 110
#define LEVEL_HERO (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL (MAX_LEVEL - 8)
#define LEVEL_MORTAL 100

#define GROUP_RANGE 8

#define IMPLEMENTOR MAX_LEVEL
#define CREATOR (MAX_LEVEL - 1)
#define SUPREME (MAX_LEVEL - 2)
#define DEITY (MAX_LEVEL - 3)
#define GOD (MAX_LEVEL - 4)
#define IMMORTAL (MAX_LEVEL - 5)
#define DEMI (MAX_LEVEL - 6)
#define ANGEL (MAX_LEVEL - 7)
#define AVATAR (MAX_LEVEL - 8)
#define HERO LEVEL_HERO


/*
 * Structure types.
 */

typedef struct        time_info_data                TIME_INFO_DATA;
typedef struct        weather_data                WEATHER_DATA;



/*
 * String and memory management parameters.
 */
#define MAX_STRING_LENGTH         4608
#define MAX_INPUT_LENGTH          1024


/*
 * death and player killing
 */
#define PK_SLAIN                (A)
#define        PK_KILLER                (B)
#define        PK_VIOLENT                (C)
#define PK_GHOST                (D)
#define        PK_THIEF                (E)

#define PK_MIN_LEVEL 5

#define        GHOST_TIME                3  * PULSE_MOBILE

#define        PK_TIME_SLAIN                7  * PULSE_MOBILE
#define        PK_TIME_KILLER                10 * PULSE_MOBILE
#define        PK_TIME_VIOLENT                5 * PULSE_MOBILE
#define        PK_TIME_THIEF                15 * PULSE_MOBILE

#define        MAX_DEATH_TIME                6 * PULSE_MOBILE

#define        IS_VIOLENT( ch )        (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_VIOLENT ))
#define        IS_KILLER( ch )                (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_KILLER ))
#define        IS_SLAIN( ch )                (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_SLAIN ))
#define        IS_GHOST( ch )                (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_GHOST ))
#define        IS_THIEF( ch )                (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_THIEF ))
#define IS_BLOODY( ch )         (!ch->is_npc( ) && IS_SET( ch->getPC( )->PK_flag, PK_THIEF|PK_KILLER|PK_VIOLENT ))

#define        REMOVE_VIOLENT( ch )        REMOVE_BIT( ch->getPC( )->PK_flag, PK_VIOLENT )
#define        REMOVE_KILLER( ch )        REMOVE_BIT( ch->getPC( )->PK_flag, PK_KILLER )
#define        REMOVE_SLAIN( ch )        REMOVE_BIT( ch->getPC( )->PK_flag, PK_SLAIN )
#define        REMOVE_GHOST( ch )        REMOVE_BIT( ch->getPC( )->PK_flag, PK_GHOST )
#define        REMOVE_THIEF( ch )        REMOVE_BIT( ch->getPC( )->PK_flag, PK_THIEF )

#define        IS_DEATH_TIME( ch )        (!ch->is_npc( ) && ch->getPC( )->last_death_time > 0 )
#define FIGHT_DELAY_TIME           20 // в секундах!
/*---------------------------------------------------------------------------*/


#define PULSE_MOBILE                  (dreamland->getPulseMobile( ))
#define PULSE_TICK                  (dreamland->getPulseTick( ))


/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                    2
#define SUN_SET                            3

#define SKY_CLOUDLESS                    0
#define SKY_CLOUDY                    1
#define SKY_RAINING                    2
#define SKY_LIGHTNING                    3

struct        time_info_data
{
    int                hour;
    int                day;
    int                month;
    int                year;
};

struct        weather_data
{
    int                mmhg;
    int         avg_mmhg;
    int                change;
    int         change_;
    int                sky;
    int                sunlight;
};




/* general align */
#define ALIGN_NONE                -1
#define ALIGN_GOOD                1000
#define ALIGN_NEUTRAL                0
#define ALIGN_EVIL                -1000




/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                      0
#define DIR_EAST                      1
#define DIR_SOUTH                      2
#define DIR_WEST                      3
#define DIR_UP                              4
#define DIR_DOWN                      5
#define DIR_SOMEWHERE                        6


/*
 * Utility macros.
 */
#define URANGE(a, b, c)                ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

#define        GET_SEX(ch, male, neutral, female) ((ch->getSex( ) == SEX_MALE) ? (male) : \
                                       ((ch->getSex( ) == SEX_NEUTRAL) ? (neutral) : \
                                       (female)))
#define        GET_COUNT(var, s1, s2, s3) (((var % 10) == 1 && (var % 100) != 11) ? (s1) : (((var % 10) > 1 && (var % 10) < 5 && ((var % 100) < 11 || (var % 100) > 15)) ? (s2) : (s3)))

/*
 * Character macros.
 */
#define IS_AFFECTED(ch, sn)        (IS_SET((ch)->affected_by, (sn)))
#define CAN_DETECT(ch, sn)        (IS_SET((ch)->detection, (sn)))

#define IS_GOOD(ch)                ((ch)->getAlignment() >= 350)
#define IS_EVIL(ch)                ((ch)->getAlignment() <= -350)
#define IS_NEUTRAL(ch)                (!IS_GOOD(ch) && !IS_EVIL(ch))
#define ALIGNMENT(ch)           (IS_GOOD(ch) ? N_ALIGN_GOOD : IS_EVIL(ch) ? N_ALIGN_EVIL : N_ALIGN_NEUTRAL)

#define IS_AWAKE(ch)                (ch->position > POS_SLEEPING)

#define MOUNTED(ch)        ((ch->mount && ch->riding) ?  ch->mount : NULL)
#define RIDDEN(ch)        ((ch->mount && !ch->riding) ?  ch->mount : NULL)

#define DIGGED( ch )            (!ch->is_npc( ) && IS_SET(ch->act, PLR_DIGGED))
#define IS_VAMPIRE(ch)        (!ch->is_npc() && IS_SET((ch)->act , PLR_VAMPIRE))
#define IS_HARA_KIRI(ch) (IS_SET((ch)->act , PLR_HARA_KIRI))

#define HEALTH(ch) ((ch)->hit * 100 / max(1, (ch)->max_hit.getValue( )))

#define IS_BLOODLESS(ch) ( IS_SET(ch->form, FORM_UNDEAD) || IS_SET(ch->form, FORM_CONSTRUCT) || IS_SET(ch->form, FORM_MIST) ) 

/*
 * Object macros.
 */
#define IS_OBJ_STAT(obj, stat)        (IS_SET((obj)->extra_flags, (stat)))


/*
 * Global variables.
 */
extern                Character          *        char_list;
extern                Character          *        newbie_list;
extern                Object          *        object_list;

extern                TIME_INFO_DATA                time_info;
extern                WEATHER_DATA                weather_info;




#endif
