/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "xmlvariableregistrator.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "mocregistrator.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"

#include "templeman.h"
#include "gods_impl.h"
#include "tattoo.h"
#include "defaultreligion.h"
#include "religionattribute.h"

TABLE_LOADER(ReligionLoader, "religions", "Religion");

extern "C"
{
    SO::PluginList initialize_religion( ) 
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<MobileBehaviorRegistrator<Templeman> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ReligionTattoo> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<ReligionHelp> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeReligion> >( ppl );
        
        Plugin::registerPlugin<MocRegistrator<DefaultReligion> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AtumRaGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ZeusGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SiebeleGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AhuramazdaGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ShamashGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<EhrumenGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<VenusGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SethGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<OdinGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PhobosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<TeshubGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AresGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HeraGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DeimosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ErosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<EnkiGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<GoktengriGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<BastGod> >( ppl );
        
        Plugin::registerPlugin<ReligionLoader>( ppl );

        return ppl;
    }
}

