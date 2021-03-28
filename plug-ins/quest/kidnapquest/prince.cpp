/* $Id: prince.cpp,v 1.1.2.31.6.5 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2003
 */

#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "dreamland.h"
#include "act.h"
#include "magic.h"
#include "handler.h"
#include "fight.h"

#include "selfrate.h"

#include "questexceptions.h"
#include "kidnapquest.h"
#include "kidnapquestregistrator.h"
#include "objects.h"
#include "scenario.h"
#include "prince.h"
#include "bandit.h"


KidnapPrince::KidnapPrince( ) 
{
    state = STAT_INIT;
}

bool KidnapPrince::spec( ) 
{
    PCharacter *hero;
    NPCharacter *king;
    
    if (!getQuest( ))
        return false;

    switch (state.getValue( )) {
    case STAT_INIT:
    case STAT_FOLLOW:
    case STAT_LOST:
        if (( king = getKingRoom( ) )) {
            kingReunion( king );        
            return true;
        }

        break;
    }
    
    switch (state.getValue( )) {
    case STAT_INIT:
        quest->getScenario( ).actHeroWait( ch );
        break;

    case STAT_FOLLOW:
        if (( hero = getHeroRoom( ) ))
            banditsUnleash( hero );
        else        
            heroDetach( hero );

        break;
        
    case STAT_LOST:
        hero = getHeroRoom( );

        if (hero && !hero->fighting) 
            heroReattach( hero );
        else {
            quest->getScenario( ).actNoHero( ch, hero );
            banditsUnleash( getHeroWorld( ) );
        }

        break;
    case STAT_KIDNAPPED:
        if (!getBanditRoom( ))
            state = STAT_LOST;
            
        break;
    default:
        break;
    }
  
    return true;
}

void KidnapPrince::greet( Character *victim ) 
{
    switch (state.getValue( )) {
    case STAT_INIT:
    case STAT_FOLLOW:
    case STAT_LOST:
        if (ourKing( victim )) {
            kingReunion( victim->getNPC() );
            return;
        }

        break;
    }

    switch (state.getValue( )) {
    case STAT_INIT:
        break;
    case STAT_LOST:
        if (ourHero( victim )) {
            heroReattach( victim->getPC( ) );
            return;
        }

        break;

    default:
        break;
    }
}

void KidnapPrince::entry(  ) 
{
    NPCharacter *king;
    PCharacter *hero;
   
    switch (state.getValue( )) {
    case STAT_INIT:
    case STAT_FOLLOW:
    case STAT_LOST:
        king = getKingRoom( );
        if (king) {
            kingReunion( king );        
            return;
        }

        break;
    }

    switch (state.getValue( )) {
    case STAT_INIT:
        break;
    case STAT_FOLLOW:
        hero = getHeroRoom( );
        if (!hero) {
            heroDetach( hero );
            return;
        }

        break;
        
    case STAT_LOST:
        hero = getHeroRoom( );
        if (hero && !hero->fighting) {
            heroReattach( hero );
            return;
        }

        break;

    default:
        break;
    }
}


void KidnapPrince::give( Character *victim, Object *obj ) 
{
    if (!getQuest( ))
        return;

    if (!ourHero( victim )) {
        quest->getScenario( ).actWrongGiver( ch, victim, obj );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        oldact("$c1 бросает $o4.", ch, obj, 0, TO_ROOM );
        return;
    }
        
    if (state == STAT_INIT) {
        if (quest->check<KidnapMark>( obj )) {
            quest->getScenario( ).actGoodMark( ch, victim, obj );
            heroAttach( victim->getPC( ) );    
        }
        else
            quest->getScenario( ).actWrongMark( ch, victim, obj );
            
        return;
    }
    
    if (obj->item_type == ITEM_POTION || obj->item_type == ITEM_PILL) {
        oldact("$c1 $T $o4.", ch, obj, (obj->item_type == ITEM_PILL ? "съедает" : "осушает"), TO_ROOM );
        spell_by_item( ch, obj );
        extract_obj( obj );
        return;
    }
}

void KidnapPrince::deadAction( Quest::Pointer aquest, PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( ) && getQuest( ))
        quest->getScenario( ).msgKidDeath( ch, killer, pcm->getPlayer( ) );

    ProtectedClient::deadAction( aquest, pcm, killer );
}

void KidnapPrince::config( PCharacter *hero )
{
    int level = hero->getModifyLevel( );

    ch->setLevel( level );
    ch->hit = ch->max_hit = level;
    ch->move = ch->max_move = 100;
}


/* methods for our hero */

void KidnapPrince::heroAttach( PCharacter *hero )
{
    if (!getQuest( ))
        return;

    ch->add_follower( hero );
    
    quest->state.setValue( QSTAT_KID_FOUND );
    quest->wiznet( "", "kid attached" );
        
    state = STAT_FOLLOW;
}

void KidnapPrince::heroDetach( PCharacter *hero ) 
{
    if (!getQuest( ))
        return;

    quest->getScenario( ).actHeroDetach( ch, hero );
    quest->wiznet( "", "kid detached" );

    ch->master = NULL;
    state = STAT_LOST;
}

void KidnapPrince::heroReattach( PCharacter *hero ) 
{
    if (!getQuest( ))
        return;

    ch->add_follower( hero );
    state = STAT_FOLLOW;
    quest->wiznet( "", "kid reattached" );
}

/* methods for our king */

void KidnapPrince::kingReunion( NPCharacter *king )
{
    std::basic_ostringstream<char> buf;
    int time;
    PCMemoryInterface *pci;
    PCharacter *hero;
    
    if (!getQuest( ))
        return;

    state.setValue( STAT_REUNION );
    ch->master = NULL;
    
    if (!( pci = getHeroMemory( ) ))
        return;
   
    hero = pci->getPlayer( );

    switch (quest->state.getValue( )) {
    case QSTAT_INIT:
    case QSTAT_MARK_RCVD:
        time = quest->getAccidentTime( pci );

        if (hero) {
            buf << king->getNameP( '1' ) << " и " << ch->getNameP( '1' ) 
                << " встретились без твоей помощи." << endl
                << "Квест отменен, через {Y" << time 
                << "{x минут ты сможешь получить новое задание." << endl;
            hero->send_to( buf );
        }
        
        quest->setTime( pci, time );
        quest->state = QSTAT_BROKEN_BY_OTHERS;
        quest->scheduleDestroy( );
        break;

    case QSTAT_KID_FOUND:
        if (hero == NULL) {
            quest->setTime( pci, quest->getAccidentTime( pci ) );
            quest->state = QSTAT_BROKEN_BY_OTHERS;
            quest->scheduleDestroy( );
        } 
        else {
            if (hero->in_room == ch->in_room) {
                quest->getScenario( ).actReunion( ch, king, hero );
                quest->state = QSTAT_FINISHED;
            } 
            else {
                quest->getScenario( ).msgRemoteReunion( ch, king, hero );
                quest->state = QSTAT_KING_ACK_WAITING;        
            }

            quest->destroyBandits( );
        }
        
        break;
    }
}

/* methods for bandits */

NPCharacter * KidnapPrince::getBanditRoom( )
{
    if (getQuest( ))
        return quest->getMobileRoom<KidnapBandit>( ch->in_room );
    else
        return NULL;
}

void KidnapPrince::banditsUnleash( PCharacter *hero )
{
    Room *target_room;
    int level;
    int quest_time, invoc_times, probability;
    int number;
    
    if (!hero)
        return;

    if (!getQuest( ))
        return;
    
    if ((rated_as_newbie( hero ) && quest->ambushes >= 1) 
            || quest->ambushes >= 3)
        return;
    
    target_room = ch->in_room;

    if (!quest->checkRoom( hero, target_room ))
        return;

    level = hero->getModifyLevel( );
    quest_time = quest->getTime( hero );
    invoc_times = (quest_time * 60 * dreamland->getPulsePerSecond( )) / PULSE_MOBILE;
    probability = (quest->ambushes > 0 ? 5 : 10);
    
    if (number_range( 1, invoc_times ) > probability)
        return;
    
    number = number_range( 2, 2 + level / 50 );
    
    try {
        for (int i = 0; i < number; i++) {
            NPCharacter *bandit;
            
            bandit = quest->createBandit( );
            char_to_room( bandit, target_room );
            
            if (i == 0)
                quest->getScenario( ).actBanditsUnleash( ch, hero, bandit );
                
            bandit->behavior->spec( );
        }
    }
    catch (const QuestCannotStartException &e) {
        return;
    }
    
    quest->ambushes++;
    quest->wiznet( "unleash", "ambush #%d with %d bandits", 
                    quest->ambushes.getValue( ), number );
}


