/* $Id: rainbowinfo.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef RAINBOWGQUESTINFO_H
#define RAINBOWGQUESTINFO_H

#include "xmlvector.h"
#include "xmlmap.h"
#include "xmlstring.h"

#include "globalquestinfo.h"
#include "scenarios.h"

class RainbowGQuest;

class RainbowGQuestInfo : public GQuestInfoEveryone {
XML_OBJECT    
public:
    typedef ::Pointer<RainbowGQuestInfo> Pointer;
    typedef XMLMapBase<XMLPointer<RainbowScenario> > Scenarios;

    RainbowGQuestInfo( );
    virtual ~RainbowGQuestInfo( );

    virtual GlobalQuestPointer getQuestInstance( ) const;
    virtual int getDefaultTime( ) const;
    virtual const DLString & getQuestID( ) const;
    virtual const DLString & getQuestName( ) const;  

    virtual void fromXML( const XMLNode::Pointer& ) ;

    inline static RainbowGQuestInfo* getThis( ); 

    Scenarios::iterator getRandomScenariosEntry( );
    RainbowScenario * getScenario( const DLString& );
    RainbowScenario * findScenario( const DLString& );

private:
    XML_VARIABLE Scenarios  scenarios;

    static const DLString QUEST_ID;
    static RainbowGQuestInfo *thisClass;
};


inline RainbowGQuestInfo* RainbowGQuestInfo::getThis( ) 
{
    return thisClass;
}

#endif

