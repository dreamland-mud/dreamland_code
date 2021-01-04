/* $Id: impl.cpp,v 1.1.2.5 2008/03/26 10:57:28 rufina Exp $
 *
 * ruffina, 2006
 */

#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"
#include "commandtemplate.h"
#include "defaultwearlocation.h"
#include "misc_wearlocs.h"

TABLE_LOADER(WearlocationLoader, "wearlocations", "Wearlocation");

#define _WEARLOC_( name ) WearlocationReference wear_##name( #name );
#include "wearloc_utils.h"

extern "C"
{
        SO::PluginList initialize_wearlocation( )
        {
            SO::PluginList ppl;
            
            Plugin::registerPlugin<MocRegistrator<DefaultWearlocation> >( ppl );
            Plugin::registerPlugin<MocRegistrator<StuckInWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<HairWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<TailWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<ShieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<SheathWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<WieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<HorseWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<SecondWieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<TattooWearloc> >( ppl );

            Plugin::registerPlugin<WearlocationLoader>( ppl );
            
            return ppl;
        }
        
}

