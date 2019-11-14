/* $Id: globalquestinfo.h,v 1.1.2.1.6.1 2008/11/13 03:33:28 rufina Exp $
 * 
 * ruffina, 2003
 */
#ifndef GLOBALQUESTINFO_H
#define GLOBALQUESTINFO_H

#include <vector>

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

#include "plugin.h"

class GlobalQuest;
class PCharacter;

/*---------------------------------------------------------------------------
 * GlobalQuestInfo
 *---------------------------------------------------------------------------*/
class GlobalQuestInfo : public Plugin, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<GlobalQuestInfo> Pointer;
    typedef ::Pointer<GlobalQuest> GlobalQuestPointer;
    typedef vector<PCharacter *> PlayerList;

    struct Config {
        int minLevel, maxLevel;
        int time;
        DLString arguments;
        bool force;
        int playerCnt;

        bool parseLevels( DLString &, ostringstream & );
        bool parsePlayerCount( DLString &, int, ostringstream & );
        bool parseTime( DLString &, int, ostringstream & );
        bool parseArguments( DLString &, ostringstream & );
    };
    
    virtual void initialization( );
    virtual void destruction( );
    
    virtual bool parseArguments( const DLString &, Config &, ostringstream & ) const = 0;
    void findParticipants( PlayerList & ) const;
    virtual bool canParticipate( PCharacter * ) const;
    virtual bool canHear( PCharacter * ) const;
    virtual bool canAutoStart( const PlayerList &, Config & ) const = 0;
    virtual void tryStart( const Config & );
    virtual GlobalQuestPointer getQuestInstance( ) const = 0;
    virtual int getDefaultTime( ) const = 0;
    
    virtual const DLString & getQuestID( ) const = 0;

    inline virtual const DLString & getQuestName( ) const;  
    inline const DLString & getQuestShortDescr( ) const;
    inline int getLastTime( ) const;
    inline void setLastTime( int );
    inline int getWaitingTime( ) const;
    inline void setWaitingTime( int );
    inline bool getAutostart( ) const;
    inline void setAutostart( bool );

protected:

    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString shortDescr;
    XML_VARIABLE XMLInteger lastTime;
    XML_VARIABLE XMLInteger waitingTime;
    XML_VARIABLE XMLBoolean autostart;
};


inline const DLString & GlobalQuestInfo::getQuestName( ) const
{
    return name.getValue( );
}
inline const DLString & GlobalQuestInfo::getQuestShortDescr( ) const
{
    return shortDescr.getValue( );
}

inline int GlobalQuestInfo::getLastTime( ) const
{
    return lastTime.getValue( );
}

inline void GlobalQuestInfo::setLastTime( int time )
{
    lastTime = time;
}

inline int GlobalQuestInfo::getWaitingTime( ) const
{
    return waitingTime.getValue( );
}

inline void GlobalQuestInfo::setWaitingTime( int time )
{
    waitingTime = time;
}

inline bool GlobalQuestInfo::getAutostart( ) const
{
    return autostart.getValue( );
}

inline void GlobalQuestInfo::setAutostart( bool autostart )
{
    this->autostart = autostart;
}

/*---------------------------------------------------------------------------
 * GQuestInfoEveryone
 *---------------------------------------------------------------------------*/
class GQuestInfoEveryone : public GlobalQuestInfo {
XML_OBJECT    
public:   
    typedef ::Pointer<GQuestInfoEveryone> Pointer;

    virtual bool parseArguments( const DLString &, Config &, ostringstream & ) const;
    virtual bool canAutoStart( const PlayerList &, Config & ) const;

protected:
    XML_VARIABLE XMLInteger minPlayers;
};

/*---------------------------------------------------------------------------
 * GQuestInfoLevels
 *---------------------------------------------------------------------------*/
class GQuestInfoLevels : public GlobalQuestInfo {
XML_OBJECT    
public:   
    typedef ::Pointer<GQuestInfoLevels> Pointer;

    virtual bool parseArguments( const DLString &, Config &, ostringstream & ) const;
};

#endif

