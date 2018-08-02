/* $Id: impl_butcherquest.cpp,v 1.1.2.5.6.1 2007/09/29 19:33:49 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"

#include "butcherquest.h"
#include "steakcustomer.h"

extern "C"
{
	SO::PluginList initialize_quest_butcher( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<MobileBehaviorRegistrator<SteakCustomer> >( ppl );
		Plugin::registerPlugin<ButcherQuestRegistrator>( ppl );
		
		return ppl;
	}
}
