/* $Id$
 *
 * ruffina, 2004
 */
#include "descriptor.h"
#include "so.h"

/*-------------------------------------------------------------------
 * DescriptorPlugin
 *-------------------------------------------------------------------*/
class DescriptorPlugin : public Plugin
{
public:
    virtual void initialization();
    virtual void destruction();
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
	
	return ppl;
    }
}


