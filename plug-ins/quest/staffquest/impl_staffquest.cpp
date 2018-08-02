/* $Id: impl_staffquest.cpp,v 1.1.2.5.6.1 2007/09/29 19:34:07 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mocregistrator.h"
#include "objectbehaviorplugin.h"

#include "staffquest.h"
#include "staffbehavior.h"

extern "C"
{
	SO::PluginList initialize_quest_staff( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<ObjectBehaviorRegistrator<StaffBehavior> >( ppl );
		Plugin::registerPlugin<MocRegistrator<StaffScenario> >( ppl );
		Plugin::registerPlugin<StaffQuestRegistrator>( ppl );
		
		return ppl;
	}
}
