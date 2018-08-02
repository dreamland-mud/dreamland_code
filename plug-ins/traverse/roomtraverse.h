/* $Id: roomtraverse.h,v 1.1.2.4.6.2 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __ROOMTRAVERSE_H__
#define __ROOMTRAVERSE_H__

#include "traverse.h"

#include "character.h"
#include "object.h"
#include "room.h"
#include "act_move.h"
#include "itemflags.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

struct Road {
    enum {
	NONE,
	DOOR,
	EEXIT,
	PORTAL,
    } type;
    
    union {
	int door;
	EXTRA_EXIT_DATA *eexit;
	Object *portal;
    } value;
    
    inline Road( ) : type( NONE )
    {
    }

    inline Room * target( Room * const from ) const
    {
	switch (type) {
	default:
	    return NULL;
	case DOOR:
	    return from->exit[value.door]->u1.to_room;
	case EEXIT:
	    return value.eexit->u1.to_room;
	case PORTAL:
	    return get_room_index( value.portal->value[3] );
	}
    }

    inline const DLString speedwalk( ) const
    {
	DLString str;
	
	switch (type) {
	default:
	    return DLString( );
	case DOOR:
	    str.assign( dirs[value.door].name[0] );
	    return str;
	case EEXIT:
	    str = value.eexit->keyword;
	    str = str.getOneArgument( );
	    return "walk " + str;
	case PORTAL:
	    str = value.portal->getName( );
	    str = str.getOneArgument( );
	    return "enter " + str;
	}
    }
};

typedef std::list<Road> RoomTraverseResult;

struct RoomTraverseTraits {
    typedef Room NodeType;
    typedef Road HookType;
    
    inline static void mark( NodeType *const node )
    {
	SET_BIT( node->room_flags, ROOM_MARKER );
    }
    inline static void unmark( NodeType *const node )
    {
	REMOVE_BIT( node->room_flags, ROOM_MARKER );
    }
    inline static bool marked( const NodeType *const node )
    {
	return IS_SET( node->room_flags, ROOM_MARKER );
    }
};

template <typename DoorCondFunc, typename EExitCondFunc, typename PortalCondFunc>
struct RoomRoadsIterator 
{
    RoomRoadsIterator( DoorCondFunc d = DoorCondFunc( ), 
	               EExitCondFunc e = EExitCondFunc( ), 
		       PortalCondFunc p = PortalCondFunc( ), 
		       int s = 0 )
	    : canGoDoor( d ), canGoEExit( e ), canGoPortal( p ), seed( s )
    {
    }
    
    template <typename Action>
    inline void operator () ( Room *const room, Action act ) const
    {
	Road road;
	int dir, i, j0, j1;
	int shuffle[DIR_SOMEWHERE];

	for (dir = 0; dir < DIR_SOMEWHERE; dir++) 
	    shuffle[dir] = dir;
	
	for (i = 0; i < seed; i++) {
	    j0 = i % DIR_SOMEWHERE;
	    j1 = number_mm( ) % DIR_SOMEWHERE;

	    dir = shuffle[j0];
	    shuffle[j0] = shuffle[j1];
	    shuffle[j1] = dir;
	}

	for (dir = 0; dir < DIR_SOMEWHERE; dir++) {
	    EXIT_DATA *exit = room->exit[shuffle[dir]];

	    if (!exit || !exit->u1.to_room) 
		continue;

	    if (!canGoDoor( room, exit ))
		continue;

	    road.type = Road::DOOR;
	    road.value.door = shuffle[dir];

	    act( road );
	}

	for (EXTRA_EXIT_DATA *eexit = room->extra_exit; eexit; eexit = eexit->next) {
	    if (!eexit->u1.to_room)
		continue;

	    if (!canGoEExit( room, eexit ))
		continue;

	    road.type = Road::EEXIT;
	    road.value.eexit = eexit;

	    act( road );
	}

	for (Object *obj = room->contents; obj; obj = obj->next_content) {
	    if (obj->item_type != ITEM_PORTAL)
		continue;
	    
	    if (IS_SET(obj->value[2], GATE_RANDOM|GATE_BUGGY)) 
		continue;

	    if (get_room_index( obj->value[3] ) == NULL)
		continue;

	    if (!canGoPortal( room, obj ))
		continue;

	    road.type = Road::PORTAL;
	    road.value.portal = obj;

	    act( road );
	}
    }
    
    DoorCondFunc const &canGoDoor;
    EExitCondFunc const &canGoEExit;
    PortalCondFunc const &canGoPortal;

    int seed; /* seed of disorder */
};


template <typename HookIterator, typename RoomTraverseComplete>
void room_traverse( Room *src, HookIterator &iter, 
                    RoomTraverseComplete complete, int limit )
{
    typedef 
	BroadTraverse<RoomTraverseTraits, 
	              HookIterator, RoomTraverseComplete> RoomTraverse;

    RoomTraverse traverse( iter, complete );
    
    traverse( src, limit );
}

void make_speedwalk( RoomTraverseResult &elements, ostringstream & );
int room_distance( Character *ch, Room *src, Room *dst, int limit = 10000 );
Road room_first_step( Character *ch, Room *start_room, Room *target_room,
                     bool fDoor, bool fExtra, bool fPortal, int limit = 10000 );
#endif
