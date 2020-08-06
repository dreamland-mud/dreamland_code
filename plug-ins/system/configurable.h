#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include <map>
#include <list>
#include "plugin.h"
#include "oneallocate.h"
#include "xmlpolymorphvariable.h"
#include "class.h"
#include "classselfregistratorplugin.h"
#include "plugininitializer.h"
#include "dlfile.h"

namespace Json { class Value; }

class Configurable: public virtual Plugin {
public:
    typedef ::Pointer<Configurable> Pointer;

    virtual void initialization();
    virtual void destruction();
    virtual const DLString & getPath() const = 0;
    virtual void loaded(Json::Value) { }
    virtual void unloaded() { }
    DLString getText() const;
    void setText(const DLString &text);
    bool validate(const DLString &text, ostringstream &errbuf) const;
};

class ConfigurableRegistry: public OneAllocate, public Plugin {
public:
    ConfigurableRegistry();
    virtual ~ConfigurableRegistry();

    virtual void initialization();
    virtual void destruction();

    void add(Configurable::Pointer);
    void remove(Configurable::Pointer);
    Configurable::Pointer get(const DLString &path) const;
    std::list<Configurable::Pointer> getAll(const DLString &path) const;
    std::list<DLString> getPaths() const;

protected:
    std::map<DLString, Configurable::Pointer> elements;
};

extern ConfigurableRegistry *configReg;

template <const char *&tn>
class ConfigurableTemplate : public Configurable, public ClassSelfRegistratorPlugin<tn> {
public:
    typedef ::Pointer<ConfigurableTemplate> Pointer;
    
    ConfigurableTemplate( ) {
        _path = DLFile(folder,file).getPath();
    }

    virtual const DLString & getPath() const {
        return _path;
    }

    virtual void loaded(Json::Value) {         
    }

    virtual void unloaded() { 
    }

protected:
    virtual void initialization( ) 
    {
        ClassSelfRegistratorPlugin<tn>::initialization( );
        Configurable::initialization( );
    }
    virtual void destruction( ) 
    {
        Configurable::destruction( );
        ClassSelfRegistratorPlugin<tn>::destruction( );
    }
    virtual void fromXML( const XMLNode::Pointer& node )  {
    }
    virtual bool toXML( XMLNode::Pointer& node ) const {
        return false;
    }
    
private:
    static const char *file;
    static const char *folder;
    DLString _path;
};

#define CONFIGURABLE_DUMMY(x)         config_ ##x## _TypeName
#define CONFIGURABLE(x) ConfigurableTemplate<CONFIGURABLE_DUMMY(x)>

#define CONFIGURABLE_DECL(collection,x) \
const char * CONFIGURABLE_DUMMY(x) =  #x; \
template<> const char *CONFIGURABLE(x)::file = #x; \
template<> const char *CONFIGURABLE(x)::folder = #collection; \
PluginInitializer<CONFIGURABLE(x)> config_ ##x## _init(INITPRIO_NORMAL);

#define CONFIGURABLE_LOADED(collection,x) \
CONFIGURABLE_DECL(collection,x) \
template <> void CONFIGURABLE(x)::loaded( Json::Value value )




#endif
    