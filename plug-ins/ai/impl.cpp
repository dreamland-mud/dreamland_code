/* $Id: impl.cpp,v 1.1.2.2 2005/11/26 16:59:51 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mobilebehaviorplugin.h"

#include "basicmobilebehavior.h"

extern "C"
{
    SO::PluginList initialize_ai( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<BasicMobileBehavior> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<BasicMobileDestiny> >( ppl );

        return ppl;
    }        
}
