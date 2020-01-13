/* $Id: gangsters.h,v 1.1.2.2.6.1 2008/04/14 19:36:15 rufina Exp $
 * 
 * ruffina, 2003
 */
#ifndef GANGSTERS_H
#define GANGSTERS_H

#include "globalquest.h"

#include "xmlvector.h"
#include "logstream.h"
#define log(x) LogStream::sendNotice() << "Gangsters: " << x << endl


class Gangsters : public GlobalQuest {
XML_OBJECT    
friend class GangMember;    
friend class GangChef;    
friend class GangKey;
friend class GangPortal;

public:
    typedef ::Pointer<Gangsters>        Pointer;
    
    Gangsters( );
    Gangsters( const DLString& );
    virtual ~Gangsters( );
    
    virtual void create( const Config& ) ;
    virtual void destroy( );

    virtual void resume( );
    virtual void after( );
    virtual int getTaskTime( ) const;

    virtual void report( std::ostringstream &, PCharacter* ) const;
    virtual void progress( std::ostringstream & ) const;

    virtual void getQuestDescription( std::ostringstream & ) const;
    virtual void getQuestStartMessage( std::ostringstream & ) const;

    inline static Gangsters* getThis( );

protected:
    NPCharacter * createMob( );
    NPCharacter * createChef( );
    Object * createPortal( RoomList& );
    Object * createKey( );
public:    
    void resetKeys( );

protected:
    void rewardLeader( );
    void rewardChefKiller( );
    void rewardNobody( );
    void rewardMobKiller( PCharacter *, Character * );

    inline const DLString & getHint( ) const;
    inline void setHint( const DLString & );

    void createFirstHint( MobileList & );
    Room * findHintRoom( std::ostringstream& );
    bool createSecondHint( );
    void createThirdHint( );

    void populateArea( area_data *, RoomList& , int );
    void populateLair( );

    DLString lairHint( );
    Room * pickRandomRoom( );
    static Room * recursiveWalk ( Room *, int, int );
    static bool isPoliceman( Character * );
    static bool checkRoom( Room *const );

    XML_VARIABLE XMLString areaName;
    XML_VARIABLE XMLString informerName;
    XML_VARIABLE XMLString informerRoom;
    XML_VARIABLE XMLString hint;
    XML_VARIABLE XMLInteger hintCount;
    XML_VARIABLE XMLString chefKiller;
    XML_VARIABLE XMLInteger keyCount;
    
    XML_VARIABLE XMLVectorBase<XMLInteger> mobRoomVnums;
    XML_VARIABLE XMLVectorBase<XMLInteger> portalRoomVnums;
    
    enum {
        ST_NONE,
        ST_NO_MORE_HINTS,
        ST_BROKEN,
        ST_CHEF_KILLED,
    };
    XML_VARIABLE XMLInteger state;

private:
    void cleanup( bool );

    static Gangsters *thisClass;
};

inline const DLString & Gangsters::getHint( ) const
{
    return hint.getValue( );
}

inline void Gangsters::setHint( const DLString &hint ) 
{
    this->hint = hint;
}

inline Gangsters * Gangsters::getThis( )
{
    return thisClass;
}


#endif

