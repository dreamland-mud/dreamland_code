/* $Id: impl_stealquest.cpp,v 1.1.2.3.6.1 2007/09/29 19:34:09 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"

#include "stealquest.h"
#include "objects.h"
#include "mobiles.h"

extern "C"
{
	SO::PluginList initialize_quest_steal( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<StealQuestRegistrator>( ppl );
		Plugin::registerPlugin<ObjectBehaviorRegistrator<HiddenChest> >( ppl );
		Plugin::registerPlugin<ObjectBehaviorRegistrator<LockPick> >( ppl );
		Plugin::registerPlugin<ObjectBehaviorRegistrator<RobbedItem> >( ppl );
		Plugin::registerPlugin<MobileBehaviorRegistrator<RobbedVictim> >( ppl );
		Plugin::registerPlugin<MobileBehaviorRegistrator<Robber> >( ppl );
		
		return ppl;
	}
}
