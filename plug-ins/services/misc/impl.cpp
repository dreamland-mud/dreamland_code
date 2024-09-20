/* $Id: impl.cpp,v 1.1.4.2.6.4 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "mocregistrator.h"
#include "commandtemplate.h"

#include "smithman.h"

extern "C"
{
    SO::PluginList initialize_services_misc( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<HorseshoeSmithService> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AlignSmithService> >( ppl );
        Plugin::registerPlugin<MocRegistrator<BurnproofSmithService> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SharpSmithService> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<Smithman> >( ppl );

        return ppl;
    }
}
