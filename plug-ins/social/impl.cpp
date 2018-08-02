/* $Id: impl.cpp,v 1.1.2.1.6.4 2008/03/26 10:57:28 rufina Exp $
 *
 * ruffina, 2004
 */

#include "xmlattributeplugin.h"
#include "commandtemplate.h"

#include "socialmanager.h"
#include "social.h"
#include "customsocial.h"
#include "mysocial.h"
#include "so.h"

#include <stdio.h>

extern "C"
{
    SO::PluginList initialize_social( )
    {
	SO::PluginList ppl;
	Plugin::registerPlugin<SocialManager>( ppl );
	Plugin::registerPlugin<CustomSocialManager>( ppl );
	Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeCustomSocials> >( ppl );
	Plugin::registerPlugin<MySocial>( ppl );
	
	return ppl;
    }
}
