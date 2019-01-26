/* $Id: butcherquest.h,v 1.1.2.13.10.1 2007/09/29 19:33:49 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef BUTCHERQUEST_H
#define BUTCHERQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questregistrator.h"
#include "questscenario.h"

#include "xmlshort.h"

class ButcherQuest : public ClientQuestModel, public VictimQuestModel {
friend class SteakCustomer;
XML_OBJECT
public:
    typedef ::Pointer<ButcherQuest> Pointer;

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual void destroy( );

    XML_VARIABLE XMLString raceName;
    XML_VARIABLE XMLString raceRusName;
    XML_VARIABLE XMLString areaName;
    XML_VARIABLE XMLString customerName;
    XML_VARIABLE XMLString customerSex;
    XML_VARIABLE XMLString customerArea;
    XML_VARIABLE XMLShort  ordered;
    XML_VARIABLE XMLShort  delivered;

protected:
    virtual bool checkRoomVictim( PCharacter *, Room * );
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );
    virtual bool checkMobileVictim( PCharacter *, NPCharacter * );
};

class ButcherQuestRegistrator : public QuestRegistrator<ButcherQuest> {
XML_OBJECT
public:
    ButcherQuestRegistrator( );
    virtual ~ButcherQuestRegistrator( );

    virtual bool applicable( PCharacter *, bool ) const;

    static inline ButcherQuestRegistrator * getThis( ) {
        return thisClass;
    }
    
    XML_VARIABLE XMLReverseVector<XMLString> races;
    XML_VARIABLE NameList cooks;

private:    
    static ButcherQuestRegistrator * thisClass;
};

#endif
