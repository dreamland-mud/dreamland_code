/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"

#include "groupchannel.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_follow_command( )
    {
        SO::PluginList ppl;
    
        Plugin::registerPlugin<GroupChannel>( ppl );
        return ppl;
    }
}
