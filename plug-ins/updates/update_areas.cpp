/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "profiler.h"
#include "update_areas.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "wearlocation.h"
#include "wearloc_codes.h"
#include "affect.h"

#include "dreamland.h"
#include "loadsave.h"
#include "wiznet.h"
#include "gsn_plugin.h"
#include "descriptor.h"
#include "occupations.h"
#include "save.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(none);

static void rprog_reset( Room *room )
{
    FENIA_VOID_CALL( room, "Reset", "" );
}

/*
 * Repopulate areas periodically.
 */
void area_update( )
{
//    ProfilerBlock be("area_update");

    for (auto &pArea: areaInstances)
    {
        if ( ++pArea->age < 3 )
            continue;

        /*
         * Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15).
         */
        if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
                || pArea->age >= 31)
        {
            reset_area( pArea );
            wiznet( WIZ_RESETS, 0, 0, "%s has just been reset.", pArea->pIndexData->name );

            pArea->age = number_range( 0, 3 );
            if (IS_SET(pArea->area_flag, AREA_POPULAR))
                pArea->age = 15 - 2;
            else if (pArea->nplayer == 0)
                pArea->empty = true;
        }
    }
}

static Object * get_obj_list_vnum( Object *list, int vnum )
{
    Object *obj;

    for (obj = list; obj; obj = obj->next_content) 
        if (obj->pIndexData->vnum == vnum)
            return obj;
    
    return NULL;
}

static Object * get_obj_here_vnum( Room *room, int vnum )
{
    Object *obj, *result;
    
    for (obj = room->contents; obj; obj = obj->next_content) {
        if (obj->pIndexData->vnum == vnum)
            return obj;
            
        if (( result = get_obj_list_vnum( obj->contains, vnum ) ))
            return result;
    }

    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        if (rch->is_npc() && (result = get_obj_list_vnum(rch->carrying, vnum )))
            return result;

    return NULL;
}

static RESET_DATA * find_mob_reset(RoomIndexData *pRoom, NPCharacter *mob)
{
    RESET_DATA *pReset;
    for (pReset = pRoom->reset_first; pReset != 0; pReset = pReset->next)
        if (pReset->command == 'M' && pReset->arg1 == mob->pIndexData->vnum)
            return pReset;

    return 0;
}

static int count_mob_room(Room *room, MOB_INDEX_DATA *pMob, int limit)
{
    int count = 0;

    for (Character *rch = room->people; rch; rch = rch->next_in_room ) {
        if (rch->is_npc() && rch->getNPC()->pIndexData == pMob) {
            count++;
            if (limit > 0 && count >= limit)
                return count;
        }
    }

    return count;
}


/** Equip an item if required by reset configuration. */
static void reset_obj_location(RESET_DATA *pReset, Object *obj, NPCharacter *mob, bool verbose)
{
    if (pReset->command == 'E' && obj->wear_loc == wear_none) {
        Wearlocation *wloc = wearlocationManager->find( pReset->arg3 );
        if (!wloc)
            return;

        // Equip an item back, removing an odd item that ended up in that slot, i.e. scavenged weapons.
        if (verbose && obj->level <= mob->getRealLevel()) {
            Object *worn = wloc->find(mob);
            if (!worn || worn->pIndexData != obj->pIndexData)
                wloc->wear(obj, F_WEAR_VERBOSE | F_WEAR_REPLACE);
        }
        else { // New mob and item.
            wloc->equip(obj);
        }
    }
}

/** Create item inside of a container (P), based on the args: arg2 max count, arg4 min count. */
static bool create_item_for_container(RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, Object *obj_to)
{
    int limit;

    if (!obj_to)
        return false;

    // TODO: review this nonsense.
    if ( pReset->arg2 > 50 )         /* old format */
        limit = 6;
    else if (pReset->arg2 == -1 || pReset->arg2 == 0)     /* no limit */
        limit = 999;
    else
        limit = pReset->arg2;

    int count = count_obj_list(pObjIndex, obj_to->contains);

    if (count > limit)
        return false;

    if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit)
        return false;
    
    while (count < pReset->arg4)
    {
        Object *obj = create_object( pObjIndex, 0 );
        obj->reset_obj = obj_to->getID();
        obj_to_obj( obj, obj_to );
        count++;

        if (pObjIndex->count >= limit)
            break;
    }

    /* fix object lock state! */
    obj_to->value1(obj_to->pIndexData->value[1]);
    return true;
}

/** Create an item for inventory or equipment for a given mob, based on reset data. */
static Object * create_item_for_mob(RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, NPCharacter *mob, bool verbose)
{
    Object *obj = NULL;

    // Obj limit reached, do nothing.
    if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit) 
        return obj;
    
    obj = create_object(pObjIndex, 0);
    obj->reset_mob = mob->getID();

    // Mark shop items with 'inventory' flag.
    if (mob->behavior && IS_SET(mob->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
    {
        if (pReset->command == 'G')
            SET_BIT( obj->extra_flags, ITEM_INVENTORY );
    }

    // Give and equip the item.
    obj_to_char( obj, mob );
        
    if (obj->pIndexData->limit != -1)
        if (verbose)
            mob->recho("Милость богов снисходит на %C2, принося с собой %O4.", mob, obj);

    reset_obj_location(pReset, obj, mob, verbose);

    return obj;
}

/** Recreate an item on a mob based on reset data, if it doesn't exist yet. */
static bool reset_mob_item(RESET_DATA *myReset, NPCharacter *mob)
{
    OBJ_INDEX_DATA *pObjIndex = get_obj_index(myReset->arg1);
    if (!pObjIndex)
        return false;

    Object *self = get_obj_list_vnum(mob->carrying, pObjIndex->vnum);
    if (self) {
        reset_obj_location(myReset, self, mob, true);
        return false;
    }

    Object *newItem = create_item_for_mob(myReset, pObjIndex, mob, true);
    if (newItem) {
        wiznet(WIZ_RESETS, 0, 0, "Created [%d] %s for mob %s in [%d].", 
               pObjIndex->vnum, newItem->getShortDescr('1').c_str(), 
               mob->getNameP('1').c_str(), mob->in_room->vnum);
        return true;
    }

    return false;
}

/** Recreate inventory and equipment for a mob, if an item has been
  * requested or stolen from it.
  */
static bool reset_one_mob(NPCharacter *mob)
{
    // Find room where this mob was created and corresponding reset for it.
    // Reset may not be exact, p.ex. when several guards are reset in the same room with different equipment.
    // Such situations should be avoided in general, by creating different vnums for guards.
    Room *home = get_room_instance(mob->reset_room);
    if (!home)
        return false;

    RESET_DATA *myReset = find_mob_reset(home->pIndexData, mob);
    if (!myReset)
        return false;
   
    // Update all inventory and equipment on the existing mob. 
    RESET_DATA *pReset;
    bool changed = false;
    for (pReset = myReset->next; pReset; pReset = pReset->next)
        switch (pReset->command) {
        case 'E':
        case 'G':
            if (reset_mob_item(pReset, mob)) 
                changed = true;
            break;

        default:
            return changed;
        }
        
        
    return changed; 
}

/** Recreate inventory and equipment for all NPC in the room. */
static bool reset_room_mobs(Room *pRoom)
{
    bool changed = false;

    for (Character *rch = pRoom->people; rch; rch = rch->next_in_room) {
        if (!rch->is_npc())
            continue;
        if (IS_CHARMED(rch))
            continue;

        NPCharacter *mob = rch->getNPC();
        if (mob->reset_room == 0)
            continue;
        // FIXME: zone should point to area instance.
        if (mob->zone != 0 && mob->in_room->areaIndex() != mob->zone)
            continue;

        if (reset_one_mob(mob))
            changed = true;
    }

    return changed;
}

void reset_room(Room *pRoom, int flags)
{
    RESET_DATA *pReset;
    NPCharacter *mob;
    bool last;
    int iExit;
    bool changedMob, changedObj;
    
    // Only create chests and their content if there's no one in the area or during 'redit reset'.
    bool loadFloorItems;
    if (IS_SET(flags, FRESET_ALWAYS))
        loadFloorItems = true;
    else if (pRoom->area->nplayer > 0 && !IS_SET(pRoom->area->area_flag, AREA_POPULAR))
        loadFloorItems = false;
    else
        loadFloorItems = true;

    if (weather_info.sky == SKY_RAINING 
        && !IS_SET(pRoom->room_flags, ROOM_INDOORS) ) 
    {
        pRoom->history.erase( );

        if (number_percent( ) < 50)
            pRoom->history.erase( );
    }

    for(iExit = 0; iExit < DIR_SOMEWHERE; iExit++) {
        EXIT_DATA *pExit = pRoom->exit[iExit];
        if (pExit)
            pExit->reset();
    }

    for(auto &eexit: pRoom->extra_exits)
        eexit->reset();

    mob     = 0;
    last    = true;
    changedMob = changedObj = false;

    dreamland->removeOption( DL_SAVE_OBJS );
    dreamland->removeOption( DL_SAVE_MOBS );

    // Update existing mobs before creating new ones.
    if (reset_room_mobs(pRoom))
        changedMob = true;

    for ( pReset = pRoom->pIndexData->reset_first; pReset != 0; pReset = pReset->next )
    {
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        EXIT_DATA *pexit;
        Object *obj = 0;
        Object *obj_to = 0;
        int roomCount, worldCount;
        
        switch ( pReset->command )
        {
        default:
            LogStream::sendError( ) << "Reset_area: bad command " << pReset->command << '.' << endl;
            break;

        case 'M':

            if ( ( pMobIndex = get_mob_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

            worldCount = pMobIndex->count; // FIXME: global count should consider room instances and charmed mobs
            if (pReset->arg2 != -1 && worldCount >= pReset->arg2) {
                last = false;
                break;
            }

            roomCount = count_mob_room(pRoom, pMobIndex, pReset->arg4);
            if (roomCount >= pReset->arg4) {
                last = false;
                break;
            }

            mob = create_mobile( pMobIndex );

            /* set area */
            mob->zone = pRoom->areaIndex();
            mob->reset_room = pRoom->vnum;

            char_to_room( mob, pRoom );
            changedMob = true;
            last  = true;
            break;

        case 'O':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if (!loadFloorItems) {
                last = false;
                break;
            }

            roomCount = count_obj_list(pObjIndex, pRoom->contents);
            if (roomCount > 0) {
                last = false;
                break;
            }

            if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit) {
                last = false;
                break;
            }

            obj = create_object( pObjIndex, 0 );
            obj->cost = 0;
            obj->reset_room = pRoom->vnum;
            obj_to_room( obj, pRoom );
            changedObj = true;
            last = true;
            break;

        case 'P':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == 0 )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            obj_to = get_obj_here_vnum(pRoom, pObjToIndex->vnum);

            if (!obj_to || (obj_to->in_room && !loadFloorItems)) {
                last = false;
                break;
            }

            if (create_item_for_container(pReset, pObjIndex, obj_to)) {
                changedObj = true;
                last = true;
            } else {
                last = false;
            }

            break;

        case 'G':
        case 'E':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if (!last)
                break;

            if ( mob == 0 )
            {
                bug( "Reset_area: 'E' or 'G': null mob for vnum %d.",    pReset->arg1 );
                last = false;
                break;
            }

            if (!create_item_for_mob(pReset, pObjIndex, mob, false))
                break;

            changedMob = true;
            last = true;
            break;

        case 'D':
            break;

        case 'R':
            {
                int d0, d1;
                int min, max;

                min = pReset->arg3;
                max = pReset->arg2 - 1;
            
                for (d0 = min; d0 < max; d0++)
                {
                    d1 = number_range( d0, max );
                    pexit = pRoom->exit[d0];
                    pRoom->exit[d0] = pRoom->exit[d1];
                    pRoom->exit[d1] = pexit;
                }
            }
            break;
        }
    }
    
    dreamland->resetOption( DL_SAVE_OBJS );
    dreamland->resetOption( DL_SAVE_MOBS );

    if (changedMob)
        save_mobs( pRoom );

    if (changedObj)
        save_items( pRoom );

    rprog_reset(pRoom);
}

/*
 * Reset one area.
 */
void reset_area( Area *pArea, int flags )
{
    const char *resetmsg;
    static const char *default_resetmsg = "Ты слышишь мелодичный перезвон колокольчиков.";        

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++)
        reset_room( i->second, flags );
    
    if (pArea->pIndexData->behavior) 
        pArea->pIndexData->behavior->update( );

    if (pArea->pIndexData->resetmsg)
        resetmsg = pArea->pIndexData->resetmsg;
    else
        resetmsg = default_resetmsg;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
        Character *ch;
        
        if (d->connected == CON_PLAYING
                && ( ch = d->character )
                && IS_AWAKE(ch)
                && ch->in_room 
                && !IS_SET(ch->in_room->room_flags, ROOM_NOWHERE)
                && ch->in_room->area == pArea) 
        {
            if (weather_info.sky == SKY_RAINING 
                && !IS_SET(ch->in_room->room_flags, ROOM_INDOORS)
                && gsn_track->getEffective( ch ) > 50)
            {
                ch->println( "Внезапно налетевший дождь смывает все следы." );
            }

            ch->println( resetmsg );
        }
    }
}


