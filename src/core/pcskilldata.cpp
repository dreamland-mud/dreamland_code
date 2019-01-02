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

PCSkillData PCSkillData::empty;

PCSkillData::PCSkillData( )
                    : forgetting( false ), temporary( false )
{
}

bool PCSkillData::isValid() const
{
    return learned > 0;
}

PCSkills::PCSkills() 
            : Base(skillManager)
{
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

