/* $Id: impl.cpp,v 1.1.2.3 2010-08-31 14:57:21 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"

#include "elvish_effects.h"
#include "quenia.h"
#include "ahenn.h"
#include "khuzdul.h"
#include "arcadian.h"
#include "arcadian_effects.h"


extern "C"
{
    SO::PluginList initialize_languages_impl( ) 
    {
        SO::PluginList ppl;
                
        Plugin::registerPlugin<ElvishEffectsPlugin>( ppl );
        Plugin::registerPlugin<QueniaLanguage>( ppl );
        Plugin::registerPlugin<AhennLanguage>( ppl );
        Plugin::registerPlugin<KhuzdulLanguage>( ppl );
        Plugin::registerPlugin<ArcadianLanguage>( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ArcadianDrinkBehavior> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<BeerElementalBehavior> >( ppl );

        return ppl;
    }
}

