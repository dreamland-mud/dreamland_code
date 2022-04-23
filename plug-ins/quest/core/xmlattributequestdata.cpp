/* $Id: xmlattributequestdata.cpp,v 1.1.4.6.6.5 2009/03/16 20:35:53 rufina Exp $
 *
 * ruffina, 2003
 */
#include "xmlattributequestdata.h"
#include "quest.h"

#include "date.h"
#include "pcharacter.h"
#include "room.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "wiznet.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

bool XMLAttributeQuestData::handle( const DeathArguments &args ) 
{
    XMLAttributes *attributes = &(args.pch->getAttributes( ));
    Quest::Pointer quest = attributes->findAttr<Quest>( "quest" );
    
    if (quest) {
        quest->scheduleDestroy( );
        setTime( quest->getDeathTime( args.pch ) );
    }

    return false;
}

bool XMLAttributeQuestData::pull( PCMemoryInterface *pcm ) 
{
    if (pcm->isOnline())
        return false;

    if (!takesTooLong())
        return false;

    XMLAttributes *attributes = &pcm->getAttributes( );
    Quest::Pointer quest = attributes->findAttr<Quest>( "quest" );
    if (!quest)
        return false;

    quest->wiznet( "expired", "started at %lld", startTime.getValue());

    attributes->eraseAttribute( "quest" );
    setTime(1);
    PCharacterManager::saveMemory(pcm);
    return false;
}

bool XMLAttributeQuestData::pull( PCharacter *pch ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributes *attributes = &pch->getAttributes( );
    Quest::Pointer quest = attributes->findAttr<Quest>( "quest" );
    int time = getTime( );
    
    if (time == 0)
        return false;
    if (!quest && attributes->isAvailable( "quest" ))
        return false;

    setTime( --time );

    if (quest) {

        if (time == 0) {
            pch->pecho("Время, отведенное на задание, вышло!");
            attributes->eraseAttribute( "quest" );
            
            time = quest->getFailTime( pch );
            setTime( time );

            buf << "Через " << time << " минут" << GET_COUNT( time, "у", "ы", "")
                << " ты можешь снова получить задание." << endl;
            pch->send_to( buf );

        } else if (time < 6) {
            pch->pecho("Поторопись! Время, отведенное на выполнение задания, заканчивается!");
        }
        
    } else if (time == 0) {
        pch->pecho("Теперь ты снова можешь взять задание.");
    }
        
    PCharacterManager::save(pch);
    return false;
}

bool XMLAttributeQuestData::handle( const RemortArguments &args ) 
{
    setTime( 0 );
    lastQuestCount = 0;
    lastQuestType.clear();
    return XMLAttributeStatistic::handle( args );
}

bool XMLAttributeQuestData::handle( const PromptArguments &args ) 
{
    if (args.letter == 't') {
        args.buffer << getTime( );
        return true;
    }
    else
        return false;
}

int XMLAttributeQuestData::getTime( ) const
{
    return countdown.getValue( );
}

void XMLAttributeQuestData::setTime( int value )
{
    countdown.setValue( value );
}

void XMLAttributeQuestData::rememberLastQuest(const DLString &questType)
{
    if (lastQuestType != questType) {
        lastQuestCount = 1;
        lastQuestType = questType;
        return;
    }

    lastQuestCount++;
}

int XMLAttributeQuestData::getLastQuestCount(const DLString &questType) const
{
    if (lastQuestType != questType)
        return 0;

    return lastQuestCount;
}

void XMLAttributeQuestData::setStartTime()
{
    startTime = dreamland->getCurrentTime();
}

bool XMLAttributeQuestData::takesTooLong() const
{
    if (startTime.getValue() <= 0)
        return false;

    long long interval = dreamland->getCurrentTime() - startTime.getValue();
    return interval >= Date::SECOND_IN_WEEK;
}

