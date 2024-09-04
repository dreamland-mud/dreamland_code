#ifndef BEDIT_H
#define BEDIT_H

#include "olcstate.h"
#include "xmlinteger.h"
#include "behavior.h"

class DefaultBehavior;

class OLCStateBehavior : public OLCStateTemplate<OLCStateBehavior>,
                        public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateBehavior> Pointer;

    OLCStateBehavior();
    OLCStateBehavior(DefaultBehavior *bhv);
    virtual ~OLCStateBehavior();

    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLBehaviorReference original;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );

    DefaultBehavior *getOriginal();
    
    XML_VARIABLE XMLBoolean isChanged;
};

#define BEDIT(C, cname, help) OLC_CMD(OLCStateBehavior, C, cname, help)



#endif
