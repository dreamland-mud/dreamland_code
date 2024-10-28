/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "mobilebehaviorplugin.h"
#include "commandtemplate.h"

#include "shoptrader.h"

extern "C"
{
    SO::PluginList initialize_services_shop( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<ShopTrader> >( ppl );
        
        return ppl;
    }
}
