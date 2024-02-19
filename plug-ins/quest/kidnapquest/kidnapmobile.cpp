/* $Id: kidnapmobile.cpp,v 1.1.2.10.6.2 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "kidnapmobile.h"
#include "kidnapquestregistrator.h"
#include "kidnapquest.h"
#include "king.h"

#include "class.h"

#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "interp.h"
#include "merc.h"

#include "def.h"


bool KidnapMobile::ourKing( Character *king )
{
    return getQuest( ) && quest->check<KidnapKing>( king );
}

NPCharacter * KidnapMobile::getKingRoom( )
{
    return getKingRoom( ch->in_room );
}

NPCharacter * KidnapMobile::getKingRoom( Room *room )
{
    Character *king;
    
    for (king = room->people; king; king = king->next_in_room)
        if (ourKing( king ))
            return king->getNPC( );
    
    return NULL;
}

Character * KidnapMobile::getAggrRoom( Room *room )
{
    Character *wch;

    for (wch = room->people; wch; wch = wch->next_in_room) {
        if (!wch->is_npc( ))
            continue;
            
        if (IS_AFFECTED( wch, AFF_BLOODTHIRST ))
            break;
        
        if (!IS_AWAKE( wch ))
            continue;
        
        if (wch->last_fought)
            continue;
        
        if (wch->getModifyLevel( ) < ch->getModifyLevel( ) + 10)
            continue;
        
        if (IS_SET( wch->act, ACT_AGGRESSIVE ))
            break;
    }

    return wch;
}

void KidnapMobile::debug( const DLString &msg )
{
    if (getQuest( ) && quest->debug) 
        interpret_raw( ch, "say", msg.c_str( ) );
}

