/* $Id: rainbow.h,v 1.1.2.1.6.1 2008/03/04 07:24:12 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef RAINBOW_GQUEST_H 
#define RAINBOW_GQUEST_H 

#include <vector>

#include "globalquest.h"

#include "xmlvector.h"
#include "logstream.h"
#define log(x) LogStream::sendNotice() << "Rainbow: " << x << endl

class PCharacter;
class NPCharacter;
class Object;
class Room;
struct area_data;
class RainbowScenario;
class RainbowMob;

class RainbowGQuest : public GlobalQuest {
XML_OBJECT    
friend class RainbowMob;
friend class RainbowPiece;

public:
    typedef ::Pointer<RainbowGQuest>	Pointer;
    
    RainbowGQuest( );
    RainbowGQuest( const DLString& );
    virtual ~RainbowGQuest( );
    
    virtual void create( const Config& ) throw ( GQCannotStartException );
    virtual void destroy( );
    
    virtual void after( );
    virtual int getTaskTime( ) const;
    virtual const DLString & getDisplayName( ) const;

    virtual void report( std::ostringstream &, PCharacter* ) const;
    virtual void progress( std::ostringstream & ) const;

    virtual void getQuestDescription( std::ostringstream & ) const;
    virtual void getQuestStartMessage( std::ostringstream & ) const;
    
    virtual bool isHidden( ) const;

    inline static RainbowGQuest* getThis( );

protected:
    static int countPieces( Character * );
    Object * createPiece( int );
    void recreateMob( RainbowMob * );
    RainbowScenario * getScenario( ) const;

    enum {
	ST_INIT,
	ST_RUNNING,
	ST_ENDING,
	ST_FINISHED
    };
    XML_VARIABLE XMLInteger state;
    XML_VARIABLE XMLString winnerName;
    XML_VARIABLE XMLVectorBase<XMLInteger> roomVnums;
    XML_VARIABLE XMLString scenName;

private:
    void rewardNobody( );
    void rewardWinner( );
    
    void cleanup( );
    
    static RainbowGQuest *thisClass;
};

inline RainbowGQuest * RainbowGQuest::getThis( )
{
    return thisClass;
}


#endif
