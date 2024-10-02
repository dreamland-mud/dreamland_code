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
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT                           *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Mandrake}        canpunar@rorqual.cc.metu.edu.tr    *        
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
#include <jsoncpp/json/json.h>

#include "lastlogstream.h"
#include "logstream.h"
#include "profiler.h"
#include "grammar_entities_impl.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "commonattributes.h"
#include "core/behavior/behavior_utils.h"
#include "../loadsave/behavior_utils.h"
#include "mobilebehavior.h"
#include "affecthandler.h"
#include "skill.h"
#include "skillcommand.h"
#include "clanreference.h"
#include "pcharactermanager.h"
#include "room.h"
#include "roomutils.h"
#include "race.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "liquid.h"
#include "desire.h"

#include "update.h"
#include "update_areas.h"
#include "weather.h"

#include "configurable.h"
#include "dreamland.h"
#include "interp.h"
#include "stats_apply.h"

#include "act_move.h"
#include "descriptor.h"
#include "fight.h"
#include "damage_impl.h"
#include "magic.h"
#include "../anatolia/handler.h"
#include "save.h"
#include "wiznet.h"
#include "act.h"
#include "material.h"
#include "merc.h"

#include "vnum.h"
#include "def.h"
#include "messengers.h"

CLAN(battlerager);
CLAN(hunter);
PROF(vampire);
GSN(ambush);
GSN(arrow);
GSN(bonedagger);
GSN(caltraps);
GSN(chill_touch);
GSN(dark_shroud);
GSN(path_find);
GSN(protection_heat);
GSN(spear);
GSN(spellbane);

#define PULSE_SAVING                  ( 5 * dreamland->getPulsePerSecond( ))
#define PULSE_WATER_FLOAT          ( 4 * dreamland->getPulsePerSecond( ))
#define PULSE_RAFFECT                  ( 3 * PULSE_MOBILE + 1)
#define PULSE_AREA                  (110 * dreamland->getPulsePerSecond( )) /* 97 saniye */
#define PULSE_TRACK                  ( 7 * dreamland->getPulsePerSecond( ))


// A set of update settings defined in config/update.json.
Json::Value config;
CONFIGURABLE_LOADED(config, update)
{
    config = value;
}

/*
 * Local functions.
 */
void aggr_update( );
void auction_update( );
void room_affect_update( );
void check_reboot( );

void light_update( PCharacter * );
void wield_update( Character *ch );
void lantern_update( Character *ch );
void idle_update( PCharacter *ch );
void char_update_affects( Character *ch );


/* used for saving */

//#define MAX_VNUM ( 0x08000 )
#define MAX_VNUM ( 110000 )

static Room * saving_table[MAX_VNUM];
static int saving_size = 655;   // because 10 savings per tick.
// 10 * 5 ticks - full saving
static int saving_position = 0;

void room_to_save( Room * room )
{
    if ( room == 0 )
        return;

    if ( room->vnum < 0
            || room->vnum >= MAX_VNUM )
    {
        bug( "Saving utility: vnum %d over save space!" , room->vnum );
        return;
    }

    saving_table[room->vnum] = room;
}

void room_to_save( Character *ch )
{
    if (ch->is_npc( ) && !IS_CHARMED(ch))
        room_to_save( ch->in_room );
}

void room_to_save( Object *obj )
{
    Character *carrier = obj->getCarrier( );
    Room *room = obj->getRoom( );

    if (carrier)
        room_to_save( carrier );
    else
        room_to_save( room );
}


void room_saving( void )
{
    ProfilerBlock profiler("room_saving", 50);

    if ( saving_position >= MAX_VNUM )
        saving_position = 0;

    int i_max = min( saving_position + saving_size, MAX_VNUM );

    for ( int i = saving_position; i < i_max; i++ )
        if ( saving_table[i] != 0 )
        {
            save_items( saving_table[i] );
            save_mobs( saving_table[i] );

            saving_table[i] = 0;
        }

    saving_position = i_max;
}

bool is_bright_for_vampire( Character *ch )
{
    if (weather_info.sunlight == SUN_DARK)
        return false;
        
    if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
        return false;

    if (!RoomUtils::isOutside(ch))
        return false;

    if(ch->in_room->getSectorType() == SECT_UNDERWATER)
        return false;
        
    if (ch->isAffected(gsn_dark_shroud)) 
        return false;
    return true;
}

bool is_room_saving( Room * room )
{
    if ( room == 0 )
        return false;

    if ( room->vnum < 0
            || room->vnum >= MAX_VNUM )
    {
        bug( "Saving utility: checking vnum %d over save space!" , room->vnum );
        return false;
    }

    return saving_table[room->vnum] != 0;
}

void remove_room_saving( Room * room )
{
    if ( room == 0 )
        return;

    if ( room->vnum < 0
            || room->vnum >= MAX_VNUM )
    {
        bug( "Saving utility: remove vnum %d over save space!" , room->vnum );
        return ;
    }

    saving_table[room->vnum] = 0;
}

static bool afprog_spec(Character *ch)
{
    bool rc = false;

    for (auto &paf: ch->affected.findAllWithHandler())
        if (paf->type->getAffect())
            if (paf->type->getAffect( )->onSpec(SpellTarget::Pointer(NEW, ch), paf))
                rc = true;

    return rc;
}

static bool mprog_special( Character *ch )
{
    if (behavior_trigger(ch, "Spec", "C", ch))
        return true;

    FENIA_CALL( ch, "Spec", "" );
    FENIA_NDX_CALL( ch->getNPC( ), "Spec", "C", ch );
    BEHAVIOR_CALL( ch->getNPC( ), spec );

    if (afprog_spec(ch))
        return true;
    
    if (ch->is_npc( ) && !IS_CHARMED(ch)) {
        if (*(ch->getNPC( )->spec_fun) != 0 
            && (*(ch->getNPC( )->spec_fun))( ch->getNPC( ) ))
            return true;
    }

    if (!ch->is_npc())
        gprog("onSpec", "C", ch);

    return false;
}

/* 
 * Autonomous mobile action 
 */
void mobile_update( ) 
{
    ProfilerBlock profiler("mobile_update", 50);
    Character *ch, *ch_next;

    for (ch = char_list; ch; ch = ch_next) {
        ch_next = ch->next;

        try {
            wield_update( ch );

            mprog_special( ch );
        } catch (const VictimDeathException &) {
        }
    }
}

static bool mprog_area( Character *ch )
{
    if (behavior_trigger(ch, "Area", "C", ch))
        return true;
        
    FENIA_CALL( ch, "Area", "" );
    FENIA_NDX_CALL( ch->getNPC( ), "Area", "C", ch );
    BEHAVIOR_CALL( ch->getNPC( ), area );
    return false;
}

// Trying to catch "You are 100500 years old" bug 
// by checking and reporting player ages outside of reasonable limits.
static void debug_player_age(PCharacter *ch)
{
    int age = ch->age.getTrueYears();

    if (age < 17) {
        LogStream::sendWarning() << "AGE_DEBUG: Low age for " << ch->getName() << " true_years=" << age
            << " years="  << ch->age.getYears() << " played=" << ch->age.played << " true_played=" << ch->age.true_played << " logon=" << ch->age.logon
            << " time=" << ch->age.getTime() << " true_time=" << ch->age.getTrueTime() 
            << " current_time=" << dreamland->getCurrentTime() << " " << Date::getCurrentTimeAsString() << endl;
        return;
    }

    if (age > 30 && ch->get_trust() < LEVEL_IMMORTAL) {
        LogStream::sendWarning() << "AGE_DEBUG: High age for " << ch->getName() << " true_years=" << age
            << " years="  << ch->age.getYears() << " played=" << ch->age.played << " true_played=" << ch->age.true_played << " logon=" << ch->age.logon
            << " time=" << ch->age.getTime() << " true_time=" << ch->age.getTrueTime() 
            << " current_time=" << dreamland->getCurrentTime() << " " << Date::getCurrentTimeAsString() << endl;
        return;
    }
}

/*
 * Update all chars, including mobs.
 */
void char_update( )
{
    ProfilerBlock profiler("char_update", 50);
    Character *ch;
    Character *ch_next;

    for ( ch = char_list; ch != 0; ch = ch_next )
    {
        ch_next = ch->next;

        if (ch->in_room == 0)
            continue;

        // Special mobs don't get their HP or position updated automatically.
        bool noupdate = ch->is_npc() && IS_SET(ch->getNPC()->act, ACT_NOUPDATE);

        // Reset hunters path find.
        if (!ch->is_npc() && ch->getClan( ) == clan_hunter)
        {
            if (number_percent() < gsn_path_find->getEffective( ch ) )
            {
                ch->endur += (gsn_path_find->getEffective( ch ) / 2);
                gsn_path_find->improve( ch, true );
            }
            else
            {
                gsn_path_find->improve( ch, false );
            }
        }

        // Reset native sneak outside of battle.
        if ( !(ch->fighting) && !IS_AFFECTED(ch,AFF_SNEAK)
                && (ch->getRace( )->getAff( ).isSet( AFF_SNEAK )) && !MOUNTED(ch) )
        {
            ch->pecho("Ты пытаешься двигаться незаметно.");
            SET_BIT(ch->affected_by ,AFF_SNEAK);
            room_to_save( ch );
        }

        // Reset native hide outside of battle.
        if ( !(ch->fighting) && !IS_AFFECTED(ch,AFF_HIDE)
                && (ch->getRace( )->getAff( ).isSet( AFF_HIDE )) && !MOUNTED(ch) )
        {
            ch->pecho("Ты прячешься обратно в тень.");
            room_to_save( ch );
        }

        // Reset race affect bits. 
        SET_BIT(ch->affected_by, ch->getRace( )->getAff( ) );

        // Recover from 'stunned' state if HP is ok.
        if (ch->position == POS_STUNNED && !noupdate) {
            update_pos( ch );
            room_to_save( ch );
        }
        
        if (!ch->is_npc( ))
            lantern_update( ch );

        if (!ch->is_npc( ) && !dreamland->hasOption( DL_BUILDPLOT ))
            try {
                for (int i = 0; i < desireManager->size( ); i++)
                    desireManager->find( i )->update( ch->getPC( ) );
            } catch (const VictimDeathException &) {
                continue;
            }
        
        if (!ch->is_npc( ) && !ch->is_immortal( ) && !ch->getPC( )->switchedTo) {
            if (++ch->timer == 12)
                idle_update( ch->getPC( ) );
        }

        if (ch->is_npc( ) && ch->timer > 0) {
            if (--ch->timer == 0) {
                if (IS_SET(ch->act, ACT_UNDEAD))
                    oldact("$c1 развалил$gось|ся|ась на куски.", ch, 0, 0, TO_ROOM);
                else
                    oldact("$c1 исчезает.", ch, 0, 0, TO_ROOM);
                extract_char( ch );
                continue;
            }
        }

        if (!ch->isDead() && !noupdate)
            char_update_affects( ch );

        if (!ch->is_npc( )
                && ch->getClan( ) == clan_battlerager
                && !ch->isAffected( gsn_spellbane ))
        {
            interpret_raw( ch, "spellbane" );
        }

        // Coup de grace.
        if ((ch->position == POS_INCAP && number_range(0, 1) == 0)
            || ch->position == POS_MORTAL
            || (ch->position == POS_DEAD && !ch->isDead()))
        {
            if (!noupdate) {
                room_to_save( ch );
                ch->pecho("Ты умираешь от неизлечимых ран.");
                ch->recho("%^C1 умирает от неизлечимых ран.", ch);
                rawdamage( ch, ch, DAM_NONE, 1, false, "wounds" );
            }
        }

        // Wtf?
        if (ch->position <= POS_STUNNED && ch->hit > 0 && !noupdate) {
            room_to_save( ch );
            update_pos( ch );
        }
    }
}

void char_update_prog( )
{
    ProfilerBlock profiler("char_update_prog", 50);
    Character *ch;
    Character *ch_next;

    for ( ch = char_list; ch != 0; ch = ch_next )
    {
        ch_next = ch->next;

        if (ch->in_room == 0)
            continue;

        if (mprog_area( ch ))
            continue;
    }
}

void char_update_autosave()
{
    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    ProfilerBlock profiler("char_update_autosave", 50);

    static time_t last_save_time = -1;
    int autoquit_timer = config["autoquit"].asInt();
    int lostlink_timer = config["lostlink"].asInt();

    if (last_save_time == -1 || dreamland->getCurrentTime( ) - last_save_time > 60)
    {
        Character *ch, *ch_next;

        last_save_time = dreamland->getCurrentTime( );

        for (ch = char_list; ch != 0; ch = ch_next)
        {
            ch_next = ch->next;
            if (ch->is_npc())
                continue;

            PCharacter *pch = ch->getPC();
            pch->save();
            debug_player_age(pch);

            if (pch->isCoder())
                continue;

            if (autoquit_timer > 0 && pch->timer > autoquit_timer) {
                pch->getAttributes( ).getAttr<XMLStringAttribute>( "quit_flags" )->setValue( "auto" );
                interpret_raw(pch, "quit", "");
                continue;
            }

            if (lostlink_timer > 0 && !pch->desc && pch->timer > lostlink_timer) {
                pch->getAttributes( ).getAttr<XMLStringAttribute>( "quit_flags" )->setValue( "auto" );
                interpret_raw(pch, "quit", "");
                continue;
            }
        }
    }
}

static void rafprog_spec(Room *room)
{
    for (auto &paf: room->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect()->onSpec(SpellTarget::Pointer(NEW, room), paf);
}

void diving_update( )
{
    ProfilerBlock profiler("diving_update", 50);
   
    for (auto &r: roomInstances) {
        try {
            FENIA_VOID_CALL(r, "DiveUpdate", "");

            behavior_trigger(r, "DiveUpdate", "R", r);

            if (behavior_trigger(r, "Spec", "R", r))
                continue;

            rafprog_spec(r);
            
            FENIA_VOID_CALL(r, "Spec", "");

        } catch (const VictimDeathException &ex) {
            // DO NOTHING
        }
    }
}

static bool oprog_spec( Object *obj )
{
    if (behavior_trigger(obj, "Spec", "O", obj))
        return true;
        
    FENIA_CALL(obj, "Spec", "");
    FENIA_NDX_CALL(obj, "Spec", "O", obj);

    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect( )->onSpec(SpellTarget::Pointer(NEW, obj), paf))
            return true;

    return false;
}

void water_float_update( )
{
    ProfilerBlock profiler("water_float_update", 50);
    Object *obj_next;
    Object *obj;

    for (obj = object_list; obj != 0; obj = obj_next) {
        obj_next = obj->next;

        try {
            if (oprog_spec( obj ))
                continue;
        } catch (const VictimDeathException &) {
            // DO NOTHING
        }
        
        if (!obj->in_room)
            continue;
        
        if (!RoomUtils::isWater( obj->in_room ))
            continue;

        if (IS_SET( obj->extra_flags, ITEM_NOPURGE ))
            continue;

        if (obj->pIndexData->limit > 0)
            continue;

        // Don't drown items that reset in this location.
        if (obj->reset_room == obj->in_room->vnum)
            continue;
        
        if (obj->water_float == -2) {
            if (obj->may_float( ) || material_swims( obj ) == SWIM_ALWAYS)
                obj->water_float = -1;
            else
                obj->water_float = obj->floating_time( );
        }
        
        if (obj->item_type == ITEM_DRINK_CON && !IS_SET(obj->value4(), DRINK_CLOSED)) {
            obj->in_room->echo( POS_RESTING, "%1$^O1 дела%1$nет|ют пузыри на поверхности %2$N2.", obj, obj->in_room->getLiquid()->getShortDescr( ).c_str( ) );

            obj->value1(URANGE( 1, obj->value1() + 8, obj->value0() ));
            obj->water_float = obj->value0() - obj->value1();
            obj->value2(obj->in_room->getLiquid());
            room_to_save( obj );
        }

        if (obj->water_float > 0)
            obj->water_float--;

        if (obj->water_float == 0) {
            if (obj->item_type == ITEM_CORPSE_NPC
                    || obj->item_type == ITEM_CORPSE_PC
                    || obj->item_type == ITEM_CONTAINER)
            {
                obj->in_room->echo( POS_RESTING, "%1$^O1 тон%1$nет|ут в %2$N6, оставляя лишь несколько пузырьков.", obj, obj->in_room->getLiquid()->getShortDescr( ).c_str( ) );
            }
            else
                obj->in_room->echo( POS_RESTING, "%1$^O1 тон%1$nет|ут в %2$N6.", obj, obj->in_room->getLiquid()->getShortDescr( ).c_str( ) );

            extract_obj( obj );
        }
    }
}

/* 
 * check if object is on its reset place
 */
static bool reset_check_obj( Object *obj )
{
    int mobVnum = -1;
    int v = obj->pIndexData->vnum;
    Room * room = obj->getRoom( );
    NPCharacter *mob;

    if (obj->carried_by && obj->carried_by->is_npc( ))
        mob = obj->carried_by->getNPC( );
    else
        mob = NULL;
    
    for (auto &r: room->pIndexData->resets)
        switch (r->command) {
        case 'O':
            if (r->arg1 == v && obj->in_room) 
                return true;
            break;
        case 'P':
            if (r->arg1 == v && obj->in_obj && r->arg3 == obj->in_obj->pIndexData->vnum) 
                return true;
            break;
        case 'M':
            mobVnum = r->arg1;
            break;
        case 'E':
        case 'G':
            if (mob && mob->pIndexData->vnum == mobVnum) 
                return true;
            break;
        }
    
    if (mob && mob->pIndexData->area == obj->pIndexData->area)
        return true;

    return false;
}

/* TODO: make this method of class 'Object' */
static Object * obj_get_container( Object *obj )
{
    Object *o;

    for (o = obj->in_obj; o; o = o->in_obj)
        ;

    return o;
}

/*
 * update progs for key: set rot timers
 * returns true if timer altered
 */
static bool oprog_update_key( Object *obj )
{
    Character *carrier = obj->getCarrier( );
    Object *container = obj_get_container( obj );
    
    /* ground */
    if (carrier == 0) {
        if (obj->value1() == 0) { // 0: never rot
            obj->timer = 0;
            return true;
        }
        
        // always allow keys in the mansions
        if (IS_SET(obj->getRoom( )->room_flags, ROOM_MANSION)) 
            return false;

        // don't touch keys in the pits or corpses or keyrings.
        if (container && IS_PIT(container))
            return false;
        if (container 
                && (container->item_type == ITEM_CORPSE_PC
                    || container->item_type == ITEM_CORPSE_NPC))
            return false;

        if (obj->in_obj && obj->in_obj->item_type == ITEM_KEYRING)
            return false;        

        if (reset_check_obj( obj )) { // object is in reset place
            obj->timer = 0;
            return true;
        }
        
        if (!obj->timer) {
            obj->timer = 60;      // disappear in 60 ticks
            return true;
        }

        return false;
    }

    /* inventory or bags */
    if (obj->value0() == 1) { // 1: never rot        
        obj->timer = 0;
        return true;
    }
    
    if (obj->value0() > 1) {// > 1: means TTL
        if (reset_check_obj( obj )) {
            obj->timer = 0;
            return true;
        }
        
        if (!obj->timer) {
            obj->timer = obj->value0();
            return true;
        }
            
        return false;
    }
    
    // 0: means rot on save, automatic for pc, checked for mobs

    if (carrier->is_npc( ) && !reset_check_obj( obj )) { // rot in wrong hands
        if (!obj->timer) {
            obj->timer = 10;
            return true;
        }
        
        return false;
    }

    return false;
}

static bool oprog_area( Object *obj )
{
    if (behavior_trigger(obj, "Area", "O", obj))
        return true;

    FENIA_CALL( obj, "Area", "" )
    FENIA_NDX_CALL( obj, "Area", "O", obj )
    BEHAVIOR_CALL( obj, area )
    return false;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
    ProfilerBlock profiler("obj_update", 50);
    Object *obj;
    Object *obj_next;
    Room *room;
    Character *carrier;
    list<Object *> extracted; // Keep track of all items that are about to die this round.

    for ( obj = object_list; obj != 0; obj = obj_next )
    {
        const char *message;

        if(!obj->pIndexData) {
            LogStream::sendError() << "obj_update aborted" << endl;
            return;
        }
            
        obj_next = obj->next;
        room = obj->getRoom( );
        carrier = obj->getCarrier( );
        
        if (!room)
            continue;

        if (obj->item_type == ITEM_KEY)
            if (oprog_update_key( obj ))
                room_to_save( obj );

        /* no limits on the floor inside 'mansions' */
        if (carrier == 0 && room && IS_SET(room->room_flags, ROOM_MANSION)
            && obj->pIndexData->limit != -1) 
        {

            obj->setProperty("extract", "Внезапно появляется злобная домоуправительница и подметает %1$O4 на совок!");
            extracted.push_back(obj);
            continue;
        }

        if (oprog_area( obj ))
            continue;        

        /* go through affects and decrement */
        AffectList affects = obj->affected.clone();
        for (auto paf_iter = affects.cbegin( ); paf_iter != affects.cend( ); paf_iter++) {
            Affect *paf = *paf_iter;

            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                    paf->level--;  /* spell strength fades with time */

                // Issue periodic message or action.
                if (!affects.hasNext(paf_iter) && paf->type->getAffect( )) 
                    paf->type->getAffect()->onUpdate(SpellTarget::Pointer(NEW, obj), paf );

                room_to_save( obj );
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                room_to_save( obj );

                if (!affects.hasNext(paf_iter))
                    if (paf->type->getAffect())
                        paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, obj), paf );


                affect_remove_obj( obj, paf, false );
            }
        }

        if (!obj->in_obj 
            && room->getSectorType() == SECT_DESERT
            && material_is_flagged( obj, MAT_MELTING ))
        {
            if (obj->isAffected(gsn_protection_heat))
                continue;
            
            if (carrier) {
                if (chance( 40 )) {
                    obj->setProperty("extract", "%1$^O1 та%1$nет|ют от жара.");
                    extracted.push_back(obj);
                    continue;
                }
            }
            else {
                if (chance( 50 )) {
                    obj->setProperty("extract", "%1$^O1 та%1$nет|ют от жара.");
                    extracted.push_back(obj);
                    continue;
                }
            }
        }
        
        if (obj->item_type == ITEM_POTION
            && !obj->in_obj
            && room->getSectorType() == SECT_DESERT
            && material_is_flagged( obj, MAT_FRAGILE ))
        {
            if (obj->isAffected(gsn_protection_heat))
                continue;
            
            if (carrier) {
                if (!carrier->is_npc( ) && chance( 20 )) {
                    obj->setProperty("extract", "%1$^O1 лопа%1$nется|ются от жары.");
                    extracted.push_back(obj);
                    continue;
                }
            }
            else {
                if (chance( 30 )) {
                    obj->setProperty("extract", "%1$^O1 разбива%1$nется|ются на мелкие осколки.");
                    extracted.push_back(obj);
                    continue;
                }
            }
        }

        // Time stops for auctioned items.
        if (auction->item == obj)
            continue;
            
        if ( obj->condition > -1
                && ( obj->timer <= 0
                     || --obj->timer > 0 ) )
        {
            if (obj->timer > 0) {
                room_to_save( obj );
            }

            continue;
        }

        switch ( obj->item_type )
        {
        default:
            message = "%1$^O1 превраща%1$nется|ются в пыль.";
            break;
        case ITEM_FOUNTAIN:
            message = "%1$^O1 высыха%1$nет|ют.";
            break;
        case ITEM_CORPSE_NPC:
            message = "%1$^O1 обраща%1$nется|ются в прах.";
            break;
        case ITEM_CORPSE_PC:
            message = "%1$^O1 обраща%1$nется|ются в прах.";
            break;
        case ITEM_FOOD:
            message = "%1$^O1 разлага%1$nется|ются.";
            break;
        case ITEM_POTION:
            message = "%1$^O1 испаря%1$nется|ются.";
            break;
        case ITEM_PORTAL:
            message = "%1$^O1 исчеза%1$nет|ют в дымке.";
            break;
        case ITEM_FURNITURE:
        case ITEM_TATTOO:
        case ITEM_CRAFT_TATTOO:
            message = "%1$^O1 исчеза%1$nет|ют.";
            break;
        case ITEM_CONTAINER:
            if (obj->contains)
                message = "%1$^O1 исчеза%1$nет|ют, рассыпая все содержимое.";
            else
                message = "%1$^O1 обраща%1$nется|ются в прах.";
            break;
        }

        /* Save the contents of the decaying obj. Always preserve everything inside
         * PC corpses and carried containers. In other cases, only save undestructible items.
         * TODO: this code, unlike 'sacrifice' command, is not recursive, so items 
         * within containers within containers are not rescued.
         */
        if (obj->contains) {
            Object *t_obj, *next_obj;
            Object *pit = find_pit_for_obj(obj);
            bool saveAll;

            if (obj->item_type == ITEM_CORPSE_PC)
                saveAll = true;
            else if (obj->item_type == ITEM_CONTAINER && obj->getCarrier())
                saveAll = true;
            else
                saveAll = false;

            for (t_obj = obj->contains; t_obj != 0; t_obj = next_obj) {
                next_obj = t_obj->next_content;

                if (!saveAll && !IS_OBJ_STAT(t_obj, ITEM_NOPURGE) && !IS_OBJ_STAT(t_obj, ITEM_NOSAC))
                    continue;

                obj_from_obj(t_obj);

                if (obj->in_obj) { /* in another object */
                    obj_to_obj(t_obj, obj->in_obj);
                } else if (obj->carried_by) { /* carried */
                    obj_to_char(t_obj, obj->carried_by);
                } else if (obj->in_room == 0) { /* destroy it */
                    extracted.push_back(t_obj);
                } else { /* to the pit */
                    if (pit == 0)
                        obj_to_room(t_obj, obj->in_room);
                    else
                        obj_to_obj(t_obj, pit);
                }
            }
        }

        obj->setProperty("extract", message);
        extracted.push_back(obj);
    }

    // Do the extract now that it's safe and we don't have to worry about obj->next->next
    // pointer becoming invalid.
    while (!extracted.empty()) {
        extract_obj(extracted.front());
        extracted.pop_front();
    }
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    ProfilerBlock profiler("update_handler", 100);
    static  int     pulse_area;           // 110 sec
    static  int     pulse_mobile;         // 4 sec
    static  int     pulse_violence;       // 3 sec
    static  int     pulse_point;          // 60 sec
    static  int     pulse_water_float;    // 4 sec
    static  int     pulse_raffect;        // 13 sec
    static  int     pulse_saving;         // 5 sec
    static  int     pulse_track;          // 7 sec

    bool heavy = false; // Did we have a time-consuming update this pulse?

    LastLogStream::send( ) <<  "Start handler"  << endl;

    LastLogStream::send( ) <<  "Erase dead NPC handler"  << endl;

    {
        Character *ch_next;

        for (Character* ch = char_list; ch != 0; ch = ch_next)
        {
            ch_next = ch->next;

            if( ch->isDead( ) )
                extract_char( ch );
        }
    }

    if ( --pulse_mobile   <= 0 )
    {
        pulse_mobile        = PULSE_MOBILE;

        LastLogStream::send( ) <<  "Player update"  << endl;
        player_update( );

        LastLogStream::send( ) <<  "Mobile update"  << endl;
        mobile_update( );

        LastLogStream::send( ) <<  "Diving update"  << endl;
        diving_update( );        

        heavy = true;
    }

    if ( --pulse_violence <= 0 )
    {
        pulse_violence        = dreamland->getPulseViolence( );

        LastLogStream::send( ) <<  "Violence update"  << endl;
        violence_update        ( );
    }

    if ( --pulse_water_float <= 0 )
    {
        pulse_water_float = PULSE_WATER_FLOAT;

        LastLogStream::send( ) <<  "Water float update"  << endl;
        water_float_update( );
    }

    if ( --pulse_raffect <= 0 )
    {
        pulse_raffect = PULSE_RAFFECT;

        LastLogStream::send( ) <<  "Room affects update"  << endl;
        room_affect_update( );
    }

    // Stagger tick updates across 4 pulses, just before the current tick ends.
    if (--pulse_point    <= 0)
    {
        LastLogStream::clear( );

        wiznet( WIZ_TICKS, 0, 0, "CHAR TICK!" );
        pulse_point     = PULSE_TICK;

        LastLogStream::send( ) <<  "Char update"  << endl;
        char_update        ( );
        char_update_prog();
        char_update_autosave();

        LastLogStream::send( ) <<  "Reboot update"  << endl;
        check_reboot        ( );

        /* room counting */
        {
            Character *ch;

            for (ch = char_list; ch != 0; ch = ch->next)
                if (!ch->is_npc() && ch->in_room != 0)
                    ch->in_room->areaIndex()->count =
                        min(ch->in_room->areaIndex()->count+1,5000000UL);
        }

        heavy = true;

    } else if (pulse_point == 1) {
        LastLogStream::send( ) <<  "Room update"  << endl;
        room_update        ( );
        heavy = true;

    } else if (pulse_point == 2) {
        LastLogStream::send( ) <<  "Objects update"  << endl;
        obj_update        ( );
        heavy = true;

    } else if (pulse_point == 3) {
        LastLogStream::send( ) <<  "Weather/Sunlight update"  << endl;
        weather_update( );
        sunlight_update( );
    }
    
    if (--pulse_track <= 0) 
    {
        pulse_track = PULSE_TRACK;

        LastLogStream::send( ) <<  "Track update"  << endl;
        track_update( );

        heavy = true;
    }
    
    LastLogStream::send( ) <<  "Aggressive update"  << endl;
    aggr_update( );

    if (--pulse_saving <= 0)
    {
        pulse_saving = PULSE_SAVING;

        LastLogStream::send( ) <<  "Room saving update"  << endl;
        room_saving();

        heavy = true;
    }

    LastLogStream::send( ) <<  "Auction update"  << endl;
    auction_update( );

    if (--pulse_area <= 0)
    {
        if (!heavy) {
            wiznet( WIZ_TICKS, 0, 0, "AREA & ROOM TICK!" );
            pulse_area        = PULSE_AREA;

            LastLogStream::send( ) <<  "Area update"  << endl;
            area_update        ( ); // Run onUpdate, mark areas for update

            heavy = true;
        }
    } 

    if (!heavy)
        area_update_next(); // Update one of the marked areas
}

/*
 * Aggression: bloodthirst, ambush, bonedagger, npc behavior
 */
void aggr_update( )
{
    ProfilerBlock profiler("aggr_update", 20);
    
    Character *ch, *ch_next;
    NPCharacter *mob;

    for (ch = char_list; ch; ch = ch_next) {
        ch_next = ch->next;

        if (ch->in_room == 0)
            continue;
        
        // Decrease wait and daze state for mobs and link-dead players.
        // For connected players these are decresed in IOManager::ioRead.
        if (ch->is_npc( ) || ch->desc == 0) {
            if (ch->wait > 0)
                ch->wait--;

            if (ch->daze > 0)
                ch->daze--;
        }

        check_bloodthirst( ch );

        if (ch->ambushing[0]) {
            gsn_ambush->getCommand( )->apply( ch );
            gsn_bonedagger->getCommand( )->apply( ch );
        }
        
        if (ch->in_room->area->empty)
            continue;

        if (!ch->is_npc( ))
            continue;
            
        mob = ch->getNPC( );
        
        if (mob->behavior)
            mob->behavior->aggress( );
    }
}

struct LightVampireDamage : public Damage {
    LightVampireDamage( Character *ch ) : Damage( ch, ch, DAM_NONE, 0 )
    {
        deathReason = "daylight";
    }
    
    virtual void calcDamage( ) {
        int n;

        switch (weather_info.sunlight) {
        case SUN_LIGHT: n = 15; break;
        case SUN_RISE:  n = 5;  break;
        case SUN_SET:   n = 3;  break;
        default:        n = 0;  break;
        }
        
        switch (weather_info.sky) {
        case SKY_CLOUDLESS: 
            n *= 2;
            break;
        case SKY_CLOUDY:
            n += n / 3;
            break;
        }
        
        dam = max( 1, (ch->max_hit * n) / 500 );

        if(ch->in_room->getSectorType() == SECT_FOREST) dam=dam/2;
        if(ch->in_room->getSectorType() == SECT_INSIDE) dam=dam/2;
        if(ch->in_room->getSectorType() == SECT_AIR) dam=dam+dam/2;

        protectRazer( );
    }
     
    virtual bool canDamage( ) {
        if (ch->is_immortal( ))
            return false;
        
        if (!ch->is_vampire( ))
            return false;
        
        if (!is_bright_for_vampire( ch ))
            return false;
            
        return Damage::canDamage( );
    }
        
    virtual void message( ) {
        DLString msg;
        
        if (weather_info.sunlight == SUN_LIGHT) 
            msg = "Солнечный свет тревожит тебя.";
        else if (weather_info.sunlight == SUN_RISE) 
            msg = "Лучи восходящего солнца тревожат тебя." ;
        else if (weather_info.sunlight == SUN_SET) 
            msg = "Закатные лучи тревожат тебя." ;
  
        oldact( msg.c_str( ), ch, 0, 0, TO_CHAR );

        msg.clear();

        if(ch->in_room->getSectorType() == SECT_FOREST) msg = "Листья деревьев частично защищают тебя от солнца.";
        if(ch->in_room->getSectorType() == SECT_INSIDE) msg = "Тени в помещении помогают выдержать солнечный свет.";
        if(ch->in_room->getSectorType() == SECT_AIR) msg = "В воздухе под открытым небом лучи солнца жалят тебя особенно сильно.";

        if(!msg.empty()) oldact( msg.c_str( ), ch, 0, 0, TO_CHAR );

        RussianString sunlight("солнечный свет", MultiGender::MASCULINE);
        if (dam == 0)
            msgRoom( "%2$^O1\6%3$C2", dam, sunlight, ch);
        else
            msgRoom( "%2$^O1\6%3$C4", dam, sunlight, ch);
            
        msgChar( "%2$^O1\6тебя", dam, sunlight );
    }
};

void light_update( PCharacter *ch )
{
    try {
        LightVampireDamage( ch ).hit( true );
    }
    catch (const VictimDeathException &) {
    }
}


void room_update( void )
{
    ProfilerBlock profiler("room_update", 50);
    RoomSet tempAffected;

    // Create temporary set to avoid removing set elements from inside the loop.
    tempAffected.insert(roomAffected.begin(), roomAffected.end());    

    for (auto &room: tempAffected) {
        AffectList affects = room->affected.clone();
        for (auto paf_iter = affects.cbegin( ); paf_iter != affects.cend( ); paf_iter++) {
            Affect *paf = *paf_iter;

            if ( paf->duration > 0 )
            {
                paf->duration--;
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if (!affects.hasNext(paf_iter))
                    if (paf->type->getAffect( ))
                        paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, room), paf );

                room->affectRemove( paf );
            }
        }
    }
}

void room_affect_update( )
{
    RoomSet tempAffected;

    // Create temporary set to avoid removing set elements from inside the loop.
    tempAffected.insert(roomAffected.begin(), roomAffected.end());    

    for (auto &room: tempAffected) {
        AffectList affects = room->affected.clone();
        for (auto paf_iter = affects.cbegin( ); paf_iter != affects.cend( ); paf_iter++) {
            Affect *paf = *paf_iter;
            
            if (paf->level == 1)
                paf->level = 2;

            if (!affects.hasNext(paf_iter) && paf->type->getAffect( )) 
                paf->type->getAffect()->onUpdate(SpellTarget::Pointer(NEW, room), paf );
        }
    }
}

void track_update( )
{
    Character *ch, *ch_next;
    NPCharacter *mob;

    for (ch = char_list; ch != 0; ch = ch_next)
    {
        ch_next = ch->next;

        if (!ch->is_npc( ))
            continue;

        mob = ch->getNPC( );

        if (mob->behavior)
            mob->behavior->track( );
    }
}


void check_reboot( void )
{
    Descriptor *d;
    DLString msg, msg2;

    switch(dreamland->getRebootCounter( ))
    {
    case -1:
        break;
    case 0:
        msg2 = "Мир Мечты перезапускается, надо немного подождать.";
        send_to_discord_stream(":red_circle: " + msg2);
        send_telegram(msg2);
        reboot_anatolia();
        return;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 10:
    case 15:
        msg2 = fmt( NULL, "Внимание! Через %1$d мину%1$Iту|ты|т будет перезагрузка Мира Мечты!",
                   dreamland->getRebootCounter( ) );
        if (dreamland->getRebootCounter( ) == 5) {
            send_to_discord_stream(":red_circle: " + msg2);
            send_telegram(msg2);
        }
        msg = fmt( NULL, "%1$^s громко кричит '{R%2$s{x'",
                   (chance( 50 ) ? "Хассан" : "Валькирия"), msg2.c_str() );
        for (d = descriptor_list; d != 0; d = d->next)
            if (d->connected == CON_PLAYING && d->character)
                d->character->pecho( msg );
    default:
        dreamland->setRebootCounter( dreamland->getRebootCounter( ) - 1 );
        break;
    }
}

void player_update( )
{
    PCharacter *ch;
    Descriptor *d;

    for( d = descriptor_list; d; d = d->next )
    {
        if( d->connected != CON_PLAYING || !d->character || !d->character->in_room)
            continue;

        ch = d->character->getPC( );

        if (HAS_SHADOW(ch)) 
            if (--ch->shadow < 0)
                oldact("Твоя вторая тень исчезает.",ch,0,0,TO_CHAR);
        
        if (ch->move <= 0 && ch->position != POS_SLEEPING)
            ch->pecho("Тебе нужно отдохнуть!");

        if ( IS_DEATH_TIME( ch ) ) {
            ch->last_death_time--;
            if (!IS_DEATH_TIME(ch)) {
                oldact("Ты полностью возвращаешься в мир живых.", ch, 0, 0, TO_CHAR);
                oldact("$c1 полностью возвращается в мир живых.", ch, 0, 0, TO_NOTVICT);
                UNSET_DEATH_TIME(ch);
            }
        }

        if( ch->PK_time_v > 0 )
            ch->PK_time_v--;
        else if( !ch->PK_time_v && IS_VIOLENT( ch ) )
        {
            oldact_p("Лихорадочный блеск смертоубийства в глазах $c2 пропадает.",
                   ch, 0, 0, TO_ROOM,POS_RESTING );
            oldact_p("Ты успокаиваешься после недавнего смертоубийства.",
                   ch, 0, 0, TO_CHAR,POS_RESTING );
            REMOVE_VIOLENT( ch );
        }

        if( ch->ghost_time > 0 )
            ch->ghost_time--;
        else if( !ch->ghost_time && IS_GHOST( ch ) )
        {
            oldact_p("В комнате начинает сгущаться божественная энергия и $c1 обретает плоть.\n\rНо похоже $c1 еще в мире мертвых.",
                   ch, 0, 0, TO_ROOM,POS_RESTING);
            oldact_p("Ты слышишь далекий колокольный звон.\n\rНа тебя накатывается волна ужасной боли...\n\rТы рождаешься заново, обретая плоть.\n\rНо ты пока еще между живыми и мертвыми.",
                   ch, 0, 0, TO_CHAR,POS_RESTING);
            REMOVE_GHOST( ch );
        }

        if( ch->PK_time_sk > 0 )
            ch->PK_time_sk--;
        else if( !ch->PK_time_sk )
        {
            if( IS_KILLER( ch ) )
            {
                oldact_p("Аура проклятия вокруг $c2 исчезает.",
                      ch, 0, 0, TO_ROOM,POS_RESTING);
                oldact_p("Боги забывают убийство, совершенное тобой.",
                      ch, 0, 0, TO_CHAR,POS_RESTING);
                REMOVE_KILLER( ch );
            }
            else if( IS_SLAIN( ch ) )
            {
                oldact_p("Все забывается... и даже записи жрецов Мира Мечты превращаются в прах.",
                      ch, 0, 0, TO_ROOM,POS_RESTING);
                oldact_p("Правда о твоем поражении забывается.",
                      ch, 0, 0, TO_CHAR,POS_RESTING);
                REMOVE_SLAIN( ch );
            }
        }

        if( ch->PK_time_t > 0 )
            ch->PK_time_t--;
        else if( !ch->PK_time_t )
        {
            if( IS_THIEF( ch ) )
            {
                oldact_p("Ты вздыхаешь с облегчением, ведь все забывают о твоей неспособности\n\rхоть что-то украсть.",
                      ch, 0, 0, TO_CHAR,POS_RESTING );
                REMOVE_THIEF( ch );
            }
        }

        light_update( ch );
    }
}

void lantern_update( Character *ch )
{
    Object *obj;

    if ( ( obj = get_eq_char( ch, wear_light ) ) != 0
            && obj->item_type == ITEM_LIGHT
            && obj->value2() > 0 )
    {
        obj->value2(obj->value2() - 1);
        if ( obj->value2() == 0 && ch->in_room != 0 )
        {
            ch->pecho("%1$^O1 мигну%1$Gло|л|ла|ли и поту%1$Gхло|х|хла|хли.", obj );
            ch->recho("%1$^O1 поту%1$Gхло|х|хла|хли.", obj );
            extract_obj( obj );
        }
        else if ( obj->value2() <= 5 && ch->in_room != 0)
            ch->pecho("%1$^O1 мига%1$nет|ют.", obj );
    }
}

void idle_update( PCharacter *ch )
{
    if (IS_VIOLENT( ch ))
    {
        oldact("Лихорадочный блеск смертоубийства в глазах $c2 пропадает.", ch, 0, 0, TO_ROOM );
        oldact("Ты успокаиваешься от недавнего смертоубийства.", ch, 0, 0, TO_CHAR );
        REMOVE_VIOLENT( ch );
        ch->PK_time_v = 0;
    }

    oldact("$c1 растворяется в воздухе.",ch, 0, 0, TO_ROOM );
    oldact("Ты растворяешься в воздухе.", ch, 0, 0, TO_CHAR );

    if (IS_SET(ch->config, CONFIG_AUTOAFK) && !IS_SET(ch->comm, COMM_AFK))
        interpret_raw( ch, "afk" );
}

void char_update_affects( Character *ch )
{
    AffectList affects = ch->affected.clone();
   
    try {    
        for (auto paf_iter = affects.cbegin( ); paf_iter != affects.cend( ); paf_iter++) {
            Affect *paf = *paf_iter;

            if ( paf->duration > 0 )
            {
                room_to_save( ch );
                paf->duration--;

                // spell strength fades with time
                if (number_range(0,4) == 0 && paf->level > 0)
                    paf->level--;
                
                if (!affects.hasNext(paf_iter) && paf->type->getAffect( )) 
                    paf->type->getAffect()->onUpdate(SpellTarget::Pointer(NEW, ch), paf );

            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                room_to_save( ch );

                if (!affects.hasNext(paf_iter))
                    if (paf->type->getAffect( )) 
                        paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, ch), paf );

                affect_remove( ch, paf );
            }
        }
    }
    catch (const VictimDeathException &) {
    }
}

/*
 * Check for weight of wielded weapon.
 */
static bool check_native_weapon( Character *ch, Object *obj )
{
    if (!ch->is_npc( )
        || ch->getNPC( )->pIndexData->area != obj->pIndexData->area)
    {
        return false;
    }

    for (auto &paf: ch->affected)
        if (paf->location == APPLY_STR && paf->modifier < 0)
            return false;
    
    return true;
}

void wield_update( Character *ch )
{
    Object *wield, *second;
    
    second = get_eq_char( ch, wear_second_wield );
    
    if (second 
            && wear_second_wield->canRemove( ch, second, 0 )
            && second->getWeight( ) > (get_str_app(ch).wield * 5)
            && !check_native_weapon( ch, second ))
    {
        oldact("Ты не в силах удержать $o4 в левой руке.", ch, second, 0, TO_CHAR);
        oldact("$c1 не в силах удержать $o4.", ch, second, 0, TO_ROOM);
        unequip_char( ch, second );
    }
    
    wield = get_eq_char( ch, wear_wield );
    
    if (wield 
            && wear_wield->canRemove( ch, wield, 0 )
            && wield->getWeight( ) > (get_str_app(ch).wield * 10)
            && !check_native_weapon( ch, wield ))
    {
        oldact("Ты не в силах удержать $o4 в правой руке.", ch, wield, 0, TO_CHAR);
        oldact("$c1 не в силах удержать $o4.", ch, wield, 0, TO_ROOM);
        unequip_char( ch, wield );
    }
    
    if (IS_AWAKE(ch) 
            && (second && second->wear_loc == wear_second_wield) 
            && (!wield || wield->wear_loc == wear_none)) 
    {
        oldact("Ты вооружаешься вторичным оружием, как основным!", ch, 0,0,TO_CHAR);
        oldact("$c1 вооружается вторичным оружием, как основным!", ch, 0,0,TO_ROOM);
        unequip_char( ch, second );
        equip_char( ch, second, wear_wield );
    }
}

