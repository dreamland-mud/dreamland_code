/* $Id: invasioninfo.cpp,v 1.1.2.2.6.1 2007/06/26 07:14:29 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "invasioninfo.h"
#include "invasion.h"

#include "class.h"

#include "room.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

InvasionGQuestInfo* InvasionGQuestInfo::thisClass = NULL;
const DLString InvasionGQuestInfo::QUEST_ID= "invasion";

InvasionGQuestInfo::InvasionGQuestInfo( ) 
{
    thisClass = this;
    Class::regMoc<InvasionGQuestInfo>( );
    Class::regMoc<InvasionGQuest>( );
}

InvasionGQuestInfo::~InvasionGQuestInfo( )
{
    Class::unregMoc<InvasionGQuest>( );
    Class::unregMoc<InvasionGQuestInfo>( );
    thisClass = NULL;
}
   
GlobalQuest::Pointer InvasionGQuestInfo::getQuestInstance( ) const
{
    return InvasionGQuest::Pointer( NEW, QUEST_ID );
}

int InvasionGQuestInfo::getDefaultTime( ) const
{
    return number_range( 35, 55 );
}

const DLString & InvasionGQuestInfo::getQuestID( ) const 
{
    return QUEST_ID;
}

InvasionGQuestInfo::Scenarios::iterator
InvasionGQuestInfo::getRandomScenariosEntry( )
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

InvasionScenario *
InvasionGQuestInfo::getScenario( const DLString& key )
{
    Scenarios::iterator i = scenarios.find( key );
    
    if (i == scenarios.end( ))
        throw GQRuntimeException( "scenario not found" );

    return i->second.getPointer( );
}

InvasionScenario *
InvasionGQuestInfo::findScenario( const DLString& key )
{
    Scenarios::iterator i = scenarios.find( key );
    
    if (i == scenarios.end( ))
        return NULL;

    return i->second.getPointer( );
}

void InvasionGQuestInfo::fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
{
    scenarios.clear( );
    GlobalQuestInfo::fromXML( node );
}

