/* $Id: impl_kidnapquest.cpp,v 1.1.2.6.6.1 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mocregistrator.h"
#include "objectbehaviorplugin.h"
#include "mobilebehaviorplugin.h"

#include "kidnapquestregistrator.h"
#include "bandit.h"
#include "king.h"
#include "prince.h"
#include "objects.h"
#include "scenario_bidon.h"
#include "scenario_dragon.h"
#include "scenario_urchin.h"
#include "scenario_urka.h"
#include "scenario_cyclop.h"

extern "C"
{
    SO::PluginList initialize_quest_kidnap( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<KidnapDragonScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KidnapBidonScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KidnapUrchinScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KidnapUrkaScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KidnapUrkaPoliteScenario> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KidnapCyclopScenario> >( ppl );
        
        Plugin::registerPlugin<KidnapQuestRegistrator>( ppl );

        Plugin::registerPlugin<MobileBehaviorRegistrator<KidnapKing> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<KidnapPrince> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<KidnapBandit> >( ppl );

        Plugin::registerPlugin<ObjectBehaviorRegistrator<KidnapMark> >( ppl );
                
        return ppl;
    }
}
