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

#include <list>
using namespace std;

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "mallocexception.h"
#include "fileformatexception.h"
#include "logstream.h"
#include "class.h"
#include "grammar_entities_impl.h"
#include "json_utils.h"
#include "dlscheduler.h"

#include "skillreference.h"
#include "skillgroup.h"
#include "race.h"
#include "mobilebehavior.h"
#include "areabehavior.h"

#include "fenia/register-impl.h"
#include "feniamanager.h"
#include "wrapperbase.h"

#include "affect.h"
#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "behavior.h"

#include "dreamland.h"
#include "merc.h"
#include "dl_strings.h"
#include "def.h"
#include <string.h>

using namespace std;

/*
 * Globals.
 */
Character *                char_list;
Character * newbie_list = 0;
Object *                object_list;
TIME_INFO_DATA                time_info;
WEATHER_DATA                weather_info;

AUCTION_DATA        *        auction = new auction_data( );

RoomSet roomAffected;

RoomVector roomInstances;

AreaVector areaInstances;

AreaIndexVector areaIndexes;

MOB_INDEX_DATA *        mob_index_hash                [MAX_KEY_HASH];
OBJ_INDEX_DATA *        obj_index_hash                [MAX_KEY_HASH];
RoomIndexMap roomIndexMap;

char                        str_empty        [1];

int                        top_area;


struct area_file * area_file_list;

struct area_file *
new_area_file(const char *name)
{
    struct area_file *rc = new area_file;

    rc->file_name = name;
        
    rc->next = area_file_list;
    area_file_list = rc;
    return rc;
}

AreaIndexData::AreaIndexData()
    : 
      low_range(0), high_range(0),
      min_vnum(0), max_vnum(0),
      count(0),
      area_flag(0),
      behavior(AreaBehavior::NODE_NAME),
      security(9), vnum(0), changed(false),
      wrapper(0),
      area(0)
{
}

Area * AreaIndexData::create()
{
    if (area) // FIXME allow multiple instances
        throw Exception("Attempt to create second instance of an area.");

    area = new Area;
    area->pIndexData = this;
    area->area_flag = area_flag;

    areaInstances.push_back(area);
    
    return area;
}

DLString AreaIndexData::getName(char gcase) const
{
    return name.get(LANG_DEFAULT).ruscase(gcase);
}

Area::Area()
    : empty(true), age(15), nplayer(0),
      area_flag(0), pIndexData(0)
{
}

obj_index_data::obj_index_data()
                : behaviors(behaviorManager)
{
    next = NULL;
    area = NULL;
    vnum = 0;
    reset_num = 0;
    material = "none";
    item_type = ITEM_TRASH;
    extra_flags = 0;
    wear_flags = 0;
    level = 0;
    condition = 100;
    count = 0;
    weight = 0;
    cost = 0;

    for (int i = 0; i < 5; i++)
        value[i] = 0;
        
    limit = -1;
    wrapper = 0;
    area = 0;
}

obj_index_data::~obj_index_data()
{
    
}

DLString obj_index_data::getProperty(const DLString &key) const
{
    // Look in props on index data: props[key] or props["blablah"][key]
    DLString jsonValue;
    JsonUtils::findValue(props, key, jsonValue);
    return jsonValue;
}

const char * obj_index_data::getDescription( lang_t lang ) const
{
    return description.get(lang).c_str();
}

const char * obj_index_data::getShortDescr( lang_t lang ) const
{
    return short_descr.get(lang).c_str();
}


auction_data::auction_data( )
                     : item( NULL ), seller( NULL ), buyer( NULL )
{
}

void exit_data::resolve()
{
    u1.to_room = get_room_instance( u1.vnum );
}

void exit_data::reset()
{
    exit_info = exit_info_default;
}

extra_exit_data::extra_exit_data()
    : exit_info(0), exit_info_default(0),
      key(0),
      max_size_pass(0), level(0)
{
    u1.to_room = 0;
}

extra_exit_data::~extra_exit_data()
{
}

void extra_exit_data::resolve()
{
    u1.to_room = get_room_instance( u1.vnum );
}

void extra_exit_data::reset()
{
    exit_info = exit_info_default;
}

reset_data::reset_data()
    : command('X'), 
      arg1(0), arg2(0), arg3(0), arg4(0),
      flags(0, &reset_flags), 
      rand(RAND_NONE, &rand_table),
      bestTier(0)
{

}      

ExtraDescription * ExtraDescrList::find(const DLString &kw) const
{
    for (const auto &ed: *this) {
        if (kw == ed->keyword)
            return ed;
    }

    return 0;
}

ExtraDescription * ExtraDescrList::findUnstrict(const DLString &kw) const
{
    for (const auto &ed: *this) {
        if (is_name(kw.c_str(), ed->keyword.c_str()))
            return ed;
    }

    return 0;
}


bool ExtraDescrList::findAndDestroy(const DLString &kw)
{
    ExtraDescription *ed = find(kw);
    if (!ed)
        return false;

    remove(ed);
    delete ed;
    return true;
}

void ExtraDescrList::deallocate()
{
    for(iterator pos = begin(); pos != end(); ++pos)
        delete( *pos );

    clear();
}





/*
 * Translates room virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
          pObjIndex != 0;
          pObjIndex  = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }
#if 0
    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
        throw FileFormatException( "get_obj_index: vnum %d not found on world startup", vnum );
#endif
    return 0;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
RoomIndexData *get_room_index( int vnum )
{
    auto r = roomIndexMap.find(vnum);
    if (r != roomIndexMap.end())
        return r->second;
#if 0
    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
        throw FileFormatException( "get_room_index: vnum %d not found on world startup", vnum );
#endif
    return 0;
}

// FIXME: should look up by vnum and instance.
Room *get_room_instance(int vnum)
{
    RoomIndexData *pRoomIndex = get_room_index(vnum);
    if (pRoomIndex)
        return pRoomIndex->room;
    return 0;
}

AreaIndexData * get_area_index(const DLString &filename)
{
    for(auto &area: areaIndexes)
        if (filename == area->area_file->file_name)
            return area;
            
    return 0;
}

