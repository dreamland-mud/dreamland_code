/* $Id: roombehaviorplugin.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef ROOMBEHAVIORPLUGIN_H
#define ROOMBEHAVIORPLUGIN_H

#include "plugin.h"
#include "class.h"

class RoomBehaviorPlugin : public Plugin {
public:
    typedef ::Pointer<RoomBehaviorPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString& getName( ) const = 0;
};

template<typename C>
class RoomBehaviorRegistrator: public RoomBehaviorPlugin {
public:
    typedef ::Pointer< RoomBehaviorRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
        RoomBehaviorPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        RoomBehaviorPlugin::destruction( );
        Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
        return C::MOC_TYPE;
    }
};

#endif
