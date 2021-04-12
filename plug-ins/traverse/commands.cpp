/* $Id: cfind.cpp,v 1.1.2.2.6.5 2014-09-19 11:50:42 rufina Exp $
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"
#include "roomtraverse.h"

#include "loadsave.h"
#include "merc.h"
#include "def.h"

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


/*--------------------------------------------------------------------------
 * 'find' command for immortals, finds paths without any checks
 *--------------------------------------------------------------------------*/

struct GoDoorAlways
{
    inline bool operator () ( Room *r, EXIT_DATA * a ) const
    {
        return true;
    }
};
struct GoEExitAlways
{
    inline bool operator () ( Room *r, EXTRA_EXIT_DATA * a ) const
    {
        return true;
    }
};
struct GoPortalAlways
{
    inline bool operator () ( Room *r, Object * a ) const
    {
        return true;
    }
};

typedef RoomRoadsIterator<GoDoorAlways, GoEExitAlways, GoPortalAlways> NoFailHookIterator; 

#define ROOM_VNUM_MSM 3014

CMDRUN( find )
{
    ostringstream ostr;
    int target;
    Room *toRoom;
    RoomTraverseResult elements;
    GoDoorAlways goDoor; GoEExitAlways goEExit; GoPortalAlways goPortal;    
    NoFailHookIterator iter(goDoor, goEExit, goPortal);
    int radius = 10000;
    Room *msm = ch->in_room;

    try {
        target = constArguments.toInt( );
        if (target <= 0)
            return;
    } catch (const ExceptionBadType &e) {
        ch->pecho("Usage: find <room vnum>");
        return;
    }
    
    if (!(toRoom = get_room_instance( target )))
        return;
    
    for (auto &r: roomInstances)
        REMOVE_BIT(r->room_flags, ROOM_MARKER);
    
    FindComplete complete( toRoom, elements );
    room_traverse<NoFailHookIterator>( msm, iter, complete, radius );

    ostr << "Found path: ";
    make_speedwalk( elements, ostr );
    ostr << endl;
    
    ch->send_to( ostr );
}

/*--------------------------------------------------------------------------
 * 'path' command to construct speedwalks in the same area
 *--------------------------------------------------------------------------*/
struct GoDoorSameArea
{
    GoDoorSameArea(Character *ch) {
        this->ch = ch;
        this->area = ch->in_room->area;
    }
    inline bool operator () (Room *r, EXIT_DATA * exit) const
    {
        Room *to_room = exit->u1.to_room;
        return to_room && to_room->area == area && ch->canEnter(to_room);
    }

    Character *ch;
    Area *area;
};

struct GoEExitSameArea
{
    GoEExitSameArea(Character *ch) {
        this->ch = ch;
        this->area = ch->in_room->area;
    }

    inline bool operator () (Room *r, EXTRA_EXIT_DATA *eexit) const
    {
        Room *to_room = eexit->u1.to_room;
        return to_room && to_room->area == area && ch->canEnter(to_room);
    }

    Character *ch;
    Area *area;
};

struct GoPortalSameArea
{
    GoPortalSameArea(Character *ch) {
        this->ch = ch;
        this->area = ch->in_room->area;
    }

    inline bool operator () (Room *r, Object *portal) const
    {
        if (IS_SET(portal->value2(), GATE_RANDOM|GATE_BUGGY))
            return false;

        Room *to_room = get_room_instance(portal->value3());
        return to_room && to_room->area == area && ch->canEnter(to_room);
    }

    Character *ch;
    Area *area;
};

typedef RoomRoadsIterator<GoDoorSameArea, GoEExitSameArea, GoPortalSameArea> SameAreaHookIterator; 

CMDRUN( path )
{
    DLString roomName = constArguments;
    Room *target = 0;
    Area *myArea = ch->in_room->area;

    if (roomName.empty()) {
        ch->pecho("Укажи название комнаты, куда нужно проложить путь.");
        return;
    }

    if (IS_SET(myArea->area_flag, AREA_CLAN|AREA_DUNGEON|AREA_MANSION|AREA_WIZLOCK|AREA_SYSTEM)) {
        ch->pecho("К сожалению, в этой зоне проложить путь не получится.");
        return;
    }

    if (eyes_blinded(ch)) {
        ch->pecho("Ты не сможешь проложить путь, если даже не видишь, где сейчас находишься.");
        return;
    }

    for (auto &r: myArea->rooms)
        if (is_name(roomName.c_str(), r.second->getName())) {
            target = r.second;
            break;
        }

    if (!target) {
        ch->pecho("Не могу найти комнату с названием '%s' в зоне {c%s{x.", 
                  roomName.c_str(), ch->in_room->areaName());
        return;
    }

    if (target == ch->in_room) {
        ch->pecho("Но ты уже здесь!");
        return;
    }

    RoomTraverseResult elements;
    GoDoorSameArea goDoor(ch); GoEExitSameArea goEExit(ch); GoPortalSameArea goPortal(ch);
    SameAreaHookIterator iter(goDoor, goEExit, goPortal);    
    FindComplete complete(target, elements);
    int radius = 10000;

    room_traverse<SameAreaHookIterator>(ch->in_room, iter, complete, radius);

    if (elements.empty()) {
        ch->pecho("Не удалось проложить путь к комнате '%s'.", target->getName());
        return;
    }

    ostringstream pathBuf;
    make_speedwalk(elements, pathBuf);
    ch->pecho("Путь к комнате '%s': %s", target->getName(), pathBuf.str().c_str());
    ch->pecho("Не забывай, что на пути могут встретиться запертые или потайные выходы."); 
}

