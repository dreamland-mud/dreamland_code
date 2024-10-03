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
