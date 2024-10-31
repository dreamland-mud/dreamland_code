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
#include <jsoncpp/json/json.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pointer.h"
#include "xmlstreamable.h"
#include "globalbitvector.h"

#include "fenia/register-decl.h"
#include "grammar_entities.h"
#include "areabehavior.h"
#include "flags.h"
#include "enumeration.h"
#include "mobilespecial.h"
#include "helpmanager.h"
#include "autoflags.h"
#include "affectlist.h"
#include "clanreference.h"
#include "areaquest.h"
#include "dl_strings.h"
#include "dl_math.h"
#include "dl_ctype.h"
#include "logstream.h"


#include "mobilefactory.h"

class NPCharacter;
class Character;
class Object;
class Room;
class RoomIndexData;
class AreaIndexData;
class XMLDocument;
class AreaBehavior;
typedef ::Pointer<XMLDocument> XMLDocumentPointer;
class AreaQuest;
struct extra_exit_data;
struct obj_index_data;

#define        MAX_KEY_HASH                 1024

extern char str_empty[1];

extern obj_index_data         * obj_index_hash          [MAX_KEY_HASH];

extern int        top_area; // Keep tracks of all areas loaded; used to assign area's vnum field.

// MOC_SKIP_BEGIN
struct area_file {
    struct area_file *next;
    struct AreaIndexData *area;
    DLString file_name;
};

extern struct area_file * area_file_list;
struct area_file * new_area_file(const char *name);
// MOC_SKIP_END


obj_index_data *        get_obj_index        ( int vnum );
RoomIndexData *        get_room_index        ( int vnum );
Room * get_room_instance(int vnum);
AreaIndexData * get_area_index(const DLString &filename);


char *        str_dup                ( const char *str );
void        free_string        ( char *pstr );


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

typedef struct        obj_index_data                OBJ_INDEX_DATA;
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
 * Extra description data for a room or object.
 */
struct ExtraDescription {
    // Keyword in look/examine, contains keywords in all languages.
    DLString keyword; 

    // What to see
    XMLMultiString description; 
};

struct ExtraDescrList: public list<ExtraDescription *> {
    /** Return extra descr that matches the given keyword. */
    ExtraDescription *find(const DLString &keyword) const;

    ExtraDescription *findUnstrict(const DLString &keyword) const;

    /** 
     * Remove matching descr from list and free its memory. 
     * Returns true if found.
     */
    bool findAndDestroy(const DLString &keyword);

    /** Destroy all elements and clear the list. */
    void deallocate();
};

typedef list<Object *> ObjectList;

/*
 * Prototype for an object.  *OID*
 */
struct        obj_index_data
{
    obj_index_data();
    virtual ~obj_index_data();

    OBJ_INDEX_DATA *        next;
    ExtraDescrList extraDescriptions;
    AffectList        affected;

    // Replace 'name' with multi-lang keywords.
    XMLMultiString keyword;
    XMLMultiString   short_descr;
    XMLMultiString   description;
    XMLMultiString smell;
    XMLMultiString sound;

    int                vnum;
    int                reset_num;
    DLString material;
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
    ObjectList instances;

    GlobalBitvector behaviors;
    Json::Value props;

    /** Return props value for the key (props[key] or props["xxx"][key]). */
    DLString getProperty(const DLString &key) const;

    const char * getDescription( lang_t lang ) const;
    const char * getShortDescr( lang_t lang ) const;
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

        XMLMultiString keyword;
        XMLMultiString short_descr;
        XMLMultiString description;

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
        int                                max_size_pass;


        XMLMultiString keyword;
        XMLMultiString short_desc_from;
        XMLMultiString short_desc_to;
        XMLMultiString description;
        XMLMultiString room_description;

        int                level;

        XMLMultiString msgLeaveRoom;
        XMLMultiString msgLeaveSelf;
        XMLMultiString msgEntryRoom;
        XMLMultiString msgEntrySelf;

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
    reset_data();

    char                command;
    int                arg1;
    int                arg2;
    int                arg3;
    int                arg4;

    Flags flags;
    Enumeration rand;
    int bestTier;
    vector<int> vnums;
};

/*
 * Area definition.
 */
struct Area;
struct AreaIndexData {
    AreaIndexData();

    Area *create();

    DLString getName(char gcase = '1') const;

    XMLMultiString name; // main area name in all languages
    XMLMultiString altname; // alternative names for this area
    DLString authors;
    DLString translator;
    XMLMultiString speedwalk;
    int low_range;
    int high_range;
    int min_vnum;
    int max_vnum;
    unsigned long count;
    XMLMultiString resetMessage;
    int area_flag;
    struct area_file *area_file;
    XMLPersistentStreamable<AreaBehavior> behavior;
    HelpArticles helps;

    /*OLC*/
    int security;
    int vnum;
    bool changed;
    map<int, RoomIndexData *> roomIndexes;

    Scripting::Object *wrapper;

    list<XMLPointer<AreaQuest>> quests;
    map<int, AreaQuest *> questMap;

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
#define IS_MISOGI(ch) (IS_SET((ch)->act , PLR_MISOGI))

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

extern                AUCTION_DATA          *        auction;

typedef set<Room *> RoomSet;

/** A small collection of rooms with affects on them, to avoid going through the whole list in updates. */
extern RoomSet roomAffected;

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

#endif
