/* $Id: root.h,v 1.1.4.8.6.3 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __ROOT_H__
#define __ROOT_H__

#include "xmlvariablecontainer.h"
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
#include "xmlregister.h"
#include "pluginnativeimpl.h"

using Scripting::XMLRegister;
using Scripting::NativeHandler;

class Root : public PluginNativeImpl<Root>, 
             public NativeHandler, 
             public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    
    Root() { }

    virtual void setSelf(Scripting::Object *s) {
        self = s;
    }
    virtual Scripting::Object *getSelf() const { return self; }
   
    XML_VARIABLE XMLRegister tmp, scheduler, tables, nanny;
private:
    Scripting::Object *self;
};

#endif /* __ROOT_H__ */
