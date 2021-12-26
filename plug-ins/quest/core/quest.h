/* $Id: quest.h,v 1.1.4.5.6.4 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef QUEST_H
#define QUEST_H

#include <sstream>
#include <vector>

#include "xmlvariablecontainer.h"
#include "plugin.h"
#include "xmlattribute.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlboolean.h"

#include "logstream.h"
#define log(x) LogStream::sendNotice() << x << endl

class Character;
class PCharacter;
class NPCharacter;
class PCMemoryInterface;
class Object;
class Room;

enum {
    QSTAT_INIT,
    QSTAT_STARTED,
    /* ... */
    QSTAT_BROKEN_BY_HERO = 42,
    QSTAT_BROKEN_BY_OTHERS,
    QSTAT_FINISHED,  
};

/*
 * Quest reward
 */
struct QuestReward : public virtual DLObject {
    typedef ::Pointer<QuestReward> Pointer;
    
    QuestReward( ) : points( 0 ), exp( 0 ), gold( 0 ), prac( 0 ), clanpoints( 0 ),
                wordChance( 0 ), scrollChance( 0 )
    {
    }

    int points;
    int exp;
    int gold;
    int prac;
    int clanpoints;
    int wordChance;
    int scrollChance;
};

/*
 * Quest
 */
class Quest :        public XMLAttribute, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<Quest> Pointer;
    
    Quest( );

    virtual void create( PCharacter *, NPCharacter * ) = 0; 
    virtual bool isComplete( ) = 0;
    virtual bool hasPartialRewards() const;
    virtual void info( std::ostream &, PCharacter * ) = 0;
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual QuestReward::Pointer reward( PCharacter *, NPCharacter * ) = 0;
    virtual void scheduleDestroy( );

    virtual bool help( PCharacter *, NPCharacter * );
    virtual void helpMessage( ostringstream & );
    virtual Room *helpLocation( );
    
    void wiznet( const char *status, const char * format = NULL, ... );

    virtual Character * getActor( Character * );

    virtual int getAccidentTime( PCMemoryInterface * );
    virtual int getPunishTime( PCMemoryInterface * );
    virtual int getDeathTime( PCMemoryInterface * );
    virtual int getFailTime( PCMemoryInterface * );
    virtual int getNextTime( PCMemoryInterface * );
    virtual int getCancelTime( PCMemoryInterface * );
    void setTime( PCMemoryInterface *, int );
    int getTime( PCMemoryInterface * );

    XML_VARIABLE XMLInteger     hint;
    XML_VARIABLE XMLString        charName;
    XML_VARIABLE XMLInteger     state;

protected:
    PCharacter * getHeroWorld( );
    PCMemoryInterface * getHeroMemory( );
    
    template<typename QT> inline bool check( Character * );
    template<typename QT> inline bool check( Object * );
};


#endif
