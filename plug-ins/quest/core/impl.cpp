/* $Id: impl.cpp,v 1.1.4.1 2005/04/27 03:30:52 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "questmanager.h"
#include "questtargets.h"
#include "xmlattributequestdata.h"
#include "areaquestcleanupplugin.h"

extern "C"
{
        SO::PluginList initialize_quest_core( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<QuestManager>( ppl );

                // One mob-side and one object-side target for every Fenia quest
                // type, in place of the seventeen specialized behaviors the eight
                // C++ types register between them. Registered here rather than per
                // type because the type is now data, not a class.
                Plugin::registerPlugin<MobileBehaviorRegistrator<MobQuestTarget> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<ObjQuestTarget> >( ppl );
                Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeQuestData> >( ppl );
                Plugin::registerPlugin<AreaQuestCleanupPlugin>(ppl);

                return ppl;
        }
        
}
