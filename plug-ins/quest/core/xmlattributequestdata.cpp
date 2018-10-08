/* $Id: xmlattributequestdata.cpp,v 1.1.4.6.6.5 2009/03/16 20:35:53 rufina Exp $
 *
 * ruffina, 2003
 */
#include "xmlattributequestdata.h"
#include "quest.h"

#include "pcharacter.h"
#include "mercdb.h"
#include "merc.h"

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
            pch->send_to( "Время, отведенное на задание, вышло!\r\n" );
            attributes->eraseAttribute( "quest" );
            
            time = quest->getFailTime( pch );
            setTime( time );

            buf << "Через " << time << " минут" << GET_COUNT( time, "у", "ы", "")
                << " ты можешь снова получить задание." << endl;
            pch->send_to( buf );

        } else if (time < 6) {
            pch->send_to( "Поторопись! Время, отведенное на выполнение задания, заканчивается!\r\n" );
        }
        
    } else if (time == 0) {
        pch->send_to( "Теперь ты снова можешь взять задание.\r\n" );
    }
        
    return false;
}

bool XMLAttributeQuestData::handle( const RemortArguments &args ) 
{
    setTime( 0 );
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

