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

// A functor to check if we can traverse further through this door. Only allow doors
// leading to the same area and where character is allowed to enter.
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

// A functor to check if we can traverse further through this extra exit. Only
// allow exits leading within same area.
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

// A functor to check if we can traverse further through this portal on the floor.
// Only allow portals leading to the same area. Exclude random portals. 
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
    list<Room *> targets;
    Area *myArea = ch->in_room->area;
    unsigned int maxTargets = 5;
    unsigned int matches = 0;

    if (roomName.empty() || DLString(constArguments).getOneArgument().empty()) {
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
            matches++;
            if (targets.size() < maxTargets)
                targets.push_back(r.second);
        }

    if (targets.empty()) {
        ch->pecho("Не могу найти комнату с названием '%s' в зоне {c%s{x.", 
                  roomName.c_str(), ch->in_room->areaName());
        return;
    }

    if (targets.size() == 1 && targets.front() == ch->in_room) {
        ch->pecho("Но ты уже здесь!");
        return;
    }

    // Limit BFS radius to a number of steps, normally enough to traverse even a very big area.
    int radius = 10000;

    // Define iterator to navigate from room to room during the traverse.
    GoDoorSameArea goDoor(ch); GoEExitSameArea goEExit(ch); GoPortalSameArea goPortal(ch);
    SameAreaHookIterator iter(goDoor, goEExit, goPortal);

    // Display results for the first N matches, worn about the rest.
    ostringstream buf;
    bool foundPath = false;
    for (Room *target: targets) {
        RoomTraverseResult elements;
        FindComplete complete(target, elements);
        room_traverse<SameAreaHookIterator>(ch->in_room, iter, complete, radius);

        buf << "    путь к '{W" << target->getName() << "{w'";

        if (elements.empty()) {
            if (ch->in_room == target)
                buf << ": ты уже здесь!" << endl;
            else
                buf << " не найден" << endl;
            continue;
        }
 
        buf << ": ";
        make_speedwalk(elements, buf);
        buf << endl;
        foundPath = true;
    }

    if (foundPath)
        ch->pecho("Найдены такие комнаты и пути к ним в зоне {c%s{x:", ch->in_room->areaName());
    else
        ch->pecho("Не удалось проложить путь ни к одной из комнат:");

    ch->send_to(buf);

    if (foundPath)
        ch->pecho("Не забывай, что на пути могут встретиться запертые или потайные выходы."); 

    if (matches > targets.size())
        ch->pecho("Всего найдено {W%1$d{x подходящ%1$Iая|ие|их комна%1$Iта|ты|т, уточни название, чтобы увидеть остальные.",
                  matches);
}

