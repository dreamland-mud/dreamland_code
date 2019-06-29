/* $Id: mobiles.cpp,v 1.1.2.11.6.3 2008/03/06 17:48:35 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobiles.h"
#include "objects.h"
#include "scenarios.h"
#include "locatequest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "interp.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"


void LocateCustomer::talkToHero( PCharacter *hero )
{
    if (!getQuest( ))
        return;
    
    if (ch->position == POS_SLEEPING)
        interpret_raw( ch, "wake" );

    switch (quest->state.getValue( )) {
    case QSTAT_INIT:
        quest->state = QSTAT_SEARCH;
        quest->setTime( hero, number_range( 20, 30 ) );

        quest->getScenario( ).actTellStory( ch, hero, *quest );
        quest->wiznet( "", "%s tells story", ch->getNameP( '1' ).c_str() );
        break;
        
    case QSTAT_SEARCH:
        break;
    }
}

bool LocateCustomer::givenCheck( PCharacter *hero, Object *obj )
{
    return getQuest( ) && quest->check<LocateItem>( obj );
}

void LocateCustomer::givenGood( PCharacter *hero, Object *obj )
{
    quest->delivered++;

    if (quest->delivered >= quest->total) {
        quest->state = QSTAT_FINISHED;
        quest->getScenario( ).actLastItem( ch, hero, quest );
        quest->wiznet( "", "%s gets last item", ch->getNameP( '1' ).c_str( ) );
    }
    else {
        quest->getScenario( ).actAnotherItem( ch, hero, quest );
//        quest->wiznet( "", "%s gets item #%d", ch->getNameP( '1' ).c_str( ), quest->delivered.getValue( ) );
    }
}

void LocateCustomer::givenBad( PCharacter *hero, Object *obj )
{
    if (getQuest( ))
        quest->getScenario( ).actWrongItem( ch, hero, quest, obj );
}

void LocateCustomer::deadFromIdiot( PCMemoryInterface *pcm )
{
    act("{YТы прине$gсло|с|сла $C3 смерть, а тебя просили принести кое-что другое.{x", pcm->getPlayer( ), 0, ch, TO_CHAR);
}

void LocateCustomer::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) 
        act_p("{Y$c1 внезапно скончал$gось|ся|ась. Задание отменяется.{x", ch, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void LocateCustomer::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
        act_p("{Y$c1 подло уби$gло|л|ла того, кто нуждался в твоей помощи.{x", killer, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void LocateCustomer::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ) && getQuest( ) && !quest->isComplete( )) 
        buf << "{x({YЖдет кого-то{x) ";
}

