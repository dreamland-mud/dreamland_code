#ifndef DEFAULTBEHAVIOR_H
#define DEFAULTBEHAVIOR_H

#include "behavior.h"
#include "xmltableelement.h"

/** 
 * Declares a behavior that can be auto-loaded from its XML profile
 * from a folder on disk. Loader is defined in skills_loader plugin.
 */
class DefaultBehavior : public Behavior, public XMLTableElement {
XML_OBJECT    
public:
    typedef ::Pointer<DefaultBehavior> Pointer;
    
    DefaultBehavior();

    virtual void loaded();
    virtual void unloaded();
    virtual const DLString & getName() const;
    virtual void setName(const DLString &);
};

#endif