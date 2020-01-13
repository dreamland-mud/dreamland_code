/* $Id: globalquest.h,v 1.1.2.1.6.1 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */
#ifndef GLOBALQUEST_H
#define GLOBALQUEST_H

#include <sstream>
#include <vector>

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"

#include "schedulertask.h"
#include "dreamland.h"

#include "gqexceptions.h"
#include "globalquestinfo.h"

class PCharacter;
class NPCharacter;
class Character;
class Room;
class Object;
struct area_data;


class GlobalQuest : public SchedulerTask, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<GlobalQuest> Pointer;
    typedef vector<NPCharacter *> MobileList;
    typedef vector<Object *> ObjectList;
    typedef vector<Room *> RoomList;
    typedef vector<area_data *>        AreaList;
    typedef GlobalQuestInfo::Config Config;

    GlobalQuest( );
    GlobalQuest( const DLString& );

    virtual void create( const Config & )  = 0;
    virtual void destroy( ) = 0;
    void scheduleDestroy( );
    
    virtual void suspend( );
    virtual void resume( );

    virtual void after( );
    virtual int getPriority( ) const;
        
    
    virtual void report( std::ostringstream &, PCharacter* ) const = 0;
    virtual void progress( std::ostringstream & ) const = 0;
    
    inline virtual const DLString& getQuestID( ) const;
    virtual void getQuestDescription( std::ostringstream & ) const = 0;
    virtual void getQuestStartMessage( std::ostringstream & ) const = 0;

    inline int getElapsedTime( ) const;
    inline int getRemainedTime( ) const;
    inline void setTotalTime( int );
    inline int getTotalTime( ) const;
    inline void setStartTime( );
    inline int getStartTime( ) const;
    inline virtual int getTaskTime( ) const;
    void printRemainedTime( ostringstream& buf ) const;
    
    inline bool hasLevels( ) const;
    inline int getMinLevel( ) const;
    inline void setMinLevel( int );
    inline int getMaxLevel( ) const;
    inline void setMaxLevel( int );
    bool isLevelOK( Character * ) const;
    
    inline virtual bool isHidden( ) const;

    void clearAttributes( ) const;
    Character * getActor( Character * ) const;
    void exorcism( Character * ) const;
    void wipeRoom( Room * ) const;

protected:    
    XML_VARIABLE XMLString  questID;
    XML_VARIABLE XMLInteger startTime;
    XML_VARIABLE XMLInteger totalTime;

    XML_VARIABLE XMLInteger minLevel;
    XML_VARIABLE XMLInteger maxLevel;
};

inline const DLString& GlobalQuest::getQuestID( ) const
{
    return questID.getValue( );
}

inline int GlobalQuest::getElapsedTime( ) const
{
    return (dreamland->getCurrentTime( ) - startTime.getValue( )) / 60;
}

inline int GlobalQuest::getRemainedTime( ) const
{
    return getTotalTime( ) - getElapsedTime( );
}

inline int GlobalQuest::getTotalTime( ) const
{
    return totalTime.getValue( );
}

void GlobalQuest::setTotalTime( int time ) 
{
    totalTime = time;
}

void GlobalQuest::setStartTime( ) 
{
    startTime = dreamland->getCurrentTime( );
}

int GlobalQuest::getStartTime( ) const
{
    return startTime.getValue( );
}

int GlobalQuest::getTaskTime( ) const
{
    return getRemainedTime( );
}

inline bool GlobalQuest::hasLevels( ) const 
{
    return ((getMinLevel( ) > 0) && (getMaxLevel( ) > 0));
}

inline int GlobalQuest::getMinLevel( ) const
{
    return minLevel.getValue( );
}

inline void GlobalQuest::setMinLevel( int min ) 
{
    minLevel = min;
}

inline int GlobalQuest::getMaxLevel( ) const
{
    return maxLevel.getValue( );
}

inline void GlobalQuest::setMaxLevel( int max ) 
{
    maxLevel = max;
}

inline bool GlobalQuest::isHidden( ) const
{
    return false;
}

#endif

