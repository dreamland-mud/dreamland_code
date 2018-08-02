/* $Id: plugininitializer.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2008
 */

#ifndef __PLUGININITIALIZER_H__
#define __PLUGININITIALIZER_H__

#include "plugin.h"
#include "initializer.h"
#include "sharedobject.h"

#define INITPRIO_PLUGINS 50

template <typename P>
class PluginInitializer : public Initializer {
public:
    PluginInitializer(int prio = INITPRIO_PLUGINS) : Initializer(prio) {
    }
    virtual ~PluginInitializer( ) {
    }
    
    virtual void init(SharedObject *so) {
        so->plugins.push_back( static_cast<Plugin*>( new P( ) ) );
    }
};

#endif
