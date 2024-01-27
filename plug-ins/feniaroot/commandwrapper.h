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
#include "wrappedcommand.h"

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

using Scripting::XMLRegister;
using Scripting::NativeHandler;

/**
 * A way to add new command purely via Fenia. The command will be registered in setSelf.
 * Doesn't read any XML profile at the moment.
 */
class CommandWrapper : public PluginNativeImpl<CommandWrapper>, 
                       public NativeHandler,
                       public Command
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<CommandWrapper> Pointer;

    virtual void setSelf(Scripting::Object *);
    virtual Scripting::Object *getSelf() const { return self; }
    virtual void run( Character * ch, const DLString & );

    virtual bool saveCommand() const;
    
    virtual void backup();
private:
    XML_VARIABLE XMLRegister func;
    Scripting::Object *self;
};

/**
 * A wrapper around WrappedCommand that covers most of the commands in the world.
 * One can override 'run' method of a command from Fenia.
 */
class FeniaCommandWrapper : public PluginWrapperImpl<FeniaCommandWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaCommandWrapper> Pointer;
    
    FeniaCommandWrapper();

    virtual void setSelf( Scripting::Object * );
    void setTarget( WrappedCommand * );
    void checkTarget( ) const ;
    virtual void extract( bool );
    WrappedCommand *getTarget( ) const;

private:        
    WrappedCommand *target;
};

#endif
