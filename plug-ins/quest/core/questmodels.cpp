/* $Id$
 *
 * ruffina, 2004
 */
#include "questmodels.h"
#include "questmodels-impl.h"
#include "questscenario.h"

#include "skillreference.h"
#include "skill.h"
#include "mobilebehaviormanager.h"
#include "occupations.h"
#include "selfrate.h"
#include "pcharacter.h"
#include "save.h"
#include "merc.h"
#include "def.h"
#include "roomtraverse.h"
#include "directions.h"

GSN(detect_hide);
GSN(detect_invis);
GSN(truesight);
GSN(acute_vision);
GSN(improved_detect);

// Check if room has at least one normal exit leading somewhere.
static bool room_has_exits(Room *room)
{
    for (int door = 0; door < DIR_SOMEWHERE; door++)
        if (direction_target(room, door) != NULL)
            return true;

    return false;
}

/*--------------------------------------------------------------------
 * RoomQuestModel 
 *--------------------------------------------------------------------*/
bool RoomQuestModel::mobileCanAggress(PCharacter *pch, NPCharacter *mob)
{
    int ldiff = mob->getModifyLevel() - pch->getModifyLevel();

    if (IS_SET(mob->act, ACT_AGGRESSIVE) && !IS_SET(mob->act, ACT_WIMPY) && ldiff >= -5)
        return true;

    if (IS_SET(mob->act, ACT_VAMPIRE) && ldiff >= -8)
        return true;

    if (IS_AFFECTED( mob, AFF_BLOODTHIRST ))
        return true;

    return false;
}

bool RoomQuestModel::checkRoom( PCharacter *ch, Room *room ) 
{
    if (IS_SET(room->room_flags, ROOM_SOLITARY|ROOM_PRIVATE|ROOM_NO_QUEST|ROOM_MANSION ))
        return false;
    
    if (IS_SET( room->area->area_flag, AREA_NOQUEST ))
        return false;
    
    if (!room->isCommon( ))
        return false;

    if (!room_has_exits(room)) 
        return false;

    return true;
}

bool RoomQuestModel::checkRoomClient( PCharacter *pch, Room *room ) 
{
    if (room->area->low_range > pch->getModifyLevel( ))
        return false;

    if (!checkRoom( pch, room ))
        return false;

    // No aggressive mobs standing nearby the client.
    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        if (rch->is_npc() && mobileCanAggress(pch, rch->getNPC()))
            return false; 
    return true;
}

bool RoomQuestModel::checkRoomVictim( PCharacter *pch, Room *room, NPCharacter *victim ) 
{
    if (IS_SET( room->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE ))
        return false;

    if (IS_SET( room->area->area_flag, AREA_HOMETOWN ))
        return false;

    if (!checkRoom( pch, room ))
        return false;

    // No additional aggrs in victim room for newbie.    
    if (victim) {
        bool hasOtherAggrs = false;
        for (Character *rch = room->people; rch; rch = rch->next_in_room)
            if (rch != victim && rch->is_npc() && mobileCanAggress(pch, rch->getNPC())) {
                hasOtherAggrs = true;
                break;
            }
        
        if (hasOtherAggrs && rated_as_newbie(pch))
            return false;
    }
 
    return true;
}

Room * RoomQuestModel::getRandomRoomClient( PCharacter *pch )
{
    RoomList rooms;
    
    findClientRooms( pch, rooms );
    return getRandomRoom( rooms );
}

Room * RoomQuestModel::getRandomRoom( RoomList &rooms )
{
    if (rooms.empty( ))
        return NULL;
    else
        return rooms[number_range( 0, rooms.size( ) - 1 )];
}

void RoomQuestModel::findClientRooms( PCharacter *pch, RoomList &rooms )
{
    for (Room * r = room_list; r; r = r->rnext)
        if (checkRoomClient( pch, r ))
            rooms.push_back( r );

    if (rooms.empty( ))
        throw QuestCannotStartException( );
}

void RoomQuestModel::findClientRooms( PCharacter *pch, RoomList &rooms, const VnumList &vnums )
{
    Room *r;
    
    for (VnumList::const_iterator v = vnums.begin( ); v != vnums.end( ); v++)
        if (( r = get_room_index( *v ) ))
            if (checkRoomClient( pch, r ))
                rooms.push_back( r );

    if (rooms.empty( ))
        throw QuestCannotStartException( );
}

RoomList RoomQuestModel::findClientRooms(PCharacter *pch, struct area_data *targetArea)
{
    RoomList result;

    for (Room * r = room_list; r; r = r->rnext) {
        if (r->area != targetArea)
            continue;
        if (!checkRoomClient( pch, r ))
            continue;

        result.push_back(r);
    }

    return result;
}

RoomList RoomQuestModel::findVictimRooms(PCharacter *pch, struct area_data *targetArea)
{
    RoomList result;

    for (Room * r = room_list; r; r = r->rnext) {
        if (r->area != targetArea)
            continue;
        if (!checkRoomVictim( pch, r, NULL ))
            continue;

        result.push_back(r);
    }

    return result;
}

AreaList RoomQuestModel::findAreas(PCharacter *pch)
{
    AreaList result;

    for (AREA_DATA *area = area_first; area; area = area->next) {
        if (area->low_range > pch->getRealLevel())
            continue;
        if (IS_SET(area->area_flag, AREA_WIZLOCK|AREA_HOMETOWN|AREA_HIDDEN|AREA_NOQUEST) )
            continue;

        result.push_back(area);
    }

    return result;
}


Room * RoomQuestModel::getDistantRoom( PCharacter *pch, RoomList &rooms, Room *from, int range, int attempts )
{
    int tries = 0;
    
    while (!rooms.empty( ) && tries++ < attempts) {
        int i = number_range( 0, rooms.size( ) - 1 );

        if (room_distance( pch, rooms[i], from, 10000 ) > range)
            return rooms[i];

        rooms.erase( rooms.begin( ) + i );
    }
    
    throw QuestCannotStartException( );
}

/*--------------------------------------------------------------------
 * RoomQuestModel : accessibility checks
 *--------------------------------------------------------------------*/
bool RoomQuestModel::checkRoomForTraverse(PCharacter *pch, Room *room)
{
    if (!room)
        return false;

    if (!room->isCommon()) {
        return false;
    }

    if (!pch->canEnter( room )) {
        return false;
    }
    return true;
}

struct DoorFunc {
    DoorFunc( PCharacter *pch_, RoomQuestModel *rqm ) : pch(pch_), model(rqm) { }

    bool operator () ( Room *const room, EXIT_DATA *exit ) const
    {
        if (IS_SET(exit->exit_info, EX_LOCKED)) {
            return false;
        }

        return model->checkRoomForTraverse(pch, exit->u1.to_room);
    }
    
    PCharacter *pch;
    RoomQuestModel *model;
};

struct ExtraExitFunc {
    ExtraExitFunc( PCharacter *pch_, RoomQuestModel *rqm ) : pch(pch_), model(rqm) { }

    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
        if (IS_SET(eexit->exit_info, EX_LOCKED)) {
            return false;
        }

        return model->checkRoomForTraverse(pch, eexit->u1.to_room);
    }

    PCharacter *pch;
    RoomQuestModel *model;
};

struct PortalFunc {
    PortalFunc( PCharacter *pch_, RoomQuestModel *rqm ) : pch(pch_), model(rqm) { }

    bool operator () ( Room *const room, Object *portal ) const
    {
        if (IS_SET(portal->value[1], EX_LOCKED )) {
            return false;
        }

        return model->checkRoomForTraverse(pch, get_room_index( portal->value[3] ));
    }

    PCharacter *pch;
    RoomQuestModel *model;
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

struct FindPathComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    FindPathComplete( Room *t, std::vector<Room *> &r ) 
            : target( t ), result( r )
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->node != target)
            return false;
        
        for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
            result.push_back( i->node );

        return true;
    }
    
    Room *target;
    std::vector<Room *> &result;
};

/**
 * Ensure there is a path between player and target room that doesn't
 * contain locked doors and rooms they can't go to.
 */
bool RoomQuestModel::targetRoomAccessible(PCharacter *pch, Room *target)
{
    if (pch->getRemorts().size() > 0 || !rated_as_newbie(pch))
        return true;

    std::vector<Room *> rooms;
    MyHookIterator hookIterator(
        DoorFunc(pch, this), 
        ExtraExitFunc(pch, this), 
        PortalFunc(pch, this), 
        5);
    FindPathComplete fpComplete( target, rooms );
    room_traverse<MyHookIterator, FindPathComplete>( 
            pch->in_room, hookIterator, fpComplete, 10000 );
    
    return !rooms.empty();
}

/*--------------------------------------------------------------------
 * ItemQuestModel 
 *--------------------------------------------------------------------*/
bool ItemQuestModel::checkItem( PCharacter *pch, Object *obj )
{
    if (obj->item_type == ITEM_KEY 
        || obj->item_type == ITEM_MAP
        || obj->item_type == ITEM_POTION
        || obj->item_type == ITEM_MONEY)
        return false;

    if (IS_SET( obj->extra_flags, ITEM_ROT_DEATH|ITEM_VIS_DEATH
                                  |ITEM_MELT_DROP|ITEM_NOSAVEDROP
                                  |ITEM_NOREMOVE|ITEM_NODROP ))
        return false;

    if (obj->pIndexData->limit != -1 || obj->timer > 0)
        return false;
    
    if (obj->mustDisappear( pch ))
        return false;
    
    if (obj->isAntiAligned( pch ) || !IS_SET( obj->wear_flags, ITEM_TAKE ))
        return false;

    return true;
}

bool ItemQuestModel::isItemVisible( Object *obj, PCharacter *pch )
{
    if (IS_OBJ_STAT( obj, ITEM_INVIS ) && !gsn_detect_invis->usable( pch ))
        return false;

    return true;
}

Object * ItemQuestModel::getRandomItem( PCharacter *pch )
{
    Object *obj;
    ItemList objects;

    for (obj = object_list; obj; obj = obj->next) {
        if (!checkItem( pch, obj ))
            continue;
        
        if (number_range( 1, obj->pIndexData->count ) == 1)
            objects.push_back( obj );
    }

    if (objects.empty( ))
        throw QuestCannotStartException( );

    return objects[ number_range( 0, objects.size( ) - 1 ) ];
}

void ItemQuestModel::clear( Object *obj )
{
    if (obj) {
        obj->behavior->unsetObj( );
        obj->behavior.clear( );
    }
}

void ItemQuestModel::destroy( Object *obj )
{
    if (obj) {
        obj->behavior.clear( );
        extract_obj( obj );
    }
}

/*--------------------------------------------------------------------
 * MobileQuestModel 
 *--------------------------------------------------------------------*/
bool MobileQuestModel::checkMobile( PCharacter *pch, NPCharacter *mob )
{
    if (IS_CHARMED(mob))
        return false;

    if (mob->behavior 
        && (mob->behavior->hasDestiny( ) 
            || IS_SET(mob->behavior->getOccupation( ), (1 << OCC_SHOPPER))))
        return false;
    
    if (IS_SET( mob->pIndexData->area->area_flag, AREA_NOQUEST ))
        return false;

    if (!mob->in_room)
        return false;
    
    return true;
}

bool MobileQuestModel::isMobileVisible( NPCharacter *mob, PCharacter *pch )
{
    if (IS_AFFECTED( mob, AFF_CAMOUFLAGE ) && !gsn_acute_vision->usable( pch ))
        return false;

    if (gsn_truesight->usable( pch, false ))
        return true;

    if (IS_AFFECTED( mob, AFF_HIDE ) && !gsn_detect_hide->usable( pch ))
        return false;

    if (IS_AFFECTED( mob, AFF_INVISIBLE ) && !gsn_detect_invis->usable( pch ))
        return false;

    if (IS_AFFECTED( mob, AFF_IMP_INVIS ) && !gsn_improved_detect->usable( pch ))
        return false;

    return true;
}

NPCharacter * MobileQuestModel::getRandomMobile( MobileList &mobiles )
{
    if (mobiles.empty( ))
        return NULL;
    else
        return mobiles[ number_range( 0, mobiles.size( ) - 1 ) ];
}

mob_index_data * MobileQuestModel::getRandomMobIndex( MobIndexMap &map )
{
    int n = number_range( 0, map.size( ) - 1 ), count = 0;

    for (MobIndexMap::iterator m = map.begin( ); m != map.end( ); m++, count++)
        if (count == n) 
            return m->first;

    return NULL;
}

void MobileQuestModel::clear( NPCharacter *mob )
{
    if (mob) {
        MobileBehaviorManager::assignBasic( mob );
        save_mobs( mob->in_room );
    }
}

void MobileQuestModel::destroy( NPCharacter *mob )
{
    if (mob) {
        mob->behavior.clear( );
        extract_char( mob );
    }
}

/*--------------------------------------------------------------------
 * VictimQuestModel 
 *--------------------------------------------------------------------*/
bool VictimQuestModel::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    if (!checkMobile( pch, mob ))
        return false;

    if (IS_SET( mob->act,  ACT_NOTRACK | ACT_SAGE ))
        return false;

    if (IS_SET( mob->imm_flags, IMM_SPELL|IMM_WEAPON ))
        return false;

    if (mob->behavior && mob->behavior->getOccupation( ) & OCC_PRACTICER)
        return false;

    if (mob->fighting)
        return false;
    
    if ((IS_EVIL(mob) && IS_EVIL(pch)) || (IS_GOOD(mob) && IS_GOOD(pch)))
        return false;

    if (mob->in_room->area != mob->pIndexData->area)
        return false;

    if (mob->getRealLevel( ) != mob->pIndexData->level)
        return false;

    return checkRoomVictim( pch, mob->in_room, mob );
}

void VictimQuestModel::findVictims( PCharacter *pch, MobileList &victims )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (mob && checkMobileVictim( pch, mob ))
            victims.push_back( mob );
    }

    if (victims.empty( ))
        throw QuestCannotStartException( );
}

void VictimQuestModel::findVictims( PCharacter *pch, MobIndexMap &victims )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (mob && checkMobileVictim( pch, mob ))
            victims[mob->pIndexData].push_back( mob );
    }

    if (victims.empty( ))
        throw QuestCannotStartException( );
}

NPCharacter * VictimQuestModel::getRandomVictim( PCharacter *pch )
{
    MobileList victims;

    findVictims( pch, victims );
    NPCharacter *result = getRandomMobile( victims );

    if (!targetRoomAccessible(pch, result->in_room))
        throw QuestCannotStartException( );

    return result;    
}


/*--------------------------------------------------------------------
 * ClientQuestModel 
 *--------------------------------------------------------------------*/
bool ClientQuestModel::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if ((IS_GOOD(pch) && IS_EVIL(mob)) || (IS_EVIL(pch) && IS_GOOD(mob)))
        return false;

    if (!IS_AWAKE( mob ))
        return false;

    if (mobileCanAggress(pch, mob))
        return false;

    if (!checkMobile( pch, mob ))
        return false;
    
    return checkRoomClient( pch, mob->in_room );
}

void ClientQuestModel::findClients( PCharacter *pch, MobileList &clients )
{
    Character *wch;
    NPCharacter *mob;

    for (wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (mob && checkMobileClient( pch, mob ))
            clients.push_back( mob );
    }

    if (clients.empty( ))
        throw QuestCannotStartException( );
}

void ClientQuestModel::findClients( PCharacter *pch, MobIndexMap &clients )
{
    NPCharacter *mob;

    for (Character *wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (mob && checkMobileClient( pch, mob ))
            clients[mob->pIndexData].push_back( mob );
    }

    if (clients.empty( ))
        throw QuestCannotStartException( );
}


NPCharacter * ClientQuestModel::getRandomClient( PCharacter *pch )
{
    MobileList clients;

    findClients( pch, clients );
    NPCharacter *result = getRandomMobile( clients );

    if (!targetRoomAccessible(pch, result->in_room))
        throw QuestCannotStartException( );

    return result;    
}


