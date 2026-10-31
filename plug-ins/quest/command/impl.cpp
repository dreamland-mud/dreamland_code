/* $Id: impl.cpp,v 1.1.4.7.6.4 2009/02/07 18:32:01 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "mocregistrator.h"

#include "cquest.h"
#include "personalnamerepair.h"
#include "questmaster.h"
#include "questor.h"
#include "questscrollhook.h"
#include "questtypesplugin.h"

class Object;
class PCharacter;
Object * fenia_quest_scroll( PCharacter *client );

/** Plugs the skill scroll into quest_core while this library is loaded. */
class QuestScrollHookPlugin : public Plugin {
public:
    typedef ::Pointer<QuestScrollHookPlugin> Pointer;

    virtual void initialization( )
    {
        quest_scroll_hook = &fenia_quest_scroll;
    }

    virtual void destruction( )
    {
        quest_scroll_hook = 0;
    }
};

extern "C"
{
    SO::PluginList initialize_quest_command( )
    {
        SO::PluginList ppl;
        
        // Here and not in quest_core: see questtypesplugin.h.
        Plugin::registerPlugin<QuestTypesPlugin>( ppl );
        Plugin::registerPlugin<CQuest>( ppl );
        Plugin::registerPlugin<PersonalNameRepair>( ppl );

        Plugin::registerPlugin<MobileBehaviorRegistrator<Questor> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestScrollBehavior> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<QuestMaster> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<DefaultQuestMaster> >( ppl );
        Plugin::registerPlugin<QuestScrollHookPlugin>( ppl );
        
        return ppl;
    }
}

