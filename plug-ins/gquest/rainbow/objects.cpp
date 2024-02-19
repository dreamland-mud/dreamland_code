/* $Id: objects.cpp,v 1.1.2.2 2005/09/11 22:50:49 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objects.h"
#include "rainbow.h"
#include "rainbowinfo.h"

#include "class.h"

#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act.h"
#include "merc.h"
#include "def.h"

/* RainbowPiece */

void RainbowPiece::get( Character *ch )
{
    RainbowGQuest *gq = RainbowGQuest::getThis( );

    if (!gq || gq->isHidden( ) || gq->state == RainbowGQuest::ST_FINISHED)
        return;
    
    if (ch->is_npc( ))
        return;
    
    log("RainbowPiece: [" << number << "] by " << ch->getName( ));

    if (gq->countPieces( ch ) == gq->getScenario( )->getPiecesCount( )) {
        gq->getScenario( )->onQuestFinish( ch->getPC( ) );
        gq->winnerName = ch->getName( );
        gq->state = RainbowGQuest::ST_FINISHED;
        gq->scheduleDestroy( );
    }
}

void RainbowPiece::config( int number )
{
    RainbowGQuest *gq = RainbowGQuest::getThis( );
    
    this->number = number;
    gq->getScenario( )->dressItem( obj, number );
}

