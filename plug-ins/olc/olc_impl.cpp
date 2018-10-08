/* $Id$
 *
 * ruffina, 2004
 */

#include "merc.h"
#include "vnum.h"

#include "olc.h"
#include "aedit.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "sedit.h"
#include "tedit.h"
#include "eeedit.h"
#include "olcstate.h"
#include "onlinecreation.h"
#include "security.h"

#include "logstream.h"
#include "so.h"
#include "plugin.h"

#include "mercdb.h"

#include "def.h"


extern "C"
{
    SO::PluginList initialize_olc( )
    {
        SO::PluginList ppl;
        
        OnlineCreation::registerPlugin(ppl);
        Plugin::registerPlugin<XMLAttributeOLCPlugin>( ppl );
        Plugin::registerPlugin<OLCInterpretLayer>( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRoom> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateArea> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateMobile> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateObject> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateExtraExit> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateTrap> >( ppl );

        return ppl;
    }
}
