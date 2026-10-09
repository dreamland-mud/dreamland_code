/* $Id$
 *
 * ruffina, 2004
 */
#include "directions.h"

#include "room.h"
#include "pcharacter.h"
#include "core/object.h"

#include "merc.h"

#include "def.h"

/*
 * RoomHistory 
 */
const unsigned int RoomHistory::MAX_SIZE = 10;

static RoomHistoryEntry footprint( Character *ch, int door, long long portal, extra_exit_data *eexit )
{
    return RoomHistoryEntry( ch->getPC()->getName( ),
                             ch->getPC()->getRussianName().getFullForm(),
                             door, portal, eexit );
}

void RoomHistory::record( Character *ch, int door )
{
    if (ch->is_npc( ) || door < 0 || door >= DIR_SOMEWHERE)
        return;

    erase( );
    push_front( footprint( ch, door, 0, 0 ) );
}

void RoomHistory::record( Character *ch, Object *portal )
{
    if (ch->is_npc( ) || !portal)
        return;

    erase( );
    push_front( footprint( ch, DIR_SOMEWHERE, portal->getID( ), 0 ) );
}

void RoomHistory::record( Character *ch, extra_exit_data *eexit )
{
    if (ch->is_npc( ) || !eexit)
        return;

    erase( );
    push_front( footprint( ch, DIR_SOMEWHERE, 0, eexit ) );
}

void RoomHistory::erase( )
{
    if (size( ) > MAX_SIZE)
        pop_back( );
}

const RoomHistoryEntry * RoomHistory::find( Character *ch ) const
{
    if (ch->is_npc( ))
        return 0;

    DLString arg( ch->getPC()->getName( ) );
    return find( arg, true );
}

const RoomHistoryEntry * RoomHistory::find( DLString &arg, bool fStrict ) const
{
    bool rus = arg.isCyrillic();

    for (const_iterator h = begin( ); h != end( ); h++) {
        DLString name = rus ? h->rname.ruscase('1') : h->name;

        if ((fStrict && name == arg)
            || is_name( arg.c_str( ), name.c_str( ) ))
        {
            arg = rus ? h->rname : h->name;
            return &*h;
        }
    }

    return 0;
}

int RoomHistory::went( Character *ch ) const
{
    const RoomHistoryEntry *h = find( ch );
    return (h && h->went < DIR_SOMEWHERE ? h->went : -1);
}

int RoomHistory::went( DLString &arg, bool fStrict ) const
{
    const RoomHistoryEntry *h = find( arg, fStrict );
    return (h && h->went < DIR_SOMEWHERE ? h->went : -1);
}

void RoomHistory::toStream( ostringstream &buf ) const
{
    for (const_iterator h = begin( ); h != end( ); h++)
        if (h->went < DIR_SOMEWHERE)
            buf << h->name << " went " << dirs[h->went].name << "." << endl;
        else
            buf << h->name << " went through a portal or an extra exit." << endl;
}

bool RoomHistory::traverse( Room *start, Character *ch ) const
{
    const_iterator h;
    Room *room;

    if (ch->is_npc( ))
        return false;

    for (room = start, h = room->history.begin( ); 
          h != room->history.end( ) && room && ch->in_room != room; 
         h++)
    {
        if (h->name == ch->getPC()->getName( )) {
            if (h->went < DIR_SOMEWHERE && room->exit[h->went]) 
                room = room->exit[h->went]->u1.to_room;
            else
                room = 0;
        }
    }

    return room && ch->in_room == room;
}

