/* $Id: impl.cpp,v 1.1.2.1 2005/09/16 13:10:11 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "commandinterpreter.h"

extern "C"
{
	SO::PluginList initialize_interpret( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<CommandInterpreter>( ppl );
		
		return ppl;
	}
}
