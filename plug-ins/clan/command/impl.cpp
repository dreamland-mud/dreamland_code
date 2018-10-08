/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributeplugin.h"
#include "commandtemplate.h"

#include "cclan.h"
#include "cclantalk.h"
#include "xmlattributeinduct.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_clan_command( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<CClan>( ppl );
        Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeInduct> >( ppl );
        Plugin::registerPlugin<XMLAttributeInductListenerPlugin>( ppl );

        return ppl;
    }
}
