/* $Id$
 *
 * ruffina, 2004
 */
#include "room.h"
#include "affecthandler.h"
#include "affect.h"
#include "affectmanager.h"
#include "character.h"
#include "core/object.h"
#include "feniamanager.h"

#include "dreamland.h"
#include "roombehaviormanager.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

WEARLOC(none);

extra_exit_data * extra_exit_data::create()
{
    EXTRA_EXIT_DATA *peexit = new EXTRA_EXIT_DATA;

    peexit->description = str_dup(description);
    peexit->exit_info_default = peexit->exit_info = exit_info_default;
    peexit->key = key;
    peexit->u1.vnum = u1.vnum;
    peexit->level = level;

    peexit->keyword = str_dup(keyword);
    peexit->short_desc_from = str_dup(short_desc_from);
    peexit->short_desc_to = str_dup(short_desc_to);
    peexit->room_description = str_dup(room_description);
    peexit->max_size_pass = max_size_pass;

    peexit->moving_from = moving_from;
    peexit->moving_mode_from = moving_mode_from;
    peexit->moving_to = moving_to;
    peexit->moving_mode_to = moving_mode_to;

    return peexit;
}        

exit_data *exit_data::create()
{
    EXIT_DATA *pexit = (EXIT_DATA*)alloc_perm(sizeof(EXIT_DATA));

    pexit->keyword = str_dup(keyword);
    pexit->short_descr = str_dup(short_descr);
    pexit->description = str_dup(description);
    pexit->exit_info_default = pexit->exit_info = exit_info_default;
    pexit->key = key;
    pexit->u1.vnum = u1.vnum; 
    pexit->level = 0;

    return pexit;
}

Room * RoomIndexData::create()
{
    if (room) // FIXME allow multiple instances
        throw Exception("Attempt to create second instance of a room.");

    room = new Room;
    
    room->area = areaIndex->area; // FIXME point to relevant instance
    room->vnum = vnum;
    room->room_flags = room_flags;
    room->pIndexData = this;    
    room->setID( dreamland->genID( ) );    

    RoomBehaviorManager::assign(room);

    for (int i = 0; i < DIR_SOMEWHERE; i++)
        if (exit[i])
            room->exit[i] = exit[i]->create();

    for (auto &eexit: extra_exits)
        room->extra_exits.push_back(eexit->create());

    room->position = roomInstances.size();
    roomInstances.push_back(room);

    room->area->rooms[vnum] = room;

    if (FeniaManager::wrapperManager)
        FeniaManager::wrapperManager->linkWrapper( room );    

    return room;
}


static int zero;

static int & room_flag_by_table(Room *room, const FlagTable *table)
{
    if (table == &raffect_flags)
        return room->affected_by;
    else if (table == &room_flags)
        return room->room_flags;

    return zero;
}


/*
 * Apply or remove an affect to a room.
 */
void Room::affectModify( Affect *paf, bool fAdd )
{
    int mod;

    mod = paf->modifier;

    if (fAdd)
    {
        int &flag = room_flag_by_table(this, paf->bitvector.getTable());
        SET_BIT(flag, paf->bitvector.getValue());
    }
    else
    {
        int &flag = room_flag_by_table(this, paf->bitvector.getTable());
        REMOVE_BIT(flag, paf->bitvector.getValue());
        mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        bug( "Affect_modify_room: unknown location %d.", paf->location.getValue() );
        return;

    case APPLY_HEAL_GAIN:        mod_heal_rate += mod;                break;
    case APPLY_MANA_GAIN:        mod_mana_rate += mod;                break;
    }
}

/*
 * Give an affect to a room.
 */
void Room::affectTo( Affect *paf )
{
    if (affected.empty()) {
        roomAffected.insert(this);
    }

    affected.push_front(paf->clone());

    affectModify( paf, true );
}

void Room::affectCheck( const FlagTable *table, int vector )
{
    if (vector == 0)
        return;

    int &flag = room_flag_by_table(this, table);

    for (auto &paf: affected)
        if (paf->bitvector.getTable() == table && paf->bitvector.isSet(vector))
            SET_BIT(flag, vector);
}

/*
 * Remove an affect from a room.
 */
void Room::affectRemove( Affect *paf, bool verbose )
{
    if (paf->isExtracted())
        return;

    if (affected.empty())
    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }

    if (verbose) {
        if (paf->type->getAffect())
            paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, this), paf);
    }

    affectModify( paf, false );

    affected.remove(paf);

    affectCheck(paf->bitvector.getTable(), paf->bitvector.getValue());

    if (affected.empty())
        roomAffected.erase(this);

    AffectManager::getThis()->extract(paf);
}

/*
 * Strip all affects of a given sn.
 */
void Room::affectStrip( int sn, bool verbose )
{
    for (auto &paf: affected.findAll(sn))
        affectRemove( paf, verbose );
}


/*
 * Add or enhance an affect.
 */
void Room::affectJoin( Affect *paf )
{
    for (auto &paf_old: affected)
    {
        if ( paf_old->type == paf->type )
        {
            paf->level += paf_old->level;
            paf->level /= 2;
            paf->duration += paf_old->duration;
            paf->modifier += paf_old->modifier;
            affectRemove( paf_old );
            break;
        }
    }

    affectTo( paf );
}


/*
 * Return true if a room is affected by a spell.
 */
bool Room::isAffected( int sn ) const
{
    return affected.find(sn) != 0;
}

/** Whether an item can contribute to the light in the room. */
static bool is_lantern(Object *obj)
{
    if (obj->item_type != ITEM_LIGHT)
        return false;
        
    // Expired lanterns don't count.
    if (obj->value2() == 0)
        return false;

    // Worn item is ok.
    if (obj->carried_by)
        return obj->wear_loc != wear_none;

    // Item on the floor ok if it's a permanent fixture.
    if (obj->in_room)
        return !IS_SET(obj->wear_flags, ITEM_TAKE);

    return false;
}

/** Recalculate light in the room when an object is worn or destroyed. */
void Room::updateLight()
{
    light = 0;

    for (Character *rch = people; rch; rch = rch->next_in_room)
        for (Object *obj = rch->carrying; obj; obj = obj->next_content)
            if (is_lantern(obj))
                light++;

    for (Object *obj = contents; obj; obj = obj->next_content)
        if (is_lantern(obj))
            light++;
}
