#ifndef BEHAVIORWRAPPER_H
#define BEHAVIORWRAPPER_H

#include "pluginwrapperimpl.h"

class Behavior;

class BehaviorWrapper : public PluginWrapperImpl<BehaviorWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<BehaviorWrapper> Pointer;
    
    BehaviorWrapper();

    virtual void setSelf(Scripting::Object *);
    void setTarget(Behavior *);
    void checkTarget() const ;
    virtual void extract(bool);
    Behavior *getTarget() const;

    static Scripting::Register wrap(const DLString &);

private:        
    Behavior *target;
};

#endif