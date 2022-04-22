/* $Id$
 *
 * ruffina, 2004
 */
#include "worldchannel.h"
#include "replay.h"
#include "character.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

/*-----------------------------------------------------------------------
 * WorldChannel
 *-----------------------------------------------------------------------*/
WorldChannel::WorldChannel( )
{
}

void WorldChannel::findListeners( Character *ch, Listeners &listeners ) const
{
    for (Character *wch = char_list; wch; wch = wch->next) {
        if (!wch->getPC( ))
            continue;

        if (!isGlobalListener( ch, wch ))
            continue;

        if (wch->desc && wch->desc->connected != CON_PLAYING)
            continue;

        listeners.push_back( wch );
    }

}

void WorldChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (!outputTo->is_npc())
        remember_history_public( outputTo->getPC( ), message );
}

