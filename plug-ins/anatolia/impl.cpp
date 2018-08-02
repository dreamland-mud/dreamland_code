/* $Id: impl.cpp,v 1.1.2.13.6.2 2007/09/11 00:33:51 rufina Exp $
 *
 * ruffina, 2004
 */


#include "plugin.h"
#include "so.h"

#define _GSN_( name ) SkillReference gsn_##name( #name );
#include "gsn_plugin.h"

extern "C"
{
    SO::PluginList initialize_anatolia( )
    {
	SO::PluginList ppl;


	return ppl;
    }
}

