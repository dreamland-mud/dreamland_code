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
	Plugin::registerPlugin<MocRegistrator<PocketsQuestArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<KeyringQuestArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<PersonalQuestArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<OwnerQuestArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<PiercingQuestArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<TattooQuestArticle> >( ppl );

	Plugin::registerPlugin<MobileBehaviorRegistrator<QuestTrader> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<Questor> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestScrollBehavior> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<QuestMaster> >( ppl );
	
	return ppl;
    }
}

