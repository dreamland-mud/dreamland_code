/* $Id: impl.cpp,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "rainbowinfo.h"
#include "mobiles.h"
#include "objects.h"

extern "C"
{
    SO::PluginList initialize_gquest_rainbow( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<RainbowMob> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<RainbowPiece> >( ppl );
        Plugin::registerPlugin<RainbowScenarioRegistrator<RainbowDefaultScenario> >( ppl );
        Plugin::registerPlugin<RainbowScenarioRegistrator<RainbowSinsScenario> >( ppl );
        Plugin::registerPlugin<RainbowGQuestInfo>( ppl );

        return ppl;
    }
}
