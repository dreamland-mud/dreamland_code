#ifndef BEHAVIORLOADER_H
#define BEHAVIORLOADER_H

#include "xmltableloaderplugin.h"
#include "behavior.h"
#include "xmltableelement.h"
#include "markuphelparticle.h"

class DefaultBehavior;

/** Describes help article for a behavior. */
class BehaviorHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<BehaviorHelp> Pointer;

    virtual void setBehavior(::Pointer<DefaultBehavior> );
    virtual void unsetBehavior( );
    virtual void save() const;
    
    virtual DLString getTitle(const DLString &label) const;
    virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:

    ::Pointer<DefaultBehavior> bhv;
};

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

    XML_VARIABLE XMLPointerNoEmpty<BehaviorHelp> help;    
};

/** 
 * A loader that can load and instantiate behaviors from XMLs in a catalog. 
 */
TABLE_LOADER_DECL(BehaviorLoader)

#endif
