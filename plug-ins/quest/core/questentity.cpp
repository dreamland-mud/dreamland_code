/* $Id: questentity.cpp,v 1.1.6.4.6.3 2008/05/16 00:30:33 rufina Exp $
 *
 * ruffina, 2005
 */

#include "questentity.h"
#include "quest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"

#include "mercdb.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

void QuestEntity::mandatoryExtract( )
{
    PCMemoryInterface *pcm;
    Quest::Pointer quest;
    
    if (!( pcm = getHeroMemory( ) ))
	return;

    if (!( quest = getQuest( pcm ) ))
	return;
    
    if (quest->isComplete( ))
	return;

    if (quest->state != QSTAT_BROKEN_BY_HERO
	&& quest->state != QSTAT_BROKEN_BY_OTHERS)
    {
	quest->state = QSTAT_BROKEN_BY_OTHERS;
	quest->setTime( pcm, quest->getAccidentTime( pcm ) );
	quest->scheduleDestroy( );
    }
}

bool QuestEntity::ourHero( Character *ch ) 
{
    return ch != NULL
           && !ch->is_npc( ) 
	   && heroName == ch->getName( );
}

bool QuestEntity::ourMobile( NPCharacter *mob )
{
    QuestEntity::Pointer entity;
    
    return mob 
	   && mob->behavior
           && ( entity = mob->behavior.getDynamicPointer<QuestEntity>( ) )
	   && entity->getHeroName( ) == getHeroName( );
}

bool QuestEntity::ourObject( Object *obj )
{
    QuestEntity::Pointer entity;
    
    return obj->behavior
           && ( entity = obj->behavior.getDynamicPointer<QuestEntity>( ) )
	   && entity->getHeroName( ) == getHeroName( );
}

PCMemoryInterface * QuestEntity::getHeroMemory( )
{
    return PCharacterManager::find( heroName );
}

Quest::Pointer QuestEntity::getQuest( )
{
    return getQuest( getHeroMemory( ) );
}

Quest::Pointer QuestEntity::getQuest( PCMemoryInterface *hero )
{
    Quest::Pointer quest, null;
    
    if (!hero)
	return null;

    if (!( quest = hero->getAttributes( ).findAttr<Quest>( "quest" ) ))
	return null;

    if (quest->charName != heroName)
	return null;

    return quest;
}

PCharacter * QuestEntity::getHeroWorld( )
{
    PCMemoryInterface *pcm = getHeroMemory( );
    
    if (pcm 
	&& pcm->isOnline( ) 
	&& pcm->getPlayer( )->desc
	&& pcm->getPlayer( )->desc->connected == CON_PLAYING)
    {
	return pcm->getPlayer( );
    }

    return NULL;
}

