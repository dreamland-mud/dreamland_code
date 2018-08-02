/* $Id: impl.cpp,v 1.1.2.3 2005/07/30 14:50:07 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"

#include "mkey.h"
#include "homerecall.h"

extern "C"
{
    SO::PluginList initialize_mansion( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<MKey>( ppl );
	Plugin::registerPlugin<HomeRecall>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeHomeRecall> >( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeMansionKey> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<MansionKeyMaker> >( ppl );
	    
	return ppl;
    }
}
