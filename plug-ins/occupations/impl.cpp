/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "xmlattributeplugin.h"
#include "commandtemplate.h"

#include "attract.h"

extern "C"
{
    SO::PluginList initialize_occupations( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeAttract> >( ppl );

        return ppl;
    }
}
