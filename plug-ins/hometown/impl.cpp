/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"

#include "defaulthometown.h"

TABLE_LOADER(HometownLoader, "hometowns", "Hometown");

extern "C"
{
    SO::PluginList initialize_hometown( ) {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<DefaultHometown> >( ppl );
        Plugin::registerPlugin<HometownLoader>( ppl );
        
        return ppl;
    }
}

