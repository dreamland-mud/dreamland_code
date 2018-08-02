/* $Id: impl.cpp,v 1.1.4.1 2005/04/27 03:30:52 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "questmanager.h"
#include "xmlattributequestdata.h"

extern "C"
{
	SO::PluginList initialize_quest_core( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<QuestManager>( ppl );
		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeQuestData> >( ppl );
		
		return ppl;
	}
	
}
