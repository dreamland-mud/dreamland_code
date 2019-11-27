/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "plugin.h"
#include "mocregistrator.h"
#include "xmlvariableregistrator.h"
#include "bugtracker.h"
#include "helpcontainer.h"
#include "markuphelparticle.h"

extern "C" {
    
    SO::PluginList initialize_help( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<BugTracker>( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<GenericHelp> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HelpContainer> >( ppl );                
        Plugin::registerPlugin<HelpLoader>( ppl );

        return ppl;
    }
}


