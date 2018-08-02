/* $Id: rainbowinfo.cpp,v 1.1.2.2.6.1 2007/06/26 07:14:30 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "rainbowinfo.h"
#include "rainbow.h"
#include "globalquestmanager.h"

#include "class.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*---------------------------------------------------------------------------
 * Rainbow Global Quest Info
 *-------------------------------------------------------------------------*/
RainbowGQuestInfo* RainbowGQuestInfo::thisClass = NULL;
const DLString RainbowGQuestInfo::QUEST_ID= "rainbow";

RainbowGQuestInfo::RainbowGQuestInfo( ) 
{
    thisClass = this;
    Class::regMoc<RainbowGQuestInfo>( );
    Class::regMoc<RainbowGQuest>( );
}

RainbowGQuestInfo::~RainbowGQuestInfo( )
{
    Class::unregMoc<RainbowGQuest>( );
    Class::unregMoc<RainbowGQuestInfo>( );
    thisClass = NULL;
}
   
GlobalQuest::Pointer RainbowGQuestInfo::getQuestInstance( ) const
{
    return RainbowGQuest::Pointer( NEW, QUEST_ID );
}

int RainbowGQuestInfo::getDefaultTime( ) const
{
    return number_range( 50, 60 );
}

const DLString & RainbowGQuestInfo::getQuestID( ) const 
{
    return QUEST_ID;
}

RainbowGQuestInfo::Scenarios::iterator
RainbowGQuestInfo::getRandomScenariosEntry( )
{
    Scenarios::iterator i;
    int n;
    
    if (scenarios.empty( ))
        throw GQCannotStartException( "no scenario found" );

    n = number_range( 0, scenarios.size( ) - 1 );
    for (i = scenarios.begin( ); n > 0 && i != scenarios.end( ); i++, n--)
        ;

    return i;
}

RainbowScenario *
RainbowGQuestInfo::getScenario( const DLString& key )
{
    Scenarios::iterator i = scenarios.find( key );
    
    if (i == scenarios.end( ))
        throw GQRuntimeException( "scenario not found" );

    return i->second.getPointer( );
}

void RainbowGQuestInfo::fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
{
    scenarios.clear( );
    GlobalQuestInfo::fromXML( node );
}

RainbowScenario *
RainbowGQuestInfo::findScenario( const DLString& key )
{
    Scenarios::iterator i = scenarios.find( key );
    
    if (i == scenarios.end( ))
        return NULL;

    return i->second.getPointer( );
}

const DLString & RainbowGQuestInfo::getQuestName( ) const
{
    GlobalQuest::Pointer gq = GlobalQuestManager::getThis( )->findGlobalQuest( getQuestID( ) );
    RainbowGQuest::Pointer rgq = gq.getDynamicPointer<RainbowGQuest>( );
    if (rgq) 
        return rgq->getDisplayName( );
    
    return GlobalQuestInfo::getQuestName( );
}        
