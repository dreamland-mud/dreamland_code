/* $Id: impl.cpp,v 1.1.2.3.18.4 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "mocregistrator.h"
#include "mobilebehaviorplugin.h"
#include "commandtemplate.h"

#include "cmlt.h"
#include "fixremort.h"
#include "remortnanny.h"
#include "lifeprice.h"
#include "remortbonuses_impl.h"
#include "victorybonus.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_remort( ) {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<CMlt>( ppl );

        Plugin::registerPlugin<RemortNanny>( ppl );
        Plugin::registerPlugin<MocRegistrator<LifePrice> >( ppl );
        Plugin::registerPlugin<MocRegistrator<StatRemortBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<LevelRemortBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PretitleRemortBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HealthRemortBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ManaRemortBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SkillPointRemortBonus> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<RemortWitch> >( ppl );

        Plugin::registerPlugin<MocRegistrator<VictoryPrice> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<Koschey> >( ppl );
        
        Plugin::registerPlugin<FixRemortListener>( ppl );

        return ppl;
    }
}
