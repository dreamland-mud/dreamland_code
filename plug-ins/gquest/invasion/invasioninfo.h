/* $Id: invasioninfo.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef INVASIONGQUESTINFO_H
#define INVASIONGQUESTINFO_H

#include "xmlvector.h"
#include "xmlmap.h"

#include "globalquestinfo.h"
#include "scenarios.h"

class InvasionGQuest;

class InvasionGQuestInfo : public GQuestInfoEveryone {
XML_OBJECT    
public:
    typedef ::Pointer<InvasionGQuestInfo> Pointer;
    typedef ::Pointer<InvasionGQuest>    InvasionGQuestPointer;
    typedef XMLMapBase<XMLPointer<InvasionScenario> > Scenarios;

    InvasionGQuestInfo( );
    virtual ~InvasionGQuestInfo( );

    virtual GlobalQuestPointer getQuestInstance( ) const;
    virtual int getDefaultTime( ) const;
    virtual const DLString & getQuestID( ) const;

    inline static InvasionGQuestInfo* getThis( ); 

    Scenarios::iterator getRandomScenariosEntry( );
    InvasionScenario * getScenario( const DLString& );
    InvasionScenario * findScenario( const DLString& );
    
    virtual void fromXML( const XMLNode::Pointer& ) ;
    
private:
    XML_VARIABLE Scenarios scenarios;

    static const DLString QUEST_ID;
    static InvasionGQuestInfo *thisClass;
};

inline InvasionGQuestInfo* InvasionGQuestInfo::getThis( ) 
{
    return thisClass;
}

#endif

