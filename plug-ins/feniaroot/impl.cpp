/* $Id: impl.cpp,v 1.1.4.6.6.3 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "wrappermanager.h"
#include "wrappersplugin.h"
#include "xmlattributecodesource.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_feniaroot( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<WrapperManager>( ppl );
	Plugin::registerPlugin<WrappersPlugin>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCodeSource> >( ppl );
	
	return ppl;
    }
}


