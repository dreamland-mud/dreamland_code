/* $Id: pcskilldata.h,v 1.1.2.3.18.2 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __PCSKILLDATA_H__
#define __PCSKILLDATA_H__

#include <vector>

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlmap.h"
#include "skilleventhandler.h"

class PCSkillData : public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<PCSkillData> Pointer;
        
        PCSkillData( );

        XML_VARIABLE XMLInteger learned;
        XML_VARIABLE XMLInteger timer;
        XML_VARIABLE XMLBoolean forgetting;
};

class PCSkills : public XMLVariableContainer, public std::vector<PCSkillData> {
XML_OBJECT
public:
        typedef ::Pointer<PCSkills> Pointer;

        virtual bool toXML( XMLNode::Pointer& ) const;
        virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );

        PCSkillData & get( int );
        bool forEachLearned( SkillEventHandler::Method, ... );
};

typedef XMLMapBase<PCSkillData> XMLPCSkills;

#define SKILLEVENT_CALL(ch, method...) \
    if (!(ch)->is_npc( )) \
        if ((ch)->getPC( )->getSkills( ).forEachLearned( &SkillEventHandler::method )) \
            return true;

#endif
