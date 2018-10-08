/* $Id: cfind.cpp,v 1.1.2.2.6.5 2014-09-19 11:50:42 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sys/time.h>

#include "commandtemplate.h"
#include "roomtraverse.h"

#include "class.h"
#include "character.h"
#include "object.h"
#include "merc.h"
#include "def.h"

struct GoDoor
{
    inline bool operator () ( Room *r, EXIT_DATA * a ) const
    {
        return true;
    }
};
struct GoEExit
{
    inline bool operator () ( Room *r, EXTRA_EXIT_DATA * a ) const
    {
        return true;
    }
};
struct GoPortal
{
    inline bool operator () ( Room *r, Object * a ) const
    {
        return true;
    }
};

typedef RoomRoadsIterator<GoDoor, GoEExit, GoPortal> MyHookIterator; 

struct FindComplete {
    
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    FindComplete( Room *t, RoomTraverseResult &r ) 
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

#define ROOM_VNUM_MSM 3014

CMDRUN( find )
{
    ostringstream ostr;
    int target;
    Room *toRoom;
    RoomTraverseResult elements;
    MyHookIterator iter;
    int radius = 10000;
    Room *msm = ch->in_room;

    try {
        target = constArguments.toInt( );
        if (target <= 0)
            return;
    } catch (ExceptionBadType e) {
        ch->send_to( "Usage: find <room vnum>\r\n" );
        return;
    }
    
    if (!(toRoom = get_room_index( target )))
        return;
    
    for (Room *r = room_list; r; r = r->rnext)
        REMOVE_BIT(r->room_flags, ROOM_MARKER);
    
    FindComplete complete( toRoom, elements );
    struct timeval tv1, tv2;

    gettimeofday(&tv1, NULL);
    room_traverse<MyHookIterator>( msm, iter, complete, radius );
    gettimeofday(&tv2, NULL);

    long long l;
    l = tv2.tv_sec - tv1.tv_sec;
    l *= 1000000;
    l += tv2.tv_usec - tv1.tv_usec;
    
    ostr << "invocation time: " << l << endl;

    ostr << "found way: ";
    make_speedwalk( elements, ostr );
    ostr << endl;
    
    ch->send_to( ostr );
}

