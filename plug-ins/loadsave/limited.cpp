/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "logstream.h"

#include "pcharacter.h"
#include "object.h"
#include "room.h"

#include "fread_utils.h"
#include "dreamland.h"
#include "act.h"
#include "save.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

// Decide whether to update item count for this item prototype.
void limit_count_on_boot( OBJ_INDEX_DATA *pObjIndex, time_t ts, const DLString &playerName )
{
    int vnum = pObjIndex->vnum;

    // Sanity check.
    if (pObjIndex->limit < 0)
        return;

    // Limited item but not marked - show warning.
    if (ts <= 0) {
        LogStream::sendWarning( ) << "No timestamp on limited item " << vnum << " for player profile " << playerName << endl;
        pObjIndex->count++;
        return;
    }

    // Still has time to live.
    if (ts >= dreamland->getBootTime( )) {
        LogStream::sendNotice( ) << "Limited item " << vnum 
            << " still has " << (ts - dreamland->getBootTime( )) / Date::SECOND_IN_DAY 
            << " days in player profile " << playerName << endl;
        pObjIndex->count++;
        return;
    }

    // Item expired: do not increase obj count, new instances will be created.
    LogStream::sendNotice( ) << "Limited item " << vnum << " expired in player profile " << playerName << endl;
    return;
}

// Destroy expired limited objects once they're loaded from drops or player profile.
bool limit_check_on_load( Object *obj )
{
    if (obj->pIndexData->limit < 0)
        return false;

    if (obj->timestamp <= 0) 
        return false;
    
    if (obj->timestamp > dreamland->getCurrentTime( ))
        return false;

    bool fCount = true;

    if (obj->carried_by) {
        fCount = obj->timestamp > dreamland->getBootTime( );
        LogStream::sendNotice( ) << "Limited item " << obj->pIndexData->vnum 
            << " (" << obj->getID( ) << ") extracted for " << obj->carried_by->getName( )
            << ", " << (fCount ? "count":"nocount") << endl;
    } else {
        if (obj->getRoom( ))
            obj->getRoom( )->echo( POS_RESTING, "%1$^O1 рассыпается трухой!", obj );
        LogStream::sendNotice( ) << "Limited item " << obj->pIndexData->vnum 
            << " (" << obj->getID( ) << ") extracted in " << (obj->getRoom( ) ? obj->getRoom( )->vnum : -1)
            << ", " << (fCount ? "count":"nocount") << endl;
    }

    extract_obj_1( obj, fCount );
    return true;
}

// Return true if item is in its original reset place. Old items without reset_room etc fields
// are going to be destroyed, but only once.
static bool item_is_home(Object *obj)
{
    // Don't touch items no one can touch, e.g. roaming portal.
    if (!IS_SET(obj->wear_flags, ITEM_TAKE))
        return true;

    // Don't decay auction items.
    if (obj == auction->item)
	return true;

    if (obj->in_room) {
        if (obj->reset_room == obj->in_room->vnum)
            return true;

        return false;
    }

    if (obj->in_obj) {
        if (obj->reset_obj == obj->in_obj->getID())
            return true;

        // Deal with the timer once the corpse is decayed or item removed.
        if (obj->in_obj->item_type == ITEM_CORPSE_PC 
                || obj->in_obj->item_type == ITEM_CORPSE_NPC)
            return true;

        return false;
    }

    if (obj->carried_by && obj->carried_by->is_npc())
        return obj->reset_mob == obj->carried_by->getID();


    return false;
}

// Set up timestamp when a limited items gets into PC hands for the first time.
void limit_timestamp( Object *obj, Character *ch )
{
    if (obj->pIndexData->limit < 0)
        return;

    if (obj->timestamp > 0) 
        return;
    
    if (ch && ch->is_npc() && !IS_CHARMED(ch))
        return;

    // Two weeks from now.
    obj->timestamp = dreamland->getCurrentTime( ) + 2 * Date::SECOND_IN_WEEK; 
    LogStream::sendNotice( ) << "Limited item " << obj->pIndexData->vnum << " (" << obj->getID( ) << ") "
                             << "timestamped " << obj->timestamp << endl;
}

// Speed up decay unless carried by PC. Every minute spent on the ground counts as minus 1 day.
void limit_ground_decay(Object *obj)
{
    if (obj->pIndexData->limit < 0)
        return;

    // Don't touch items in their reset places or inside corpses.
    if (item_is_home(obj))
        return;

    // Check limited items  without a timer.
    // Example: spec_fido mob destroys a coprse, limited item falls on the ground.
    // Example: object becomes a limit after OLC changes.
    if (obj->timestamp <= 0) {
        limit_timestamp(obj, 0);
        save_items_at_holder(obj);
        return;
    }

    if (obj->carried_by && !obj->carried_by->is_npc())
        return;
    
    obj->timestamp -= 60 * 60 * 24;

    if (obj->in_room)
        obj->in_room->echo(POS_RESTING, "%1$^O1 неумолимо истонча%1$nется|ются.", obj);

    save_items_at_holder(obj);
}

// Destroy expired limited objects when room or player is saved.
bool limit_check_on_save( Object *obj )
{
    // Not a limited item.
    if (obj->pIndexData->limit < 0)
        return false;

    DLString where;
    if (obj->carried_by)
        where = obj->carried_by->getName( );
    else if (obj->in_room)
        where =  "room #" + DLString(obj->in_room->vnum);
    else
        where = "none";

    // Limited item but not marked (yet).
    if (obj->timestamp <= 0) 
        return false;

    // Still has time to live.
    if (obj->timestamp > dreamland->getCurrentTime( ))
        return false;

    // Don't destory item while on auction.	
    if (obj == auction->item)
        return false;

    if (obj->getRoom( ))
        obj->getRoom( )->echo( POS_RESTING, "%1$^O1 рассыпается трухой!", obj );

    // If item was already expired during boot time,
    // its counter hasn't been increased, thus we shouldn't decrease.
    bool fCount = obj->timestamp > dreamland->getBootTime( );
    LogStream::sendNotice( ) << "Limited item " << obj->pIndexData->vnum 
        << " (" << obj->getID( ) << ") extracted for " << where 
        << ", " << (fCount ? "count":"nocount") << endl;
    extract_obj_1( obj, fCount );
    return true;
}


void limit_purge( )
{
    Object *obj, *obj_next;

    for (obj = object_list; obj != 0; obj = obj_next) {
        obj_next = obj->next;
        limit_ground_decay(obj);
        limit_check_on_save(obj);
    }
}

