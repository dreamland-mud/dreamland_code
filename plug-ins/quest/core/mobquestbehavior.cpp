/* $Id: mobquestbehavior.cpp,v 1.1.4.3.6.4 2008/03/26 14:51:04 rufina Exp $
 *
 * ruffina, 2003
 */

#include "mobquestbehavior.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "handler.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------
 * MobQuestBehavior
 *--------------------------------------------------------------------*/
MobQuestBehavior::MobQuestBehavior( ) 
                    : imm_flags( 0, &::imm_flags ), act_flags( 0, &::act_flags )
{
}

void MobQuestBehavior::setChar( NPCharacter *mob ) 
{
    BasicMobileDestiny::setChar( mob );
    
    imm_flags.setValue( ch->imm_flags );
    act_flags.setValue( ch->act );
    SET_BIT( ch->imm_flags, IMM_SUMMON | IMM_CHARM );
    SET_BIT( ch->act, ACT_NOEYE );
}

void MobQuestBehavior::unsetChar( ) 
{
    if (!act_flags.isSet( ACT_NOEYE ))
        REMOVE_BIT( ch->act, ACT_NOEYE );
        
    if (!imm_flags.isSet( IMM_SUMMON ))
        REMOVE_BIT( ch->imm_flags, IMM_SUMMON );

    if (!imm_flags.isSet( IMM_CHARM ))
        REMOVE_BIT( ch->imm_flags, IMM_CHARM );

    BasicMobileDestiny::unsetChar( );
}

bool MobQuestBehavior::isHomesick( )
{
    return false;
}

PCharacter * MobQuestBehavior::getHeroRoom(  )
{
    PCharacter *hero = getHeroWorld( );

    if (hero && hero->in_room == ch->in_room)
        return hero;
    else
        return NULL;
}

/*--------------------------------------------------------------------
 * MandatoryMobile 
 *--------------------------------------------------------------------*/
bool MandatoryMobile::extract( bool count )
{
    mandatoryExtract( );
    return MobQuestBehavior::extract( count );
}

/*--------------------------------------------------------------------
 * ConfiguredMobile 
 *--------------------------------------------------------------------*/
void ConfiguredMobile::setChar( NPCharacter *mob ) 
{
    MobQuestBehavior::setChar( mob );

    if (ch->getLevel( ) == 0) {
        PCMemoryInterface *pcm = getHeroMemory( );

        if (pcm && pcm->isOnline( ))
            config( pcm->getPlayer( ) );
    }
}

/*--------------------------------------------------------------------
 * TalkativeClient 
 *--------------------------------------------------------------------*/
bool TalkativeClient::specIdle( ) 
{ 
    PCharacter *hero;

    if (( hero = getHeroRoom( ) )) {
        talkToHero( hero );
        return true;
    }

    return false; 
}

void TalkativeClient::greet( Character *victim ) 
{
    if (ourHero( victim ))
        talkToHero( victim->getPC( ) );
}

void TalkativeClient::entry( ) 
{
    PCharacter *hero;

    if (( hero = getHeroRoom( ) ))
        talkToHero( hero );
}

/*--------------------------------------------------------------------
 * PeacefulClient 
 *--------------------------------------------------------------------*/
bool PeacefulClient::canAggressNormal( Character *victim )
{
    return !ourHero( victim ) 
            && MobQuestBehavior::canAggressNormal( victim );
}

bool PeacefulClient::canAggressVampire( Character *victim )
{
    return !ourHero( victim )
            && MobQuestBehavior::canAggressVampire( victim );
}

/*--------------------------------------------------------------------
 * ProtectedClient*
 *--------------------------------------------------------------------*/
void ProtectedClient::deadAction( Quest::Pointer quest, PCMemoryInterface *pcm, Character *killer )
{
    if (ourHero( killer )) 
        deadFromIdiot( pcm );
    else if (killer && killer->getNPC( ) == ch) 
        deadFromSuicide( pcm );
    else
        deadFromKill( pcm, killer ); 
}

void ProtectedClient::deadFromIdiot( PCMemoryInterface *pcm )
{
}

void ProtectedClient::deadFromSuicide( PCMemoryInterface *pcm )
{
}

void ProtectedClient::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
}

bool ProtectedClient::death( Character *killer )
{
    PCMemoryInterface *pcm;
    Quest::Pointer quest;

    if (!( pcm = getHeroMemory( ) ))
        return false;

    if (!( quest = getQuest( pcm ) ))
        return false;
    
    if (quest->isComplete( ))
        return false;

    killer = quest->getActor( killer );

    if (ourHero( killer )) {
        quest->setTime( pcm, quest->getPunishTime( pcm ) );
        quest->state = QSTAT_BROKEN_BY_HERO;
    }
    else {
        quest->setTime( pcm, quest->getAccidentTime( pcm ) );
        quest->state = QSTAT_BROKEN_BY_OTHERS;
    }
    
    quest->scheduleDestroy( );
    deadAction( quest, pcm, killer );
    return false;
}

/*--------------------------------------------------------------------
 * HuntedVictim
 *--------------------------------------------------------------------*/
void HuntedVictim::deadFromHunter( PCMemoryInterface *pcm )
{
}

void HuntedVictim::deadFromSuicide( PCMemoryInterface *pcm )
{
}

void HuntedVictim::deadFromOther( PCMemoryInterface *pcm, Character *killer )
{
}

void HuntedVictim::deadFromGroupMember( PCMemoryInterface *pcm, Character *killer )
{
}

bool HuntedVictim::death( Character *killer )
{
    PCMemoryInterface *pcm;
    Quest::Pointer quest;

    if (!( pcm = getHeroMemory( ) ))
        return false;

    if (!( quest = getQuest( pcm ) ))
        return false;
    
    killer = quest->getActor( killer );

    if (ourHero( killer )) {
        quest->state = QSTAT_FINISHED;
        deadFromHunter( pcm );
        return false;
    }

    if (ourHeroGroup(killer)) {
        quest->state = QSTAT_FINISHED;
        deadFromGroupMember( pcm, killer );
        return false;
    }

    if (killer && killer->getNPC( ) != ch)
        deadFromOther( pcm, killer );
    else
        deadFromSuicide( pcm );
    
    quest->state = QSTAT_BROKEN_BY_OTHERS;
    quest->setTime( pcm, quest->getAccidentTime( pcm ) );
    quest->scheduleDestroy( );
    return false;
}

/*--------------------------------------------------------------------
 * GreedyClient 
 *--------------------------------------------------------------------*/
void GreedyClient::give( Character *victim, Object *obj ) 
{
    if (!ourHero( victim ))
        return;
    
    if (!givenCheck( victim->getPC( ), obj )) {
        obj_from_char( obj );
        obj_to_char( obj, victim );

        givenBad( victim->getPC( ), obj );
    }
    else
        givenGood( victim->getPC( ), obj );
}
