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

