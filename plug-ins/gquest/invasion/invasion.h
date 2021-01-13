/* $Id: invasion.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef INVASION_GQUEST_H 
#define INVASION_GQUEST_H 

#include <vector>

#include "globalquest.h"

#include "xmlvector.h"
#include "logstream.h"
#define log(x) LogStream::sendNotice() << "Invasion: " << x << endl

class PCharacter;
class NPCharacter;
class Object;
class InvasionScenario;
struct AreaIndexData;

class InvasionGQuest : public GlobalQuest {
XML_OBJECT    
friend class InvasionMob;
friend class InvasionHelper;
friend class InvasionInstrument;
public:
    typedef ::Pointer<InvasionGQuest>        Pointer;
    
    InvasionGQuest( );
    InvasionGQuest( const DLString& );
    virtual ~InvasionGQuest( );
    
    virtual void create( const Config & ) ;
    virtual void destroy( );

    virtual void report( std::ostringstream &, PCharacter* ) const;
    virtual void progress( std::ostringstream & ) const;

    virtual void getQuestDescription( std::ostringstream & ) const;
    virtual void getQuestStartMessage( std::ostringstream & ) const;

    inline static InvasionGQuest* getThis( );

protected:
    NPCharacter * createMob( );
    NPCharacter * createHelper( );
    Object * createObj( );
    Object * createInstrument( );
    int countInstruments( PCharacter * );
    
    InvasionScenario * getScenario( ) const;
    void rewardLeader( );
    void rewardKiller( PCharacter * );

    XML_VARIABLE XMLString scenName;
    XML_VARIABLE XMLInteger playerCnt;
    
private:
    void cleanup( bool );
    
    static InvasionGQuest *thisClass;
};

inline InvasionGQuest * InvasionGQuest::getThis( )
{
    return thisClass;
}


#endif

