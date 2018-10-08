/* $Id: impl_locatequest.cpp,v 1.1.2.8.6.1 2007/09/29 19:34:06 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "plugin.h"
#include "mocregistrator.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"

#include "locatequest.h"
#include "objects.h"
#include "mobiles.h"
#include "scenarios.h"
#include "scenarios_impl.h"

extern "C"
{
    SO::PluginList initialize_quest_locate( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<LocateMousesScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<LocateTorturerScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<LocateAlchemistScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<LocateSecretaryScenario> >( ppl );
        
        Plugin::registerPlugin<LocateQuestRegistrator>( ppl );

        Plugin::registerPlugin<ObjectBehaviorRegistrator<LocateItem> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<LocateCustomer> >( ppl );
                
        return ppl;
    }
}
