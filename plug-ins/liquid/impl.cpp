/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "objectbehaviorplugin.h"
#include "mocregistrator.h"
#include "commandtemplate.h"

#include "defaultliquid.h"
#include "drinkcontainer.h"
#include "drink_commands.h"

TABLE_LOADER(LiquidLoader, "liquids", "Liquid");

extern "C"
{
    SO::PluginList initialize_liquid( ) 
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<ObjectBehaviorRegistrator<DrinkContainer> >( ppl );
	Plugin::registerPlugin<MocRegistrator<DefaultLiquid> >( ppl );
	Plugin::registerPlugin<LiquidLoader>( ppl );

	Plugin::registerPlugin<CPour>( ppl );
	
	return ppl;
    }
}

