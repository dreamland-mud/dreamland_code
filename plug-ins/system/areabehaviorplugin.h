/* $Id: areabehaviorplugin.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef AREABEHAVIORPLUGIN_H
#define AREABEHAVIORPLUGIN_H

#include "plugin.h"
#include "class.h"

class AreaBehaviorPlugin : public Plugin {
public:
    typedef ::Pointer<AreaBehaviorPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString& getName( ) const = 0;
};

template<typename C>
class AreaBehaviorRegistrator: public AreaBehaviorPlugin {
public:
    typedef ::Pointer< AreaBehaviorRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
	Class::regMoc<C>( );
	AreaBehaviorPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
	AreaBehaviorPlugin::destruction( );
	Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
	return C::MOC_TYPE;
    }
};

#endif
