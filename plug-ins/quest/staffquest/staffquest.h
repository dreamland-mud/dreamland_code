/* $Id: staffquest.h,v 1.1.2.15.6.2 2007/09/29 19:34:08 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef STAFFQUEST_H
#define STAFFQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questregistrator.h"
#include "questscenario.h"

class StaffScenario;

class StaffQuest : public ItemQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<StaffQuest> Pointer;

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual Room *helpLocation( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual void destroy( );

    XML_VARIABLE XMLString roomName;
    XML_VARIABLE XMLString areaName;
    XML_VARIABLE XMLString objName;
    XML_VARIABLE XMLString scenName;

protected:
    virtual bool checkRoomClient( PCharacter *, Room * );

private:
    Object *createStaff( Room * );

    StaffScenario & getScenario( );
};

class StaffScenario : public QuestItemAppearence, public QuestScenario {
XML_OBJECT
public:
    virtual bool applicable( PCharacter * );

    void onQuestStart( PCharacter *, NPCharacter * );

    XML_VARIABLE XMLString msg;
};

class StaffQuestRegistrator : public QuestRegistrator<StaffQuest>,
                              public QuestScenariosContainer
{
XML_OBJECT
public:
    StaffQuestRegistrator( );
    virtual ~StaffQuestRegistrator( );

    virtual bool applicable( PCharacter * );

    static inline StaffQuestRegistrator * getThis( ) {
        return thisClass;
    }

    XML_VARIABLE XMLInteger objVnum;

private:
    static StaffQuestRegistrator *thisClass;
};

#endif
