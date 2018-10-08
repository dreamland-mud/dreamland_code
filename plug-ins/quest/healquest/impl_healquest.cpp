/* $Id: impl_healquest.cpp,v 1.1.2.5.6.2 2007/10/08 00:56:22 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "mocregistrator.h"

#include "healquest.h"
#include "patientbehavior.h"
#include "scenarios.h"

extern "C"
{
        SO::PluginList initialize_quest_heal( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<MobileBehaviorRegistrator<PatientBehavior> >( ppl );
                Plugin::registerPlugin<MocRegistrator<HealScenario> >( ppl );
                Plugin::registerPlugin<HealQuestRegistrator>( ppl );
                
                return ppl;
        }
}
