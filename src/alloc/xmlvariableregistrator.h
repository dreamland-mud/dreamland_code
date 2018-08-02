/* $Id: xmlvariableregistrator.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2005
 */
#ifndef __XMLVARREGISTRATOR_H__
#define __XMLVARREGISTRATOR_H__

#include "plugin.h"
#include "class.h"

template<typename C>
class XMLVariableRegistrator : public virtual Plugin {
public:
    typedef ::Pointer< XMLVariableRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
	Class::regXMLVar<C>( );
    }
    virtual void destruction( ) 
    {
	Class::unregXMLVar<C>( );
    }
};

#endif
