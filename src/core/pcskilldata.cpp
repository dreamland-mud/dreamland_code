/* $Id: pcskilldata.cpp,v 1.1.2.4.10.2 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "pcskilldata.h"

#include "logstream.h"
#include "xmlmap.h"

#include "skillmanager.h"
#include "skill.h"

#include "pcharacter.h"

PCSkillData::PCSkillData( )
                    : forgetting( false )
{
}

void PCSkills::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLPCSkills map;
    XMLPCSkills::iterator i;

    map.fromXML( parent );
    
    for (i = map.begin( ); i != map.end( ); i++) {
        int sn = SkillManager::getThis( )->lookup( i->first );
        PCSkillData & data = get( sn );
    
        data = i->second;
    }
}

bool PCSkills::toXML( XMLNode::Pointer& parent ) const
{
    XMLPCSkills map;
    
    for (unsigned int i = 0; i < size( ); i++) {
        
        if ((*this) [i].learned.getValue( ) > 0) {
            DLString skillName( skillManager->find( i )->getName( ) );

            map[skillName] = (*this) [i];
        }
    }
    
    return map.toXML( parent );
}

static PCSkillData emptySkill;

PCSkillData & PCSkills::get( int sn )
{
    if (sn < 0)
        return emptySkill;

    if (sn >= (int) size( ))
        resize( sn + 1 );

    return (*this) [sn];
}

bool PCSkills::forEachLearned( SkillEventHandler::Method method, ... )
{
    for (unsigned int sn = 0; sn < size( ); sn++) { 
        if (at( sn ).learned <= 1)
            continue;

        SkillEventHandlerPointer handler = skillManager->find( sn )->getEventHandler( );
        
        if (!handler)
            continue;

        va_list args;

        va_start( args, method );
        bool rc = (*handler->*method)( args );
        va_end( args );
        
        if (rc)
            return true; 
    }

    return false;
}

