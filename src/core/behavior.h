#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <jsoncpp/json/json.h>
#include "oneallocate.h"
#include "xmlvariablecontainer.h"
#include "wrappertarget.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmljsonvalue.h"
#include "xmlenumeration.h"

/**
 * A representation of 'behavior' - collection of triggers and properties - describing how an item,
 * mob or a room behaves itself (duh). Room behavior examples include various traps and obstacles; 
 * mob behavior is what historically has been defined in 'act' flags (mage/cleric/assassin/scavenger)
 * and spec_fun procedures. Item behaviors is essentially item type, or anything currently assigned
 * in Fenia that overrides 'onUse'. 
 * An item/mob/room can have multiple behaviors assigned to it and called at different situations. 
 * Behavior triggers get called after quest triggers and before normal onXXX triggers.
 * 
 * Behaviors can be soft-referenced via global reference and can have Fenia fields assigned to them.
 */
class Behavior: public XMLVariableContainer, public WrapperTarget, public GlobalRegistryElement {
XML_OBJECT
public:
    typedef ::Pointer<Behavior> Pointer;
    
    Behavior();
    Behavior(const DLString &);
    virtual ~Behavior();

    virtual long long getID() const;

    virtual const DLString &getName() const;
    virtual bool isValid() const;
    virtual const DLString &getRussianName() const;

    // Internal ID to use as unique ID for Fenia.
    XML_VARIABLE XMLInteger id;

    XML_VARIABLE XMLString nameRus;

    // Where is it assigned: mob/obj/room.
    XML_VARIABLE XMLEnumeration target;

    // Command alternative to 'use': e.g. zap, fire, snare.
    XML_VARIABLE XMLString cmd;
    
    // Json string of properties. Can have default values, otherwise 
    // each index data overrides properties values for each behavior it has.
    XML_VARIABLE XMLJsonValue props;

protected:
    DLString name;
};

/**
 * A collection of all behaviors.
 */
class BehaviorManager : public GlobalRegistry<Behavior>, 
                          public OneAllocate
{
public:
    
    BehaviorManager();
    virtual ~BehaviorManager();
    
    inline static BehaviorManager *getThis();
    /** Returns the next available ID. */
    int getNextId() const;

private:
    /** If behavior is not loaded yet but mentioned in the code, substitute this stub instead. */
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern BehaviorManager * behaviorManager;

inline BehaviorManager * BehaviorManager::getThis()
{   
    return behaviorManager;
}

GLOBALREF_DECL(Behavior)
XMLGLOBALREF_DECL(Behavior)



#endif