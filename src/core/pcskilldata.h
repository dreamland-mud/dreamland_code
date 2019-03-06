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
#include "xmllong.h"
#include "xmlmap.h"
#include "skilleventhandler.h"
#include "globalprofilearray.h"

class PCSkillData : public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<PCSkillData> Pointer;
        
        PCSkillData( );

        XML_VARIABLE XMLInteger learned;
        XML_VARIABLE XMLIntegerNoEmpty timer;
        XML_VARIABLE XMLBooleanNoFalse forgetting;
        XML_VARIABLE XMLBooleanNoFalse temporary;
        XML_VARIABLE XMLLongNoEmpty start;
        XML_VARIABLE XMLLongNoEmpty end;
    
        bool isValid() const;
        static PCSkillData empty;
};

class PCSkills : public GlobalProfileArray<PCSkillData> {
XML_OBJECT
public:
        typedef ::Pointer<PCSkills> Pointer;
        typedef GlobalProfileArray<PCSkillData> Base;

        PCSkills();
        bool forEachLearned( SkillEventHandler::Method, ... );
};

#define SKILLEVENT_CALL(ch, method...) \
    if (!(ch)->is_npc( )) \
        if ((ch)->getPC( )->getSkills( ).forEachLearned( &SkillEventHandler::method )) \
            return true;

#endif
