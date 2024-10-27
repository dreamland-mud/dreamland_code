/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
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
#include "weapontier.h"
#include "../loadsave/behavior_utils.h"

#include "descriptor.h"
#include "occupations.h"
#include "itemevents.h"
#include "save.h"
#include "merc.h"

#include "def.h"

WEARLOC(none);
WEARLOC(wield);
GSN(track);

static void rprog_reset( Room *room )
{
    if (behavior_trigger(room, "Reset", "R", room))
        return;
        
    FENIA_VOID_CALL( room, "Reset", "" );
}

// FIXME Area instance needs to be passed as a parameter, but 
// we don't have area wrappers yet and AreaWrapper name is taken.
static void aprog_update( Area *area )
{
    FENIA_NDX_VOID_CALL( area, "Update", "" );
}

// A queue of areas that are waiting to be reset at the next opportunity.
struct AreaResetEvent {
    AreaResetEvent(Area *pArea, int flags) {
        this->pArea = pArea;
        this->flags = flags;
    }

    Area *pArea;
    int flags;
};

std::list<AreaResetEvent> areasToReset;

/*
 * Repopulate areas periodically. Decides whether to reset given area based on passed flags,
 * area age and popularity. If reset is required, area (alongside the flags requested) is placed
 * at the top of the waiting queue. The queue is processed at the rate of one area per pulse, to
 * prevent multiple area resets per pulse, taking more than the pulse itself.
 */
void area_update( int flags )
{
    for (auto &pArea: areaInstances)
    {
        aprog_update(pArea);

        if (!IS_SET(flags, FRESET_ALWAYS) && ++pArea->age < 3)
            continue;

        /*
         * If a reset is forced (e.g. after world startup): mark for reset immediately.
         * Popular areas (newbie ones): reset every 3 minutes.
         * Areas with some recent activity (empty == false): reset every 15 minutes or once all players are out.
         * All other areas: reset every 30 minutes.
         */
        if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
                || pArea->age >= 31
                || IS_SET(flags, FRESET_ALWAYS))
        {            
            // Re-put the area into the waiting queue, to be reset in the next pulse.
            areasToReset.remove_if([&pArea] (auto &event) {
                return event.pArea == pArea;
            });

            areasToReset.push_front(AreaResetEvent(pArea, flags));

            wiznet( WIZ_RESETS, 0, 0, "%s has been market for reset.", pArea->pIndexData->getName().c_str() );
        }
    }
}

/** Resets top area from the waiting queue. */
void area_update_next()
{
    ProfilerBlock profiler("area_update_next", 100);

    if (areasToReset.empty())
        return;

    // Take first area out of the queue and reset it. 
    AreaResetEvent event = areasToReset.front();
    areasToReset.pop_front();

    reset_area(event.pArea, event.flags);

    wiznet( WIZ_RESETS, 0, 0, "%s has just been reset.", event.pArea->pIndexData->getName().c_str() );

    // Update area age according to its popularity.
    event.pArea->age = number_range( 0, 3 );

    if (IS_SET(event.pArea->area_flag, AREA_POPULAR))
        event.pArea->age = 15 - 2;
    else if (event.pArea->nplayer == 0)
        event.pArea->empty = true;
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

static int find_mob_reset(RoomIndexData *pRoom, NPCharacter *mob)
{
    for (unsigned int i = 0; i < pRoom->resets.size(); i++) {
        RESET_DATA *pReset = pRoom->resets[i];
        if (pReset->command == 'M' && pReset->arg1 == mob->pIndexData->vnum)
            return i;
    }

    return -1;
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

// Calculate reset probability for an item inside container. 
static int item_reset_chance(RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, Object *container, bool isForced)
{
    // Limited items never reset if count is exceeded.
    if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit)
        return 0;

    // 'redit reset' called explicitly always resets everything, for testing.
    if (isForced)
        return 100;

    // Limited items are not reset in populated areas; have a chance to be reset in an empty area.
    if (pObjIndex->limit != -1) {
        if (container->getRoom()->area->nplayer > 0)
            return 0;

        return 10;
    }

    // 'maxCount' specifies maximum reset at the given location;     
    int maxCount = pReset->arg2;
    if (maxCount <= 0)     /* no limit */
        maxCount = 999;

    // If there are too many in the world already, decrease reset chances.
    if (pObjIndex->count >= maxCount)
        return 25;
        
    return 100;
}

// Calculate reset probability for a carried item. 
static int item_reset_chance(RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, NPCharacter *mob)
{
    // Obj limit reached, do nothing.
    if (pObjIndex->limit != -1 && pObjIndex->count >= pObjIndex->limit) 
        return 0;

    // Limited items are not reset in populated areas; have a chance to be reset in an empty area.
    if (pObjIndex->limit != -1) {
        if (mob->in_room->area->nplayer > 0)
            return 0;

        return 10;
    }

    // Shop items and non-random stuff is reset immediately,
    // random weapons never appear in populated areas and are otherwise delayed.
    if (pReset->rand == RAND_NONE && !item_is_random(pObjIndex))
        return 100;

    if (IS_SET(mob->in_room->area->area_flag, AREA_DUNGEON))
        return 100;

    if (mob_has_occupation(mob, OCC_SHOPPER))
        return 100;

    if (mob->in_room->area->nplayer > 0)
        return 0;

    return 10;
}


/** Equip an item if required by reset configuration. */
static void reset_obj_location(RESET_DATA *pReset, Object *obj, NPCharacter *mob, bool verbose)
{
    if (pReset->command != 'E')
        return;

    Wearlocation *wloc = wearlocationManager->find( pReset->arg3 );
    if (!wloc)
        return;
    
    if (obj->wear_loc == wloc->getIndex())
        return;
        
    // Equip an item back, removing an odd item that ended up in that slot, i.e. scavenged weapons.
    if (verbose && obj->level <= mob->getRealLevel()) {
        Object *worn = wloc->find(mob);
        if (!worn || worn->pIndexData != obj->pIndexData)
            wloc->wear(obj, F_WEAR_VERBOSE | F_WEAR_REPLACE);
    }
    else { 
        Object *worn = wloc->find(mob);
        if (worn)
            wloc->unequip(worn);
        wloc->equip(obj);
    }
}

static int count_vnums_in_list(vector<int> &vnums, Object *list)
{
    int count  = 0;
    set<int> vnumSet;
    for (auto &vnum: vnums)
        vnumSet.insert(vnum);

    for (Object *obj = list; obj; obj = obj->next_content)
        if (vnumSet.count(obj->pIndexData->vnum) > 0)
            count++;

    return count;
}

/** Create item inside of a container (P), based on the args: arg2 max count, arg4 min count. */
static bool create_item_for_container(RESET_DATA *pReset, Object *obj_to, bool isForced)
{
    if (!obj_to)
        return false;

    vector<int> vnums;
    vnums.insert(vnums.end(), pReset->vnums.begin(), pReset->vnums.end());
    int count = count_vnums_in_list(vnums, obj_to->contains);
    int minCount = pReset->arg4;    

    if (vnums.empty()) {
        bug("Reset_area: 'P' empty list of vnums for %d", obj_to->pIndexData->vnum);
        return false;
    }
    
    // Reset either one item or a random item from the 'vnum' list, until desired
    // count is reached or max limit exceeded on item prototypes.
    while (count < minCount) {
        int random_index = number_range(0, vnums.size() - 1);
        int random_vnum = vnums.at(random_index);
        OBJ_INDEX_DATA *pObjIndex = get_obj_index(random_vnum);

        if (!pObjIndex) {
            bug("Reset_area: 'P' bad vnum %d.", random_vnum);
            break;
        }
        
        if (!chance(item_reset_chance(pReset, pObjIndex, obj_to, isForced)))
            break;

        Object *obj = create_object(pObjIndex, 0);
        obj->reset_obj = obj_to->getID();
        obj_to_obj( obj, obj_to );
        eventBus->publish(ItemResetEvent(obj, pReset));

        count++;

        // If all but one vnums had a chance to reset but minCount still not reached,
        // remaining spots will be filled by the last remaining vnum.
        if (vnums.size() > 1)
            vnums.erase(vnums.begin() + random_index);
    }
    
    return true;
}

/** Create an item for inventory or equipment for a given mob, based on reset data. */
static Object * create_item_for_mob(RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, NPCharacter *mob, bool verbose)
{
    Object *obj = NULL;

    if (!pObjIndex) {
        bug("Reset_area: bad vnum %d for mob %d.", pReset->arg1, mob->pIndexData->vnum);
        return obj;
    }

    // Not all items are reset immediately.
    if (!chance(item_reset_chance(pReset, pObjIndex, mob)))
        return obj;
    
    obj = create_object(pObjIndex, 0);
    obj->reset_mob = mob->getID();

    // Give and equip the item.
    obj_to_char( obj, mob );

    // Mark shop items with 'inventory' flag.
    if (mob_has_occupation(mob, OCC_SHOPPER)) {
        if (pReset->command == 'G')
            SET_BIT( obj->extra_flags, ITEM_INVENTORY );
    }

    if (obj->pIndexData->limit != -1)
        if (verbose)
            mob->recho("Милость богов снисходит на %C2, принося с собой %O4.", mob, obj);

    reset_obj_location(pReset, obj, mob, verbose);
    eventBus->publish(ItemResetEvent(obj, pReset));

    return obj;
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

    int myReset = find_mob_reset(home->pIndexData, mob);
    if (myReset < 0)
        return false;
   
    // Restore mob to its starting position, in case it's different from the default one.
    if (mob->in_room == home 
        && mob->position == mob->pIndexData->default_pos
        && mob->position != mob->pIndexData->start_pos)
    {
        mob->position = mob->pIndexData->start_pos;
        switch (mob->position) {
        case POS_RESTING: mob->recho("%^C1 садится отдыхать.", mob); break;
        case POS_SITTING: mob->recho("%^C1 садится.", mob); break;
        case POS_SLEEPING: mob->recho("%^C1 засыпает.", mob); break;
        case POS_STANDING: mob->recho("%^C1 встает.", mob); break;
        }
    }

    // Collect all resets for this mob, inventory and equipment.
    list<RESET_DATA *> inventoryResets;
    map<int, RESET_DATA *> equipResets;

    for (unsigned int i = myReset + 1; i < home->pIndexData->resets.size(); i++) {
        RESET_DATA *pReset = home->pIndexData->resets[i];
        bool stop = false;
        switch (pReset->command) {
        case 'E':
            equipResets[pReset->arg3] = pReset;
            break;

        case 'G':
            inventoryResets.push_back(pReset);
            break;

        case 'P':
            break;

        default:
            stop = true;
            break;
        }

        if (stop)
            break;        
    }

    bool changed = false;

    // Restore missing pieces of inventory or equip.
    for (auto &r: equipResets) {
        RESET_DATA *myReset = r.second;

        // Ignore items already in their correct place.
        Wearlocation *wloc = wearlocationManager->find(myReset->arg3);
        Object *worn = wloc->find(mob);
        if (worn && worn->pIndexData->vnum == myReset->arg1) {
            eventBus->publish(ItemResetEvent(worn, myReset));
            continue;
        }        
        
        // Item can be in inventory (after disarm) or wielded (after being removed from a sheath).
        Object *self = 0;
        for (Object *obj = mob->carrying; obj; obj = obj->next_content)
            if (obj->pIndexData->vnum == myReset->arg1
                && (obj->wear_loc == wear_none || obj->wear_loc == wear_wield)) 
            {
                self = obj;
                break;
            }

        if (self) {
            reset_obj_location(myReset, self, mob, true);
            eventBus->publish(ItemResetEvent(self, myReset));        
        } else {
            OBJ_INDEX_DATA *pObjIndex = get_obj_index(myReset->arg1);
            self = create_item_for_mob(myReset, pObjIndex, mob, true);
        }

        if (self) {
            changed = true;
        }
    }

    for (auto &r: inventoryResets) {
        RESET_DATA *myReset = r;

        // Ignore items already in inventory.
        Object *self = 0;
        for (Object *obj = mob->carrying; obj; obj = obj->next_content)
            if (obj->pIndexData->vnum == myReset->arg1 && obj->wear_loc == wear_none) {
                self = obj;
                break;
            }

        if (self) {
            eventBus->publish(ItemResetEvent(self, myReset));
            continue;
        }

        // Item could be worn, just remove it.
        self = get_obj_list_vnum(mob->carrying, myReset->arg1);
        if (self) {
            self->wear_loc->unequip(self);
            eventBus->publish(ItemResetEvent(self, myReset));
        } else {
            OBJ_INDEX_DATA *pObjIndex = get_obj_index(myReset->arg1);
            self = create_item_for_mob(myReset, pObjIndex, mob, true);
        }

        if (self) {
            changed = true;
        }
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

struct ResetRules {
    ResetRules(Room *pRoom, RESET_DATA *pReset, int flags) {
        isForced = IS_SET(flags, FRESET_ALWAYS);
        resetMobs = true;
        resetFloorItems = true;
        resetChestLocks = true;

        // FRESET_ALWAYS overrides all other conditions and resets everything.
        if (isForced)
            return;   

        if (IS_SET(pReset->flags, RESET_NEVER)) {
            resetMobs = false;
            resetFloorItems = false;
            resetChestLocks = false;
            return;   
        }

        // Reset everything in dungeons unless marked with RESET_NEVER.
        if (IS_SET(pRoom->area->area_flag, AREA_DUNGEON))
            return;

        // Only create chests and their content if there's no one in the area or during 'redit reset'.
        if (pRoom->area->nplayer > 0 && !IS_SET(pRoom->area->area_flag, AREA_POPULAR)) {
            resetFloorItems = false;
            resetChestLocks = false;
        }
    }

    bool resetMobs;
    bool resetFloorItems;
    bool resetChestLocks;
    bool isForced;
};

void reset_room(Room *pRoom, int flags)
{
    NPCharacter *mob;
    bool last;
    int iExit;
    bool changedMob, changedObj;
    
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

    for(auto &pReset: pRoom->pIndexData->resets)
    {
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        EXIT_DATA *pexit;
        Object *obj = 0;
        Object *obj_to = 0;
        int roomCount, worldCount;
        ResetRules rules(pRoom, pReset, flags);
        
        switch ( pReset->command )
        {
        default:
            LogStream::sendError( ) << "Reset_area: bad command " << pReset->command << '.' << endl;
            break;

        case 'M':

            if (!rules.resetMobs) {
                last = false;
                break;
            }

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
            if (!rules.resetFloorItems) {
                last = false;
                break;
            }

            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
                continue;
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
            eventBus->publish(ItemResetEvent(obj, pReset));
            changedObj = true;
            last = true;
            break;

        case 'P':
            if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == 0 )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            obj_to = get_obj_here_vnum(pRoom, pObjToIndex->vnum);

            if (!obj_to) {
                last = false;
                break;
            }
                
            if (obj_to->in_room && !rules.resetFloorItems) {
                last = false;
                break;
            }

            if (create_item_for_container(pReset, obj_to, rules.isForced)) {
                changedObj = true;
                last = true;
            } else {
                last = false;
            }

            if (rules.resetChestLocks) {
                /* fix object lock state! */
                obj_to->value1(obj_to->pIndexData->value[1]);
                changedObj = true;
            }

            break;

        case 'G':
        case 'E':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == 0 )
            {
                bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if (!last) {
                break;
            }

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
                ch->pecho( "Внезапно налетевший дождь смывает все следы." );
            }

            ch->pecho( resetmsg );
        }
    }
}


