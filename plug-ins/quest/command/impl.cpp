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
#include "questtrader.h"
#include "questmaster.h"
#include "questor.h"
#include "xmlattributequestreward.h"
#include "questscrollhook.h"
#include "pcharacter.h"
#include "wiznet.h"
#include "merc.h"

#include <sstream>

Object * make_quest_scroll( PCharacter *client, std::ostringstream &report );

/** A scroll for Fenia (tier loot drops): same roll as the questor's reward. */
static Object * fenia_quest_scroll( PCharacter *client )
{
    std::ostringstream report;
    Object *scroll = make_quest_scroll( client, report );
    if (scroll)
        ::wiznet( WIZ_QUEST, 0, 0, "%^C1 получает свиток познания (добыча) для умения %s", client, report.str().c_str() );
    return scroll;
}

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
        
        Plugin::registerPlugin<CQuest>( ppl );
        Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeQuestReward> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ObjectQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ConQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<GoldQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PracticeQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PocketsQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<KeyringQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PersonalQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<OwnerQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PiercingQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<WearslotQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<RefitQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<UpgradeQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<TattooQuestArticle> >( ppl );
        Plugin::registerPlugin<MocRegistrator<VaultQuestArticle> >( ppl );
        Plugin::registerPlugin<PersonalNameRepair>( ppl );

        Plugin::registerPlugin<MobileBehaviorRegistrator<QuestTrader> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<Questor> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestScrollBehavior> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<QuestMaster> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<DefaultQuestMaster> >( ppl );
        Plugin::registerPlugin<QuestScrollHookPlugin>( ppl );
        
        return ppl;
    }
}

