/* $Id: impl.cpp,v 1.1.2.5.10.2 2008/04/10 00:06:02 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "noteattrs.h"
#include "notemanager.h"
#include "unread.h"
#include "notehooks.h"


extern "C"
{
    SO::PluginList initialize_notes( ) {
	SO::PluginList ppl;
		
	Plugin::registerPlugin<NoteManager>( ppl );
	Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeNoteData> >( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLastRead> >( ppl );
	Plugin::registerPlugin<Unread>( ppl );
	Plugin::registerPlugin<UnreadListener>( ppl );
	Plugin::registerPlugin<NoteHooks>( ppl );
	
	return ppl;
    }
}

