/* $Id: xmlattributeplugin.h,v 1.1.2.1 2007/09/11 00:22:30 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEPLUGIN_H
#define XMLATTRIBUTEPLUGIN_H

#include "plugin.h"
#include "class.h"

class XMLAttributePlugin : public Plugin {
public:
	typedef ::Pointer<XMLAttributePlugin> Pointer;
	
	virtual void initialization( );
	virtual void destruction( );
	virtual const DLString& getName( ) const = 0;
};

template<typename C>
class XMLAttributeRegistrator: public XMLAttributePlugin {
public:
    typedef ::Pointer< XMLAttributeRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
	Class::regMoc<C>( );
	XMLAttributePlugin::initialization( );
    }
    virtual void destruction( ) 
    {
	XMLAttributePlugin::destruction( );
	Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
	return C::MOC_TYPE;
    }
};

template<typename C>
class XMLAttributeVarRegistrator: public XMLAttributePlugin {
public:
    typedef ::Pointer< XMLAttributeRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
	Class::regXMLVar<C>( );
	XMLAttributePlugin::initialization( );
    }
    virtual void destruction( ) 
    {
	XMLAttributePlugin::destruction( );
	Class::unregXMLVar<C>( );
    }
    virtual const DLString& getName( ) const 
    {
	return C::TYPE;
    }
};

#endif
