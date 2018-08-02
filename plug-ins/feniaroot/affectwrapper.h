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
#include "skillreference.h"

#include "lex.h"
#include "scope.h"
#include "exceptions.h"
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

using Scripting::NativeHandler;

class Affect;

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
	    
    virtual void setSelf(Scripting::Object *) { }
    
    void fromAffect( const Affect & );
    void toAffect( Affect & );
    static Scripting::Register wrap( const Affect & );

    XML_VARIABLE XMLInteger bitvector;
    XML_VARIABLE XMLShort where, level, duration, modifier, location;
    XML_VARIABLE XMLSkillReference type;
    XML_VARIABLE XMLGlobalBitvector global;
};

#endif
