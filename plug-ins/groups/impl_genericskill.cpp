/* $Id: impl_genericskill.cpp,v 1.1.2.1 2007/06/26 07:15:13 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mocregistrator.h"
#include "genericskill.h"

extern "C"
{
	SO::PluginList initialize_genericskill( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<MocRegistrator<GenericSkill> >( ppl );
		
		return ppl;
	}
	
}
