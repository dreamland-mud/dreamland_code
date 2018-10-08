/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __COMMANDWRAPPER_H__
#define __COMMANDWRAPPER_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlshort.h"
#include "xmlglobalbitvector.h"
#include "xmlregister.h"
#include "skillreference.h"

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"
#include "defaultcommand.h"

using Scripting::XMLRegister;
using Scripting::NativeHandler;

class CommandWrapper : public PluginNativeImpl<CommandWrapper>, 
                       public NativeHandler,
                       public DefaultCommand, 
                       public virtual XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<CommandWrapper> Pointer;

    virtual void setSelf(Scripting::Object *);

    virtual void run( Character * ch, const DLString & );

    virtual void backup();
private:
    XML_VARIABLE XMLRegister func;
    Scripting::Object *self;
};

#endif
