#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include <map>
#include <list>

#include "logstream.h"
#include "plugin.h"
#include "oneallocate.h"
#include "xmlpolymorphvariable.h"
#include "class.h"
#include "classselfregistratorplugin.h"
#include "plugininitializer.h"
#include "flags.h"
#include "dlfile.h"
#include "dldirectory.h"
#include "dreamland.h"

class Configurable: public virtual DLObject {
public:
    typedef ::Pointer<Configurable> Pointer;

    virtual ~Configurable();
    
    void load();
    void save();
    void unload();

    void setPath(const DLString &path) { this->path = path; }
    const DLString & getPath() const { return path; }
    const DLString & getText() const { return text; }
    void setText(const DLString &text);
    const Json::Value &getValue() const { return value; }

protected:
    virtual void loaded() { }
    virtual void unloaded() { }

    DLString getAbsolutePath() const;

    DLString path;
    DLString text;
    Json::Value value;
};

class ConfigurablePlugin: public virtual Plugin, public Configurable {
public:
    typedef ::Pointer<ConfigurablePlugin> Pointer;

    virtual void initialization();
    virtual void destruction();
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
class ConfigurableTemplate : public ConfigurablePlugin, public ClassSelfRegistratorPlugin<tn> {
public:
    typedef ::Pointer<ConfigurableTemplate> Pointer;
    
    ConfigurableTemplate( ) {
        path = DLFile(folder,file).getPath();
    }


protected:
    virtual void loaded() {  
    }

    virtual void initialization( ) 
    {
        ClassSelfRegistratorPlugin<tn>::initialization( );
        ConfigurablePlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        ConfigurablePlugin::destruction( );
        ClassSelfRegistratorPlugin<tn>::destruction( );
    }
    virtual void fromXML( const XMLNode::Pointer& node )  { }
    virtual bool toXML( XMLNode::Pointer& node ) const { return false; }
    
private:
    static const char *file;
    static const char *folder;
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
template <> void CONFIGURABLE(x)::loaded()

template <typename S>
struct json_vector : public vector<S> {
    void fromJson(const Json::Value &value) 
    {
        this->clear();
        this->resize(value.size());
        for (unsigned int i = 0; i < value.size(); i++) {
            this->at(i).fromJson(value[i]);
        }
    }
};

class Flags;
template <const FlagTable *_table>
struct json_flag : public Flags {
    json_flag() 
        : Flags(0, _table)
    {
    }

    void fromJson(const Json::Value &value) 
    {
        bitstring_t b;
        DLString valueString = value.asString();

        if (valueString.empty())
            return;
            
        if (table->enumerated)
            b = table->value(value.asString());
        else
            b = table->bitstring(value.asString());

        this->setValue(b);
    }
};


#endif
    