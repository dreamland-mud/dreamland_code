/* $Id: wanderer.cpp,v 1.1.2.5.6.9 2014-09-19 11:50:43 rufina Exp $
 *
 * ruffina, 2005
 */

#include "pcharacter.h"
#include "npcharacter.h"
#include "act_move.h"
#include "exitsmovement.h"
#include "portalmovement.h"
#include "loadsave.h"
#include "interp.h"
#include "act.h"

#include "wanderer.h"

Wanderer::Wanderer( )
{
}

bool Wanderer::canEnter( Room *const room )
{
    return ch->canEnter( room ) && !room->isPrivate( );
}

bool Wanderer::canWander( Room *const room, int door )
{
    EXIT_DATA *exit = room->exit[door];
    
    if (!exit || !ch->can_see( exit ))
        return false;

    return canWander( room, exit );
}

bool Wanderer::canWander( Room *const room, EXIT_DATA *exit )
{
    if (IS_SET( exit->exit_info, EX_LOCKED ) && IS_SET( exit->exit_info, EX_NOPASS ))
        return false;
    
    return canEnter( exit->u1.to_room );
}

bool Wanderer::canWander( Room *const room, EXTRA_EXIT_DATA *eexit )
{
    if (!ch->can_see( eexit ))
        return false;

    return true;
}

bool Wanderer::canWander( Room *const room, Object *portal )
{
    if (!portal->pIndexData || portal->pIndexData->item_type != ITEM_PORTAL)
        return false;

    if (!ch->can_see( portal ))
        return false;
        
    if (IS_SET(portal->value[1], EX_LOCKED ))
        return false;

    return true;
}


/////////////////////////////////////////////////////////////////////////////
struct DoorFunc {
    
    DoorFunc( Wanderer *bhv ) : behavior( bhv ) { }

    bool operator () ( Room *const room, EXIT_DATA *exit ) const
    {
        return behavior->canWander( room, exit );
    }
    
    Wanderer *behavior;
};

struct ExtraExitFunc {
    
    ExtraExitFunc( Wanderer *bhv ) : behavior( bhv ) { }

    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
        return behavior->canWander( room, eexit );
    }
    
    Wanderer *behavior;
};

struct PortalFunc {
    
    PortalFunc( Wanderer *bhv ) : behavior( bhv ) { }

    bool operator () ( Room *const room, Object *portal ) const
    {
        return behavior->canWander( room, portal );
    }
    
    Wanderer *behavior;
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

struct PathToTargetComplete {
    
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    PathToTargetComplete( Room *t, RoomTraverseResult &r ) 
            : target( t ), result( r )
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->node != target)
            return false;
        
        for (const MyNodesEntry *i = head; i->prev; i = i->prev) {
            Road road = i->hook;
            
            result.push_front( road );
        }

        return true;
    }
    
    Room *target;
    RoomTraverseResult &result;
};

struct PathWithDepthComplete {
    
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    PathWithDepthComplete( int d, RoomTraverseResult &r ) 
            : depth( d ), result( r )
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->generation < depth && !last)
            return false;
        
        for (const MyNodesEntry *i = head; i->prev; i = i->prev) {
            Road road = i->hook;
            
            result.push_front( road );
        }

        return true;
    }
    
    int depth;
    RoomTraverseResult &result;
};

/////////////////////////////////////////////////////////////////////////////////

void Wanderer::pathToTarget( Room *const start_room, Room *const target_room, int limit )
{
    MyHookIterator iter( DoorFunc( this ), 
                         ExtraExitFunc( this ), 
                         PortalFunc( this ) );

    PathToTargetComplete complete( target_room, path );
    
    room_traverse<MyHookIterator, PathToTargetComplete>( 
           start_room, iter, complete, limit );
}

void Wanderer::pathWithDepth( Room *const start_room, int depth, int limit )
{
    MyHookIterator iter( DoorFunc( this ), 
                         ExtraExitFunc( this ), 
                         PortalFunc( this ), 10 );
    
    PathWithDepthComplete complete( depth, path );

    room_traverse<MyHookIterator, PathWithDepthComplete>( 
            start_room, iter, complete, limit );
}

bool Wanderer::handleMoveResult( Road &road, int rc )
{
    switch (rc) {
    case RC_MOVE_CLOSED:
        switch (road.type) {
        case Road::DOOR:
            open_door( ch, road.value.door );
            return false;
            
        case Road::EEXIT:
            open_door_extra( ch, DIR_SOMEWHERE, road.value.eexit );
            return false;

        case Road::PORTAL:
            open_portal( ch, road.value.portal );
            return false;
        
        default:
            break;
        }
        
    case RC_MOVE_PASS_FAILED:
        return false;
        
    case RC_MOVE_RESTING:
        interpret_cmd( ch, "wake", "" );
        return false;
    }

    return true;
}

int Wanderer::moveOneStep( int door )
{
    return ExitsMovement( ch, door, MOVETYPE_WALK ).move( );
}

int Wanderer::moveOneStep( EXTRA_EXIT_DATA *peexit )
{
    return ExitsMovement( ch, peexit, MOVETYPE_WALK ).move( );
}

int Wanderer::moveOneStep( Object *portal )
{
    return PortalMovement( ch, portal ).move( );
}

bool Wanderer::makeOneStep( Road &road )
{
    int rc = RC_MOVE_UNDEF;
    
    switch (road.type) {
    case Road::DOOR:
        if (canWander( ch->in_room, road.value.door ))
            rc = moveOneStep( road.value.door );

        break;
        
    case Road::EEXIT:
        if (canWander( ch->in_room, road.value.eexit ))
            rc = moveOneStep( road.value.eexit );

        break;
        
    case Road::PORTAL:
        if (canWander( ch->in_room, road.value.portal ))
            rc = moveOneStep( road.value.portal );
        
        break;

    default:
        break;
    }

    return handleMoveResult( road, rc );
}

void Wanderer::makeOneStep( )
{
    Room *old_room = ch->in_room;

    if (path.empty( ))
        return;
        
    Road road = path.front( );

    if (!makeOneStep( road ))
        return;
    
    if (ch->in_room == old_room) {
        path.clear( );
        return;
    }
    
    path.pop_front( );
}

bool 
Wanderer::makeSpeedwalk( Room *start_room, Room *target_room, ostringstream &buf )
{
    if (!start_room || !target_room)
        return false;

    path.clear( );
    pathToTarget( start_room, target_room, 10000 );
    
    if (path.empty( ))
        return false;
        
    make_speedwalk( path, buf );
    path.clear( );
    return true;
}
