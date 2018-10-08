/* $Id: impl_killquest.cpp,v 1.1.2.5.6.1 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "questregistrator.h"

#include "killquest.h"
#include "victimbehavior.h"

extern "C"
{
        SO::PluginList initialize_quest_kill( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<MobileBehaviorRegistrator<VictimBehavior> >( ppl );
                Plugin::registerPlugin<QuestRegistrator<KillQuest> >( ppl );
                
                return ppl;
        }
}
