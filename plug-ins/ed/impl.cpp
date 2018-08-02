/* $Id: impl.cpp,v 1.1.2.2.6.1 2007/09/11 00:25:03 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "plugin.h"
#include "class.h"

#include "xmlpcstringeditor.h"
#include "xmlattributeplugin.h"
#include "xmleditorinputhandler.h"

extern "C"
{
    SO::PluginList initialize_coreed( )
    {
        SO::PluginList ppl;

	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeEditorState> >( ppl );
	Plugin::registerPlugin<InputHandlerRegistrator<XMLEditorInputHandler> >( ppl );

	return ppl;
    }
}

