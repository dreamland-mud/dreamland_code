/* $Id: roomtraverse.cpp,v 1.1.2.3.6.2 2014-09-19 11:50:42 rufina Exp $
 *
 * ruffina, 2004
 */

#include "roomtraverse.h"

/*
 * make_speedwalk: constructs speedwalk string based on Road's vector
 */
void make_speedwalk( RoomTraverseResult &elements, ostringstream &buf )
{
    RoomTraverseResult::iterator i;
    bool colon = false;

    for (i = elements.begin( ); i != elements.end( ); i++) {
        DLString swalk = i->speedwalk( );
        
        if (colon && swalk.size( ) <= 1)
            buf << ";";
            
        colon = (swalk.size( ) > 1);
        
        if (colon)
            buf << ";";
        
        buf << swalk;
    }
}

/*
 * room_distance: calculates minimal distance between two rooms for this player
 */

bool checkRoom( Character *ch, Room * room )
{
    if (!ch->canEnter( room ))
        return false;
    
    if (room->isPrivate( ))
        return false;

    return true;
}

struct DoorFunc {
    DoorFunc( Character *wch, bool e = true ) : ch( wch ), enabled( e ) { }

    bool operator () ( Room *const room, EXIT_DATA *exit ) const
    {
        return enabled && checkRoom( ch, exit->u1.to_room );
    }
    
    Character *ch;
    bool enabled;
};

struct ExtraExitFunc {
    ExtraExitFunc( Character *wch, bool e = true ) : ch( wch ), enabled( e ) { }

    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
        return enabled && checkRoom( ch, eexit->u1.to_room );
    }
    
    Character *ch;
    bool enabled;
};

struct PortalFunc {
    PortalFunc( Character *wch, bool e = true ) : ch( wch ), enabled( e ) { }

    bool operator () ( Room *const room, Object *portal ) const
    {
        return enabled && checkRoom( ch, get_room_index( portal->value3() ) );
    }
    
    Character *ch;
    bool enabled;
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

struct RoomDistanceComplete 
{
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;

    RoomDistanceComplete( Room *t, int &d, RoomTraverseResult &p ) 
                : target( t ), distance( d ), path( p ) 
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->node == target) {
            distance = head->generation;
            
            for (const MyNodesEntry *i = head; i->prev; i = i->prev) {
                Road road = i->hook;
                
                path.push_front( road );
            }

            return true;
        }

        return false;
    }
    
    Room *target;
    int &distance;
    RoomTraverseResult &path;
};

typedef 
    BroadTraverse<RoomTraverseTraits, 
                  MyHookIterator, RoomDistanceComplete> RoomDistanceTraverse;

int room_distance( Character *ch, Room *src, Room *dst, int limit )
{
    RoomTraverseResult dummy;
    DoorFunc df( ch );
    ExtraExitFunc eef( ch );
    PortalFunc pf( ch );
    MyHookIterator iter( df, eef, pf );
    int distance = -1;

    RoomDistanceComplete complete( dst, distance, dummy );
    RoomDistanceTraverse traverse( iter, complete );
    
    traverse( src, limit );
    return distance;
}

/*
 * room_first_step: find first direction step from room to room for this char 
 */
Road room_first_step( Character *ch, Room *start_room, Room *target_room,
                     bool fDoor, bool fExtra, bool fPortal, int limit )
{
    RoomTraverseResult result;
    DoorFunc df( ch, fDoor );
    ExtraExitFunc eef( ch, fExtra );
    PortalFunc pf( ch, fPortal );
    MyHookIterator iter( df, eef, pf, 10 );
    int distance;
    Road road;

    RoomDistanceComplete complete( target_room, distance, result );
    RoomDistanceTraverse traverse( iter, complete );
    
    traverse( start_room, limit );

    if (!result.empty( ))
        road = result.front( );

    return road;
}

