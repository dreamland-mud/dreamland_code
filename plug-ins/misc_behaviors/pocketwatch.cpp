/* $Id: pocketwatch.cpp,v 1.1.2.4 2005/09/22 21:00:57 rufina Exp $
 *
 * ruffina, 2004
 */

#include "pocketwatch.h"
#include "class.h"

#include "character.h"
#include "object.h"
#include "room.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

PocketWatch::PocketWatch( ) : broken( false )
{
    prevHour = number_range( 0, 23 );
}

bool PocketWatch::prompt( Character *ch, char letter, ostringstream &buf )
{
    int hour;

    if (letter != 'T')
	return false;

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_TIME))
	return false;
    
    if (broken.getValue( )) {
	hour = prevHour;

	if (number_range( 1, 7777) == 1)
	    prevHour = number_range( 0, 23 );
    }
    else
	hour = time_info.hour;

    buf << ((hour % 12 == 0) ? 12 : hour % 12) << " ";

    if ((hour > 16) && (hour < 24)) 
	buf << "вечера";
    if (hour < 4) 
	buf << "ночи";
    if ((hour > 3) && (hour < 12)) 
	buf << "утра";
    if ((hour > 11) && (hour < 17)) 
	buf << "дня";

    return true;
}

