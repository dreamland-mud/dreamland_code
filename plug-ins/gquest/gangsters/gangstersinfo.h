/* $Id: gangstersinfo.h,v 1.1.2.1.6.3 2008/11/13 03:33:28 rufina Exp $
 * 
 * ruffina, 2003
 */
#ifndef GANGSTERSINFO_H
#define GANGSTERSINFO_H

#include "globalquestinfo.h"

class Gangsters;

class GangstersInfo : public GQuestInfoLevels {
friend class Gangsters;
friend class GangMember;
XML_OBJECT    

public:
    typedef ::Pointer<GangstersInfo> Pointer;
    typedef ::Pointer<Gangsters>    GangstersPointer;

    GangstersInfo( );
    virtual ~GangstersInfo( );

    virtual bool canAutoStart( const PlayerList &, Config & ) const;
    virtual bool canParticipate( PCharacter * ) const;
    virtual GlobalQuestPointer getQuestInstance( ) const;
    virtual int getDefaultTime( ) const;

    virtual const DLString & getQuestID( ) const;

    inline static GangstersInfo* getThis( ); 

protected:

    XML_VARIABLE XMLInteger vnumMob;
    XML_VARIABLE XMLInteger vnumChef;
    XML_VARIABLE XMLInteger vnumKey;
    XML_VARIABLE XMLInteger vnumPortalCity;
    XML_VARIABLE XMLInteger vnumPortalForest;
    XML_VARIABLE XMLInteger vnumLair;
    XML_VARIABLE XMLInteger lowGap, highGap;

private:

    static const DLString QUEST_ID;
    
    static GangstersInfo *thisClass;
};


inline GangstersInfo* GangstersInfo::getThis( ) 
{
    return thisClass;
}

#endif

