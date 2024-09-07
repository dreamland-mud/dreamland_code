#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include <map>
#include <list>
#include "jsoncpp/json/value.h"
#include "plugin.h"
#include "oneallocate.h"
#include "xmlpolymorphvariable.h"
#include "class.h"
#include "classselfregistratorplugin.h"
#include "plugininitializer.h"
#include "flags.h"
#include "dlfile.h"

class Configurable: public virtual Plugin {
public:
    typedef ::Pointer<Configurable> Pointer;

    virtual void initialization();
    virtual void destruction();
    virtual void loaded(Json::Value &) { }
    virtual void unloaded() { }

    void refresh(const DLString &text);
    DLString getText() const;
    DLString getAbsolutePath() const;
    const DLString & getPath() const { return path; }
    const Json::Value &getValue() const { return value; }

protected:
    DLString path;
    Json::Value value;
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
        path = DLFile(folder,file).getPath();
    }

    virtual void loaded(Json::Value &) {  }

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
template <> void CONFIGURABLE(x)::loaded( Json::Value &value )

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
    