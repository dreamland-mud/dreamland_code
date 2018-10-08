/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "plugin.h"
#include "mocregistrator.h"
#include "xmlvariableregistrator.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "bugtracker.h"
#include "helpcontainer.h"
#include "markuphelparticle.h"

TABLE_LOADER(HelpLoader, "helps", "Help");

extern "C" {
    
    SO::PluginList initialize_help( ) 
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<BugTracker>( ppl );
	Plugin::registerPlugin<XMLVariableRegistrator<XMLMarkupHelpArticle> >( ppl );
	Plugin::registerPlugin<MocRegistrator<HelpContainer> >( ppl );		
	Plugin::registerPlugin<HelpLoader>( ppl );

	return ppl;
    }
}


