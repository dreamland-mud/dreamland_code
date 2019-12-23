/* $Id$
 *
 * ruffina, 2004
 */

#include "merc.h"
#include "vnum.h"

#include "olc.h"
#include "aedit.h"
#include "hedit.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "sedit.h"
#include "eeedit.h"
#include "reledit.h"
#include "olcstate.h"
#include "onlinecreation.h"
#include "security.h"

#include "logstream.h"
#include "so.h"
#include "plugin.h"
#include "xmlattributeplugin.h"
#include "mocregistrator.h"

#include "mercdb.h"

#include "def.h"


extern "C"
{
    SO::PluginList initialize_olc( )
    {
        SO::PluginList ppl;
        
        OnlineCreation::registerPlugin(ppl);
        Plugin::registerPlugin<OLCInterpretLayer>( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateReligion> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRoom> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateArea> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateHelp> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateMobile> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateObject> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateExtraExit> >( ppl );
        Plugin::registerPlugin<MocRegistrator<XMLVnumRange> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeOLC> >( ppl );

        return ppl;
    }
}
