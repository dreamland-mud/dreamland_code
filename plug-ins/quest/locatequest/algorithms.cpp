/* $Id: algorithms.cpp,v 1.1.2.5.10.2 2014-09-19 11:47:31 rufina Exp $
 *
 * ruffina, 2004
 */
#include "algorithms.h"

#include "questexceptions.h"


#include "logstream.h"
#include "pcharacter.h"
#include "object.h"

#include "merc.h"
#include "roomtraverse.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * structs for room traverse 
 *----------------------------------------------------------------------------*/
struct DoorFunc {
    DoorFunc( ) : ch( 0 ) { }
    DoorFunc( Character *wch ) : ch( wch ) { }

    bool operator () ( Room *const, EXIT_DATA *exit ) const
    {
        Room *room = exit->u1.to_room;
        
        if (!room || !ch)
            return false;

        if (IS_SET(room->room_flags, ROOM_SOLITARY|ROOM_PRIVATE ))
            return false;

        if (IS_WATER(room))
            return false;

        if (IS_SET( room->area->area_flag, AREA_NOQUEST ))
            return false;

        if (room->area->low_range > ch->getModifyLevel( ))
            return false;

        if (!ch->canEnter( room ))
            return false;
        
        return true;
    }
    
    Character *ch;
};
struct ExtraExitFunc {
    ExtraExitFunc( ) { }

    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
        return false;
    }
};
struct PortalFunc {
    PortalFunc( ) { }

    bool operator () ( Room *const room, Object *portal ) const
    {
        return false;
    }
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

/*-----------------------------------------------------------------------------
 * find path between two points 
 *----------------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------------
 * find rooms near target room 
 *----------------------------------------------------------------------------*/
struct RoomsNearComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    RoomsNearComplete( Room *s, int rd, std::vector<Room *> &r ) 
            : src( s ), radius( rd ), rooms( r )
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->generation < radius && !last)
            return false;
        
        const MyNodesEntry *i;

        for (i = head; i->prev; i = i->prev) 
            ;
        
        for ( ; i != head; i++)
            if (i->node != src)
                rooms.push_back( i->node );

        return true;
    }
    
    Room *src;
    int radius;
    std::vector<Room *> &rooms;
};

/*-----------------------------------------------------------------------------
 * "middle-point" scatter algorithm 
 *----------------------------------------------------------------------------*/

bool LocateMiddlePointAlgo::needsEndPoint( ) const
{
    return true;
}

int LocateMiddlePointAlgo::getRadius( ) const
{
    return number_range( 4, 10 );
}

void LocateMiddlePointAlgo::findRooms( 
        PCharacter *pch, Room *src, Room *target, LocateAlgo::Rooms &rooms ) const
{
    int i;
    DoorFunc df( pch ); ExtraExitFunc eef; PortalFunc pf;
    MyHookIterator iter( df, eef, pf, 5 );

    FindPathComplete fpComplete( target, rooms );
    room_traverse<MyHookIterator, FindPathComplete>( 
            src, iter, fpComplete, 10000 );

    if (rooms.empty( ))
        throw QuestCannotStartException( );
    
    i = URANGE( 0, number_fuzzy( rooms.size( ) / 2 ), (int)rooms.size( ) - 1 );
    src = rooms[i];
    rooms.clear( );

    RoomsNearComplete rnComplete( src, getRadius( ), rooms );
    room_traverse<MyHookIterator, RoomsNearComplete>( 
            src, iter, rnComplete, 10000 );
    
    if (rooms.empty( ))
        throw QuestCannotStartException( );
}
    
/*-----------------------------------------------------------------------------
 * "radial" scatter algorithm 
 *----------------------------------------------------------------------------*/
bool LocateRadialAlgo::needsEndPoint( ) const
{
    return false;
}

int LocateRadialAlgo::getRadius( ) const
{
    return number_range( 6, 16 );
}

void LocateRadialAlgo::findRooms( 
        PCharacter *pch, Room *src, Room *, LocateAlgo::Rooms &rooms ) const
{
    DoorFunc df( pch ); ExtraExitFunc eef; PortalFunc pf;
    MyHookIterator iter( df, eef, pf, 5 );

    RoomsNearComplete rnComplete( src, getRadius( ), rooms );
    room_traverse<MyHookIterator, RoomsNearComplete>( 
            src, iter, rnComplete, 10000 );
   
    if (rooms.empty( ))
        throw QuestCannotStartException( );
}

/*-----------------------------------------------------------------------------
 * "uniform" scatter algorithm 
 *----------------------------------------------------------------------------*/
bool LocateUniformAlgo::needsEndPoint( ) const
{
    return true;
}

void LocateUniformAlgo::findRooms( 
        PCharacter *pch, Room *src, Room *target, LocateAlgo::Rooms &rooms ) const
{
    DoorFunc df( pch ); ExtraExitFunc eef; PortalFunc pf;
    MyHookIterator iter( df, eef, pf, 5 );

    FindPathComplete fpComplete( target, rooms );
    room_traverse<MyHookIterator, FindPathComplete>( 
            src, iter, fpComplete, 10000 );

    if (rooms.empty( ))
        throw QuestCannotStartException( );
}
