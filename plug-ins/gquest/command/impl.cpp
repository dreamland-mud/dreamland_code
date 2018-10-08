/* $Id: impl.cpp,v 1.1.2.1 2005/09/10 21:13:00 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "cgquest.h"
#include "gquestnotifyplugin.h"


extern "C"
{
    SO::PluginList initialize_gquest_command( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<CGQuest>( ppl );
        Plugin::registerPlugin<GQuestNotifyPlugin>( ppl );
        return ppl;
    }
}
