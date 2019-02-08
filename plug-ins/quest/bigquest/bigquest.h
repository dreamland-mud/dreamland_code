/* $Id: killquest.h,v 1.1.2.15.6.2 2008/03/06 17:48:34 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef BIGQUEST_H
#define BIGQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questscenario.h"
#include "questregistrator.h"

#include "xmlshort.h"
#include "xmlstring.h"
#include "xmllimits.h"

class BigQuestScenario;
struct area_data;

class BigQuest : public VictimQuestModel,
                 public ItemQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<BigQuest> Pointer;
    
    BigQuest( );

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual bool hasPartialRewards() const;
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual Room * helpLocation( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual void destroy( );

    void mobKilled(PCMemoryInterface *hero, Character *killer);
    void mobDestroyed(PCMemoryInterface *hero);

    XML_VARIABLE XMLShort mode;       
    XML_VARIABLE XMLString areaName;       
    XML_VARIABLE XMLString scenName;

    XML_VARIABLE XMLInteger mobsTotal;
    XML_VARIABLE XMLInteger mobsKilled;
    XML_VARIABLE XMLInteger mobsDestroyed;
    XML_VARIABLE XMLInteger objsTotal;
    
protected:
   
    BigQuestScenario &getScenario();
    void notifyNoMore(PCMemoryInterface *hero);
};

class BigQuestScenario : public QuestScenario, public XMLVariableContainer {
XML_OBJECT
public:
    virtual bool applicable( PCharacter * ) const;
    virtual int getPriority() const;
    QuestMobileAppearence &getRandomMobile();
    void onQuestStart(PCharacter *, NPCharacter *, struct area_data *, int);
    void onQuestInfo(PCharacter *, int, ostream&);

protected:
    XML_VARIABLE XMLInteger priority;
    XML_VARIABLE XMLLimits criteria;
    XML_VARIABLE QuestMobileAppearanceList mobiles;
    XML_VARIABLE XMLInteger item;
    XML_VARIABLE XMLStringVector msgStart;
    XML_VARIABLE XMLString msgInfo;
};

class BigQuestRegistrator : public QuestRegistrator<BigQuest>,
                            public QuestScenariosContainer
{
XML_OBJECT
public:
    BigQuestRegistrator( );
    virtual ~BigQuestRegistrator( );

    virtual bool applicable( PCharacter *, bool fAuto ) const;
    static inline BigQuestRegistrator * getThis( ) {
        return thisClass;
    }

private:
    static BigQuestRegistrator *thisClass;
};

#endif
