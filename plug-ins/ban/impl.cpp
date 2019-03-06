/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"

#include "ban.h"

extern "C" {
    
    SO::PluginList initialize_ban( ) 
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<BanManager>( ppl );
        
        return ppl;
    }
}



