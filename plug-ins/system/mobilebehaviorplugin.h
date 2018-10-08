/* $Id: mobilebehaviorplugin.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef MOBILEBEHAVIORPLUGIN_H
#define MOBILEBEHAVIORPLUGIN_H

#include "plugin.h"
#include "class.h"

class MobileBehaviorPlugin : public Plugin {
public:
    typedef ::Pointer<MobileBehaviorPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString& getName( ) const = 0;
};

template<typename C>
class MobileBehaviorRegistrator: public MobileBehaviorPlugin {
public:
    typedef ::Pointer< MobileBehaviorRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
        MobileBehaviorPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        MobileBehaviorPlugin::destruction( );
        Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
        return C::MOC_TYPE;
    }
};

#endif
