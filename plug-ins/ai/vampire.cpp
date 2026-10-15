/* $Id: vampire.cpp,v 1.1.2.3.6.7 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
#include "basicmobilebehavior.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "affect.h"
#include "skillreference.h"

#include "dreamland.h"
#include "loadsave.h"
#include "fight.h"

#include "magic.h"
#include "interp.h"
#include "merc.h"

#include "def.h"

/*----------------------------------------------------------------------------
 *                         VAMPIRIC BRAIN
 *----------------------------------------------------------------------------*/
// Anyone the Fenia vampire brain (onVampireAI) may go for: a player or a pet,
// out of a fight. A cheap gate, aggr_update runs every pulse.
bool BasicMobileBehavior::hasVampirePrey( )
{
    for (Character *wch = ch->in_room->people; wch; wch = wch->next_in_room) {
        if (wch == ch || wch->fighting || wch->position == POS_FIGHTING)
            continue;
        if (wch->is_npc( ) && !IS_CHARMED(wch))
            continue;
        if (!wch->is_npc( ) && wch->is_immortal( )
            && !wch->getPC( )->getAttributes( ).isAvailable( "ai_aggress" )
            && !memoryFought.memorized( wch ) && !isLastFought( wch ))
            continue;
        return true;
    }
    return false;
}
