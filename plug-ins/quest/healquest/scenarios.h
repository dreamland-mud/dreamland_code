/* $Id$
 *
 * ruffina, 2004
 */
#ifndef HEALQUEST_SCENARIOS_H
#define HEALQUEST_SCENARIOS_H

#include "xmlflags.h"
#include "xmlboolean.h"
#include "skillreference.h"
#include "questscenario.h"

class HealScenario : public QuestScenario, public virtual XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<HealScenario> Pointer;
    
    HealScenario( );

    virtual bool applicable( PCharacter * ) const;
    virtual bool applicable( PCharacter *, NPCharacter * ) const;
    virtual bool healedBy( int );
    virtual bool isInfected( NPCharacter * );
    virtual void infect( NPCharacter *, int time, int level ) const;

    XML_VARIABLE XMLSkillReference malady;
    XML_VARIABLE XMLStringVector remedies;
    XML_VARIABLE XMLBooleanNoFalse obvious;
    XML_VARIABLE XMLFlagsWithTable bit; 
    XML_VARIABLE XMLFlagsNoEmpty   immune; 
    XML_VARIABLE XMLFlagsNoEmpty   profession;

protected:
    bool cancelOnly( PCharacter * ) const;
};

typedef vector<HealScenario::Pointer> HealScenarioList;

#endif
