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

#ifndef __MINGW32__
#include <netinet/in.h>
#endif

#include <string>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pointer.h"
#include "xmlstreamable.h"
#include "globalbitvector.h"

#include "fenia/register-decl.h"
#include "grammar_entities.h"
#include "areabehavior.h"
#include "mobilespecial.h"
#include "helpmanager.h"
#include "autoflags.h"
#include "affectlist.h"

class NPCharacter;
class Character;
class Object;
class Room;
class RoomIndexData;
class XMLDocument;
class AreaBehavior;
typedef ::Pointer<XMLDocument> XMLDocumentPointer;
typedef map<DLString, DLString> Properties;


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
#define MAX_LEVEL                   110
#define LEVEL_HERO                   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL                   (MAX_LEVEL - 8)
#define LEVEL_MORTAL               100

#define GROUP_RANGE                8

#define IMPLEMENTOR                MAX_LEVEL
#define        CREATOR                        (MAX_LEVEL - 1)
#define SUPREME                        (MAX_LEVEL - 2)
#define DEITY                        (MAX_LEVEL - 3)
#define GOD                        (MAX_LEVEL - 4)
#define IMMORTAL                (MAX_LEVEL - 5)
#define DEMI                        (MAX_LEVEL - 6)
#define ANGEL                        (MAX_LEVEL - 7)
#define AVATAR                        (MAX_LEVEL - 8)
#define HERO                        LEVEL_HERO


/*
 * Structure types.
 */

typedef struct        extra_descr_data        EXTRA_DESCR_DATA;
typedef struct        obj_index_data                OBJ_INDEX_DATA;
typedef struct        mob_index_data                MOB_INDEX_DATA;
typedef struct        exit_data                EXIT_DATA;
typedef struct        extra_exit_data        EXTRA_EXIT_DATA;
typedef struct        kill_data                KILL_DATA;
typedef struct        reset_data                RESET_DATA;
typedef struct        time_info_data                TIME_INFO_DATA;
typedef struct        weather_data                WEATHER_DATA;
typedef struct  auction_data            AUCTION_DATA; 



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
#define        MAX_OBJ_LOOT_KILLER        3

#define        GHOST_TIME                3  * PULSE_MOBILE

#define        PK_TIME_SLAIN                7  * PULSE_MOBILE
#define        PK_TIME_SLAIN_NOCLAN        20 * PULSE_MOBILE
#define        PK_TIME_KILLER                15 * PULSE_MOBILE
#define        PK_TIME_VIOLENT                10 * PULSE_MOBILE
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



#undef ANATOLIA_MACHINE

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





/*
 * A kill structure (indexed by level).
 */
struct        kill_data
{
    int                number;
    int                killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/


/* general align */
#define ALIGN_NONE                -1
#define ALIGN_GOOD                1000
#define ALIGN_NEUTRAL                0
#define ALIGN_EVIL                -1000




/* dice */
#define DICE_NUMBER                        0
#define DICE_TYPE                        1
#define DICE_BONUS                        2

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




/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * auction data
 */
struct  auction_data
{
    auction_data( );

    Object  * item;   /* a pointer to the item */
    Character * seller; /* a pointer to the seller - which may NOT quit */
    Character * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    int         startbet;
    int      going;  /* 1,2, sold */
    int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct        mob_index_data
{
    mob_index_data( );

    MOB_INDEX_DATA *        next;
    ProgWrapper<SPEC_FUN> spec_fun;
    int                vnum;
    int                group;
    bool                new_format;
    int                count;
    int                killed;
    char *                player_name;
    char *                short_descr;
    char *                long_descr;
    char *                description;
    int                act;
    int                affected_by;
    int                add_affected_by;
    int                detection;
    int                alignment;
    int                level;
    int                hitroll;
    int                        hit[3];
    int                        mana[3];
    int                damage[3];
    int                ac[4];
    int                 dam_type;
    int                off_flags;
    int                imm_flags;
    int                res_flags;
    int                vuln_flags;
    int                start_pos;
    int                default_pos;
    int                sex;
    char *                race;
    int                wealth;
    int                form;
    int                parts;
    int                size;
    char *                material;
    GlobalBitvector     practicer;
    GlobalBitvector religion;
    Grammar::Number     gram_number;
    XMLDocumentPointer behavior;
    Scripting::Object *wrapper;
    AreaIndexData *                area;
    DLString smell;
    Properties properties;
};


/*
 * Extra description data for a room or object.
 */
struct        extra_descr_data
{
    EXTRA_DESCR_DATA *next;        /* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};


typedef list<Object *> ObjectList;

/*
 * Prototype for an object.  *OID*
 */
struct        obj_index_data
{
    OBJ_INDEX_DATA *        next;
    EXTRA_DESCR_DATA *        extra_descr;
    AffectList        affected;
    bool                new_format;
    char *                name;
    char *                short_descr;
    char *                description;
    int                vnum;
    int                reset_num;
    char *                material;
    int                item_type;
    int               extra_flags;
    int               wear_flags;
    int                level;
    int                 condition;
    int                count;
    int                weight;
    int                        cost;
    int                        value[5];
    int                 limit;
    Grammar::MultiGender gram_gender;
    XMLDocumentPointer behavior;
    Scripting::Object *wrapper;
    AreaIndexData *                area;
    DLString smell;
    DLString sound;
    Properties properties;
    ObjectList instances;
};



/*
 * Exit data.
 */
struct        exit_data
{
        union
        {
                Room *        to_room;
                int        vnum;
        } u1;
        int                exit_info;
        int                exit_info_default;
        int                key;
        char *                keyword;
        char *                description;
        char *          short_descr;
        EXIT_DATA *        next;
        int                orig_door;
        int                level;

        /** Resolve u1 from a virtual number to the real room. */
        void resolve(); 

        /** Restore exit flags to their original values. */
        void reset();

        exit_data *create(); // Implemented in loadsave plugin.
};

struct        extra_exit_data
{
        extra_exit_data();
        virtual ~extra_exit_data();
        union
        {
                Room *        to_room;
                int        vnum;
        } u1;
        int                                exit_info;
        int                                exit_info_default;
        int                key;
        int                                moving_from;
        int                                moving_mode_from;
        int                                moving_to;
        int                                moving_mode_to;
        int                                max_size_pass;
        char *                keyword;
        char *                short_desc_from;
        char *                short_desc_to;
        char *                description;
        char *                room_description;
        int                level;

        /** Resolve u1 from a virtual number to the real room. */
        void resolve(); 

        /** Restore exit flags to their original values. */
        void reset();

        extra_exit_data *create(); // Implemented in loadsave plugin.
};


/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct        reset_data
{
    RESET_DATA *        next;
    char                command;
    int                arg1;
    int                arg2;
    int                arg3;
    int                arg4;
};



/*
 * Area definition.
 */
struct Area;
struct AreaIndexData {
    AreaIndexData();

    Area *create();

    char *name;
    char *altname;
    char *authors;
    char *credits;
    char *translator;
    char *speedwalk;
    int low_range;
    int high_range;
    int min_vnum;
    int max_vnum;
    unsigned long count;
    char *resetmsg;
    int area_flag;
    struct area_file *area_file;
    XMLPersistentStreamable<AreaBehavior> behavior;
    HelpArticles helps;

    /*OLC*/
    int security;
    int vnum;
    map<int, RoomIndexData *> roomIndexes;

    // FIXME: support multiple named instances.
    Area *area;
};
struct Area {
    Area();

    bool empty;
    int age;
    int nplayer;
    int area_flag;
    map<int, Room *> rooms;

    AreaIndexData *pIndexData;
};

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

#define IS_GOOD(ch)                ((ch)->alignment >= 350)
#define IS_EVIL(ch)                ((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)                (!IS_GOOD(ch) && !IS_EVIL(ch))
#define ALIGNMENT(ch)           (IS_GOOD(ch) ? N_ALIGN_GOOD : IS_EVIL(ch) ? N_ALIGN_EVIL : N_ALIGN_NEUTRAL)

#define IS_AWAKE(ch)                (ch->position > POS_SLEEPING)

#define MOUNTED(ch)        ((ch->mount && ch->riding) ?  ch->mount : NULL)
#define RIDDEN(ch)        ((ch->mount && !ch->riding) ?  ch->mount : NULL)

#define DIGGED( ch )            (!ch->is_npc( ) && IS_SET(ch->act, PLR_DIGGED))
#define IS_VAMPIRE(ch)        (!ch->is_npc() && IS_SET((ch)->act , PLR_VAMPIRE))
#define IS_HARA_KIRI(ch) (IS_SET((ch)->act , PLR_HARA_KIRI))

#define HEALTH(ch) ((ch)->hit * 100 / max(1, (ch)->max_hit.getValue( )))

#define IS_BLOODLESS(ch) (IS_SET( ch->form, FORM_UNDEAD ) || IS_SET( ch->form, FORM_CONSTRUCT ) ) 

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

extern                AUCTION_DATA          *        auction;

typedef set<Room *> RoomSet;

/** A small collection of rooms with affects on them, to avoid going through the whole list in updates. */
extern RoomSet roomAffected;

extern                KILL_DATA                kill_table        [];
extern                TIME_INFO_DATA                time_info;
extern                WEATHER_DATA                weather_info;

typedef map<int, RoomIndexData *> RoomIndexMap;

/** Map of all room prototypes by vnum, for quick access. */
extern RoomIndexMap roomIndexMap;

typedef vector<Room *> RoomVector;
extern RoomVector roomInstances;

typedef vector<Area *> AreaVector;
extern AreaVector areaInstances;

typedef vector<AreaIndexData *> AreaIndexVector;
extern AreaIndexVector areaIndexes;

extern                int                        top_vnum_room;
extern                int                        top_vnum_mob;
extern                int                        top_vnum_obj;
extern                int                        top_obj_index;
extern                int                        top_mob_index;



#endif
