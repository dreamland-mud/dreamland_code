/* $Id: impl.cpp,v 1.1.2.1 2005/09/07 19:56:26 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "plugin.h"

extern "C"
{
    SO::PluginList initialize_ed( )
    {
	SO::PluginList ppl;
	return ppl;
    }
}
