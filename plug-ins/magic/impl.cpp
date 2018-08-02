/* $Id: impl.cpp,v 1.1.2.5.18.5 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "scribing.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_magic( )
    {
	SO::PluginList ppl;

	Plugin::registerPlugin<ObjectBehaviorRegistrator<SpellBook> >( ppl );
	
	return ppl;
    }
}
