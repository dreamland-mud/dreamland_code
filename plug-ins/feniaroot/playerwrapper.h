#ifndef PLAYERWRAPPER_H
#define __STRUCTPLAYERWRAPPER_HWRAPPERS_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

using Scripting::NativeHandler;

class PCMemoryInterface;

class PlayerWrapper : public PluginNativeImpl<PlayerWrapper>, 
                        public NativeHandler,
                        public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<PlayerWrapper> Pointer;

    PlayerWrapper() { }
    PlayerWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    virtual Scripting::Object *getSelf() const { return 0; }
    PCMemoryInterface * getTarget() const;
    static Scripting::Register wrap( const DLString & );

protected:
    void save() const;
    
    XML_VARIABLE XMLString name;
};


#endif
