/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"

#include "misc_desires.h"

TABLE_LOADER(DesireLoader, "desires", "Desire");

extern "C"
{
    SO::PluginList initialize_desire( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<BloodlustDesire> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ThirstDesire> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HungerDesire> >( ppl );
        Plugin::registerPlugin<MocRegistrator<FullDesire> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DrunkDesire> >( ppl );
        Plugin::registerPlugin<DesireLoader>( ppl );
        
        return ppl;
    }
}

