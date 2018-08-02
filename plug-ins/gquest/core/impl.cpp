/* $Id: impl.cpp,v 1.1.2.1.6.1 2007/06/26 07:13:58 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "xmlattributeglobalquest.h"
#include "globalquestmanager.h"
#include "xmlattributereward.h"


extern "C"
{
    SO::PluginList initialize_gquest_core( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeGlobalQuest> >( ppl );
	Plugin::registerPlugin<GlobalQuestManager>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeReward> >( ppl );
	Plugin::registerPlugin<XMLAttributeRewardListenerPlugin>( ppl );
	return ppl;
    }
}
