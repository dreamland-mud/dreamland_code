/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
#include "room.h"
#include "merc.h"
#include "def.h"

void Room::echo( int pos, const char *f, ... ) const
{
    va_list av;

    va_start( av, f );
    vecho( pos, f, av );
    va_end( av );
}

void Room::vecho( int pos, const char *f, va_list av ) const
{
    for (Character *rch = people; rch; rch = rch->next_in_room)
	if (rch->position >= pos)
	    rch->vpecho( f, av );
}

void Room::echoAround( int pos, const char *f, ... ) const
{
    map<int, bool> roomMarks;
    va_list av;

    va_start( av, f );
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;
	Room *room;

        if (!( pexit = exit[door] ))
	    continue;
	    
        if (!( room = pexit->u1.to_room ))
	    continue;
	
	if (room == this)
	    continue;

	if (roomMarks[room->vnum] == true)
	    continue;
	    
	room->vecho( pos, f, av );
	roomMarks[room->vnum] = true;
    }

    va_end( av );
}

