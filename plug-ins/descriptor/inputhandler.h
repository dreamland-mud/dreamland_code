/* $Id$
 *
 * ruffina, 2004
 */
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "class.h"
#include "plugin.h"
#include "xmlpersistent.h"
#include "xmlvariablecontainer.h"
#include "xmllist.h"

class Descriptor;

/*---------------------------------------------------------------------
 * InputHandler
 *---------------------------------------------------------------------*/
class InputHandler : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<InputHandler> Pointer;
    
    virtual int handle(Descriptor*, char *);
    virtual void prompt(Descriptor*);
    virtual void close( Descriptor * );
};

extern template class XMLStub<InputHandler>;

typedef XMLListBase<XMLPersistent<InputHandler> > handle_input_t;

/*---------------------------------------------------------------------
 * InputHandlerPlugin
 *---------------------------------------------------------------------*/
struct InputHandlerPlugin : public Plugin {
    virtual void initialization();
    virtual void destruction();
    virtual const DLString &getType( ) const = 0;
};

template <typename T>
struct InputHandlerRegistrator : public InputHandlerPlugin {
    typedef ::Pointer<InputHandlerRegistrator<T> > Pointer;

    virtual void initialization() 
    {
        Class::regMoc<T>();
        InputHandlerPlugin::initialization( );
    }
    
    virtual void destruction() 
    {
        InputHandlerPlugin::destruction( );
        Class::unregMoc<T>();
    }

    virtual const DLString &getType( ) const
    {
        return T::MOC_TYPE;
    }
};

#endif
