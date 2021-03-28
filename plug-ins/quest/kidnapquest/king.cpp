/* $Id: king.cpp,v 1.1.2.11.6.4 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2003
 */

#include "king.h"
#include "kidnapquest.h"
#include "scenario.h"
#include "kidnapquestregistrator.h"
#include "objects.h"
#include "questexceptions.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"

#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

void KidnapKing::deadAction( Quest::Pointer aquest, PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( ) && getQuest( )) 
        quest->getScenario( ).msgKingDeath( ch, killer, pcm->getPlayer( ) );

    ProtectedClient::deadAction( aquest, pcm, killer );
}

void KidnapKing::talkToHero( PCharacter *hero )
{
    Object *mark;
    
    getQuest( );

    switch (quest->state.getValue( )) {
    case QSTAT_INIT:
        quest->getScenario( ).actLegend( ch, hero, quest );
        mark = giveMarkHero( hero );

        if (mark) {
            int time = number_range( 20, 30 );
            
            quest->state.setValue( QSTAT_MARK_RCVD );
            quest->setTime( hero, time );            
            quest->getScenario( ).actGiveMark( ch, hero, mark, time );
            quest->wiznet( "", "%s gives mark", ch->getNameP( '1' ).c_str() );
        }

        break;
        
    case QSTAT_MARK_RCVD:
        if (!quest->getItemWorld<KidnapMark>( )) {
            mark = giveMarkHero( hero );
            quest->getScenario( ).actMarkLost( ch, hero, mark );
        }
        
        break;
        
    case QSTAT_KING_ACK_WAITING: 
        quest->getScenario( ).actAckWaitComplete( ch, hero );
        quest->state = QSTAT_FINISHED;
        break;
    
    default:
        break;
    }
}

Object * KidnapKing::giveMarkHero( PCharacter *hero )
{
    Object *mark;
    
    getQuest( );

    try {
        mark = quest->createMark( );
        
    } catch (const QuestCannotStartException &e) {
        oldact("Глюк! $C4 нечего тебе дать.", hero, 0, ch, TO_CHAR );
        hero->pecho("Задание отменено. Через минуту сможешь получить новое.");
        LogStream::sendError( ) << e.what( ) << endl;
        
        quest->setTime( hero, 1 );
        quest->state = QSTAT_BROKEN_BY_OTHERS;
        quest->scheduleDestroy( );
        return NULL;
    }

    obj_to_char( mark, hero );
    return mark;
}

