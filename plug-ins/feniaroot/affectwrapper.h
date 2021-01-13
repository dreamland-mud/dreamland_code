/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __AFFECTWRAPPER_H__
#define __AFFECTWRAPPER_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlshort.h"
#include "xmlglobalbitvector.h"
#include "affect.h"

#include "lex.h"
#include "scope.h"
#include "exceptions.h"
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

using Scripting::NativeHandler;

class AffectWrapper : public PluginNativeImpl<AffectWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AffectWrapper> Pointer;

    AffectWrapper() { }
    AffectWrapper(const RegisterList &);
    AffectWrapper(Affect &);
            
    virtual void setSelf(Scripting::Object *) { }
    inline Affect& getTarget() { return target; }

    static Scripting::Register wrap( const Affect & );

protected:
    XML_VARIABLE Affect target;
};

#endif
