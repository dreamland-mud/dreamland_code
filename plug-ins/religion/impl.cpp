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
#include "xmlattributeplugin.h"
#include "descriptorstatelistener.h"
#include "descriptor.h"
#include "core/object.h"
#include "pcharacter.h"
#include "wearloc_utils.h"
#include "loadsave.h"
#include "wiznet.h"
#include "def.h"

#include "gods_impl.h"
#include "tattoo.h"
#include "defaultreligion.h"
#include "religionattribute.h"

RELIG(none);

extern "C"
{
    SO::PluginList initialize_religion( ) 
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<ObjectBehaviorRegistrator<ReligionTattoo> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<ReligionHelp> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeReligion> >( ppl );
        
        Plugin::registerPlugin<MocRegistrator<DefaultReligion> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ErevanGod> >( ppl );
        Plugin::registerPlugin<ReligionLoader>( ppl );

        return ppl;
    }
}

