/* $Id: scenario_urka.h,v 1.1.2.3.18.1 2007/09/29 19:34:04 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __KIDNAPURKASCENARIO_H__
#define __KIDNAPURKASCENARIO_H__

#include "scenario.h"

class KidnapUrkaScenario: public KidnapScenario {
XML_OBJECT
public:

    virtual bool applicable( PCharacter * ) const;

    virtual void msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const;
    virtual void msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const;
    virtual void msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) const;

    virtual void actAttackHero( NPCharacter *bandit, PCharacter *hero ) const;
    virtual void actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const;
    virtual void actHuntStep( NPCharacter *bandit ) const;
    virtual void actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const;
    virtual void actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const;

    virtual void actLegend( NPCharacter *king, PCharacter *hero, ::Pointer<KidnapQuest> quest ) const;
    virtual void actGiveMark( NPCharacter *king, PCharacter *hero, Object *mark, int time ) const;
    virtual void actMarkLost( NPCharacter *king, PCharacter *hero, Object *mark ) const;
    virtual void actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const;
   
    virtual void actHeroWait( NPCharacter *kid ) const;
    virtual void actNoHero( NPCharacter *kid, PCharacter *hero ) const;
    virtual void actHeroDetach( NPCharacter *kid, PCharacter *hero ) const;
    virtual void actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const;
    virtual void actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const;
    virtual void actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const;
    virtual void actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const;
    virtual void actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const;
};

class KidnapUrkaPoliteScenario : public KidnapUrkaScenario {
XML_OBJECT
public:
    virtual void actLegend( NPCharacter *king, PCharacter *hero, ::Pointer<KidnapQuest> quest ) const;
    virtual void actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const;
    virtual void actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const;
    virtual void actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const;
    virtual void actHeroDetach( NPCharacter *kid, PCharacter *hero ) const;
    virtual void actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const;
};

#endif
