/* $Id$
 *
 * ruffina, 2004
 */
#include "roomchannel.h"
#include "replay.h"

#include "room.h"
#include "character.h"
#include "act.h"

/*-----------------------------------------------------------------------
 * RoomChannel
 *-----------------------------------------------------------------------*/
RoomChannel::RoomChannel( )
{
}

void RoomChannel::findListeners( Character *ch, Listeners &listeners ) const
{
    Character *rch;
    
    for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
        if (isGlobalListener( ch, rch ))
            listeners.push_back( rch );
}

void RoomChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (outputTo->getPC( ))
        remember_history_near( outputTo->getPC( ), message );
}

