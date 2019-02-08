/* $Id: impl_bigquest.cpp,v 1.1.2.5.6.1 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mocregistrator.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "questregistrator.h"

#include "bigquest.h"
#include "bandamobile.h"

extern "C"
{
        SO::PluginList initialize_quest_big( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<MobileBehaviorRegistrator<BandaMobile> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<BandaItem> >( ppl );
                Plugin::registerPlugin<MocRegistrator<BigQuestScenario> >( ppl );
                Plugin::registerPlugin<BigQuestRegistrator>( ppl );
                
                return ppl;
        }
}
