/* $Id: impl.cpp,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "xmlattributeplugin.h"

#include "invasioninfo.h"
#include "xmlattributeinvasion.h"
#include "mobiles.h"
#include "objects.h"
#include "scenarios.h"

extern "C"
{
    SO::PluginList initialize_gquest_invasion( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeInvasion> >( ppl );

        Plugin::registerPlugin<MobileBehaviorRegistrator<InvasionMob> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<InvasionHelper> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<InvasionObj> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<InvasionInstrument> >( ppl );

        Plugin::registerPlugin<InvasionScenarioRegistrator<InvasionDenseScenario> >( ppl );
        Plugin::registerPlugin<InvasionScenarioRegistrator<InvasionSparseScenario> >( ppl );
        Plugin::registerPlugin<InvasionScenarioRegistrator<InvasionLocustScenario> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<InvasionBubblesMob> >( ppl );
        Plugin::registerPlugin<InvasionScenarioRegistrator<InvasionFootballScenario> >( ppl );

        Plugin::registerPlugin<InvasionGQuestInfo>( ppl );
        return ppl;
    }
}

