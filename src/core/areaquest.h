#ifndef AREAQUEST_H
#define AREAQUEST_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmllist.h"
#include "xmlvector.h"
#include "wrappertarget.h"
#include "xmlpointer.h"

class AreaIndexData;

/** This structure represents one step inside an area quest.
 * Consists of two blocks: step beginning and step ending. Each block
 * describes which trigger is executed for the step.
*/
class QuestStep : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<QuestStep> Pointer;
    typedef ::XMLPointer<QuestStep> XMLPointer;

    QuestStep();
    virtual ~QuestStep();

    // What starts the step: mob/item/room.
    XML_VARIABLE XMLString beginType;
    // Vnum of mob/item/room that starts the step.
    XML_VARIABLE XMLString beginValue; 
    // Name of the step's trigger (onGreet, onGive, etc).
    XML_VARIABLE XMLString beginTrigger; 

    // What ends the step: mob/item/room.
    XML_VARIABLE XMLString endType;
    // Vnum of mob/item/room that ends the step.
    XML_VARIABLE XMLString endValue;
    // Name of the step's trigger (onGreet, onGive, etc).
    XML_VARIABLE XMLString endTrigger;

    // 'quest info' output for this step
    XML_VARIABLE XMLString info; 
};

/**
 * This class describes a single area quest for an area. Contains a list of steps.
 * Can have its own fields and triggers assigned from Fenia, e.g. onInfo, for more
 * complex non-default behavior. Registered and initialized in 'areas' plugin.
*/
class AreaQuest : public XMLVariableContainer, public WrapperTarget {
XML_OBJECT
public:
    typedef ::Pointer<AreaQuest> Pointer;
    typedef ::XMLPointer<AreaQuest> XMLPointer;

    AreaQuest();
    virtual ~AreaQuest();

    virtual long long getID() const;

    XML_VARIABLE XMLInteger vnum;
    XML_VARIABLE XMLString title;
    XML_VARIABLE XMLString description;
    XML_VARIABLE XMLInteger minLevel, maxLevel;
    XML_VARIABLE XMLInteger limitPerLife;
    XML_VARIABLE XMLVectorBase<QuestStep::XMLPointer> steps;

    AreaIndexData *pAreaIndex;
};

// Global map of all area quest instances by vnum.
extern map<int, AreaQuest *> areaQuests;

#endif