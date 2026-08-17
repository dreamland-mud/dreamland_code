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
#include "feniaquest.h"
#include "areaquestcleanupplugin.h"

// Backs up / recovers the generic FeniaQuest quest attribute across a plugin
// reload. FeniaQuest is the attribute every <engine>fenia</engine> type stores
// (createQuest -> createFeniaQuest), regMoc'd by QuestManager so pfiles load it.
// But the per-type C++ registrators back up only getType()==QT::MOC_TYPE (e.g.
// "KillQuest"); nothing has getName()=="FeniaQuest", so a FeniaQuest attribute
// was never stubbed before quest_core's dlclose. On `plug reload most` that left
// a dangling attribute, and a later plugin's XMLAttributePlugin::destruction
// segfaulted calling its virtual getType() (the crash was in religion, 2026-08).
// This plugin only backs up/recovers (base XMLAttributePlugin behaviour) -- it
// does NO regMoc, leaving that to QuestManager, so there is no double registration.
class FeniaQuestAttributePlugin : public XMLAttributePlugin {
public:
        virtual const DLString& getName( ) const { return FeniaQuest::MOC_TYPE; }
};

extern "C"
{
        SO::PluginList initialize_quest_core( )
        {
                SO::PluginList ppl;

                Plugin::registerPlugin<QuestManager>( ppl );
                Plugin::registerPlugin<FeniaQuestAttributePlugin>( ppl );

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
