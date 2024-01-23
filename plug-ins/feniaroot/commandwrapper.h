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

// NOTE: class names in this file should be the other way around. Had to call actual Command wrapper
// something else because 'CommandWrapper' was taken earlier, for a fenia-based command implementatino.

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
    virtual Scripting::Object *getSelf() const { return self; }
    virtual void run( Character * ch, const DLString & );

    virtual void backup();
private:
    XML_VARIABLE XMLRegister func;
    Scripting::Object *self;
};

class FeniaCommandWrapper : public PluginWrapperImpl<FeniaCommandWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaCommandWrapper> Pointer;
    
    FeniaCommandWrapper();

    virtual void setSelf( Scripting::Object * );
    void setTarget( Command* );
    void checkTarget( ) const ;
    virtual void extract( bool );
    Command *getTarget( ) const;

private:        
    Command *target;
};

#endif
