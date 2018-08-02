/* $Id: mlove_impl.cpp,v 1.1.2.3.28.5 2008/03/26 10:57:27 rufina Exp $
 * ruffina, 2003
 */

#include "commandplugin.h"
#include "xmlattributeplugin.h"
#include "commandtemplate.h"

#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"
#include "lover.h"
#include "marry.h"
#include "divorce.h"
#include "so.h"

extern "C"
{
	SO::PluginList initialize_mlove( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<Lover>( ppl );
		Plugin::registerPlugin<Marry>( ppl );
		Plugin::registerPlugin<Divorce>( ppl );

		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLovers> >( ppl );
		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeMarriage> >( ppl );
		
		return ppl;
	}
}
		
