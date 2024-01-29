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

class QuestStep : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<QuestStep> Pointer;
    typedef ::XMLPointer<QuestStep> XMLPointer;

    QuestStep();
    virtual ~QuestStep();

    XML_VARIABLE XMLString beginType; // mob, item
    XML_VARIABLE XMLString beginValue; // vnum of mob, item
    XML_VARIABLE XMLString beginTrigger; // onGreet, onGive, onTell...
    
    XML_VARIABLE XMLString endType;
    XML_VARIABLE XMLString endValue;
    XML_VARIABLE XMLString endTrigger;
};

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