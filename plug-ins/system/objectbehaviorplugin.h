/* $Id: objectbehaviorplugin.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef OBJECTBEHAVIORPLUGIN_H
#define OBJECTBEHAVIORPLUGIN_H

#include "plugin.h"
#include "class.h"

class ObjectBehaviorPlugin : public Plugin {
public:
    typedef ::Pointer<ObjectBehaviorPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString& getName( ) const = 0;
};

template<typename C>
class ObjectBehaviorRegistrator: public ObjectBehaviorPlugin {
public:
    typedef ::Pointer< ObjectBehaviorRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
        ObjectBehaviorPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        ObjectBehaviorPlugin::destruction( );
        Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
        return C::MOC_TYPE;
    }
};

#endif
