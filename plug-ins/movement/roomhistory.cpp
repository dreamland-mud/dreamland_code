/* $Id$
 *
 * ruffina, 2004
 */
#include "directions.h"

#include "room.h"
#include "pcharacter.h"

#include "merc.h"

#include "def.h"

/*
 * RoomHistory 
 */
const unsigned int RoomHistory::MAX_SIZE = 10;

void RoomHistory::record( Character *ch, int door )
{
    if (ch->is_npc( ) || door >= DIR_SOMEWHERE)
        return;

    erase( );
    push_front( RoomHistoryEntry( 
        ch->getName( ), ch->getPC()->getRussianName().getFullForm(), door ) );
}

void RoomHistory::erase( )
{
    if (size( ) > MAX_SIZE)
        pop_back( );
}

int RoomHistory::went( Character *ch ) const
{
    if (ch->is_npc( ))
        return -1;
    else {
        DLString arg( ch->getName( ) );

        return went( arg, true );
    }
}

int RoomHistory::went( DLString &arg, bool fStrict ) const
{
    bool rus = arg.isRussian();

    for (const_iterator h = begin( ); h != end( ); h++) {
        DLString name = rus ? h->rname.ruscase('1') : h->name;

        if ((fStrict && name == arg)
            || is_name( arg.c_str( ), name.c_str( ) ))
        {
            arg = rus ? h->rname : h->name;
            return h->went;
        }
    }

    return -1;
}

void RoomHistory::toStream( ostringstream &buf ) const
{
    for (const_iterator h = begin( ); h != end( ); h++)
        buf << h->name << " went " << dirs[h->went].name << "." << endl;
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
        if (h->name == ch->getName( )) {
            if (room->exit[h->went]) 
                room = room->exit[h->went]->u1.to_room;
            else
                room = 0;
        }
    }

    return room && ch->in_room == room;
}

