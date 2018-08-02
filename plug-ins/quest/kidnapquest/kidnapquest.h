/* $Id: kidnapquest.h,v 1.1.2.12.6.2 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KIDNAPQUEST_H
#define KIDNAPQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"

#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlvector.h"
#include "xmlpointer.h"

class KidnapQuestRegistrator;
class KidnapScenario;
class NPCharacter;
class Object;
class Room;

enum {
    QSTAT_MARK_RCVD = 1, 
    QSTAT_KID_FOUND, 
    QSTAT_KING_ACK_WAITING,   
};

class KidnapQuest : public ClientQuestModel, public ItemQuestModel {
friend class KidnapMobile;
friend class KidnapBandit;
friend class KidnapKing;
friend class KidnapPrince;
XML_OBJECT
public:
    typedef ::Pointer<KidnapQuest> Pointer;
    
    KidnapQuest( );

    virtual void create( PCharacter *, NPCharacter * );
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual bool help( PCharacter *, NPCharacter * );
    virtual Room * helpLocation( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual void destroy( );

    XML_VARIABLE XMLInteger range;
    XML_VARIABLE XMLInteger ambushes;

    XML_VARIABLE XMLString kingName;
    XML_VARIABLE XMLString kingRoom;
    XML_VARIABLE XMLString kingArea;
    XML_VARIABLE XMLString princeName;
    XML_VARIABLE XMLString princeRoom;
    XML_VARIABLE XMLString princeArea;
    XML_VARIABLE XMLString scenName;
    XML_VARIABLE XMLInteger kingVnum;
    XML_VARIABLE XMLBooleanNoFalse debug;
    
protected:
    virtual bool checkRoomClient( PCharacter *, Room * );
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );

    void destroyBandits( );
    KidnapScenario &getScenario( );
    NPCharacter * createBandit( );
    Object * createMark( );

private:    
    KidnapQuestRegistrator * getReg( );
    
    NPCharacter * createKing( PCharacter * );
    NPCharacter * createPrince( NPCharacter *, Room * );
    Room * findRefuge( PCharacter *, NPCharacter * );
};


#endif
