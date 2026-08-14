/* $Id: locatequest.h,v 1.1.2.10.6.3 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef LOCATEQUEST_H
#define LOCATEQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questregistrator.h"
#include "questscenario.h"

#include "xmlshort.h"

class LocateScenario;
class NPCharacter;

enum {
    QSTAT_SEARCH = 1,
};

class LocateQuest : public ClientQuestModel, public ItemQuestModel {
friend class LocateCustomer;
XML_OBJECT
public:
    typedef ::Pointer<LocateQuest> Pointer;
    
    LocateQuest( );

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual QuestReward::Pointer reward( PCharacter *, NPCharacter * );
    virtual void destroy( );

    const LocateScenario & getScenario( ) const;

    // scenName is a lookup key into the registrator, not display text -- it stays
    // single-language. Everything below is read back out to the player, so each
    // is captured per language at create() time; XMLMultiString::fromXML files an
    // attribute-less node under RU, so quests already in flight load unchanged.
    XML_VARIABLE XMLString scenName;
    XML_VARIABLE XMLMultiString itemName;
    XML_VARIABLE XMLMultiString itemMltName;
    XML_VARIABLE XMLMultiString targetArea;
    XML_VARIABLE XMLMultiString customerName;
    XML_VARIABLE XMLMultiString customerRoom;
    XML_VARIABLE XMLMultiString customerArea;
    XML_VARIABLE XMLInteger total;
    XML_VARIABLE XMLInteger delivered;

protected:
    virtual bool checkRoomClient( PCharacter *, Room * );
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );

private:
    void scatterItems( PCharacter *, Room *, NPCharacter * );
};

class LocateQuestRegistrator : public QuestRegistrator<LocateQuest>,
                               public QuestScenariosContainer
{
XML_OBJECT
public:
    LocateQuestRegistrator( );
    virtual ~LocateQuestRegistrator( );

    virtual bool applicable( PCharacter *, bool ) const;

    static inline LocateQuestRegistrator * getThis( ) {
        return thisClass;
    }

    XML_VARIABLE XMLInteger itemVnum;

private:
    static LocateQuestRegistrator *thisClass;
};

#endif
