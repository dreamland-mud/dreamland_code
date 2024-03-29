/* $Id$
 *
 * ruffina, 2004
 */
#include "descriptor.h"
#include "outofband.h"
#include "so.h"

/*-------------------------------------------------------------------
 * DescriptorPlugin
 *-------------------------------------------------------------------*/
class DescriptorPlugin : public Plugin
{
public:
    virtual void initialization();
    virtual void destruction();

    /** Try to avoid reloading this plugin whenever possible, as it disconnects players. */
    virtual bool isCritical() const { 
        return true; 
    }
};

void
DescriptorPlugin::initialization()
{
    Class::regMoc<InputHandler>();
    Class::regMoc<BufferHandler>();
}

void
DescriptorPlugin::destruction()
{
    while(descriptor_list) {
        descriptor_list->close( );
        descriptor_list->slay( );
    }
    Class::unregMoc<InputHandler>();
    Class::unregMoc<BufferHandler>();
}

extern "C" {
    
    SO::PluginList initialize_descriptor( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<DescriptorPlugin>( ppl );
        Plugin::registerPlugin<OutOfBandManager>( ppl );
        
        return ppl;
    }
}


