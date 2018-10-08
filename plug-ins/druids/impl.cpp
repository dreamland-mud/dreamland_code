/* $Id: impl.cpp,v 1.1.2.2 2010-09-01 08:21:32 rufina Exp $
 *
 * ruffina, 2009
 */

#include "so.h"
#include "mocregistrator.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"

#include "druidic.h"
#include "druidic_effects.h"
#include "class_druid.h"


extern "C"
{
    SO::PluginList initialize_druids( ) 
    {
        SO::PluginList ppl;
                
        Plugin::registerPlugin<MobileBehaviorRegistrator<DruidSummonedAnimal> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<DruidStaff> >( ppl );

        Plugin::registerPlugin<DruidicLanguage>( ppl );

        Plugin::registerPlugin<MocRegistrator<DruidSpiritAffectHandler> >( ppl );                
/*        
        Plugin::registerPlugin<ObjectBehaviorRegistrator<AnimalSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<SnakeSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<FoxSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<BoarSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<WolverineSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ForestFaerySpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ForestTrollSpiritComponent> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<DryadSpiritComponent> >( ppl );
*/
        return ppl;
    }
}

