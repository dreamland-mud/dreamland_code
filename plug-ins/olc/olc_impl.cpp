/* $Id$
 *
 * ruffina, 2004
 */

#include "olc.h"
#include "aedit.h"
#include "hedit.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "fedit.h"
#include "sedit.h"
#include "eeedit.h"
#include "reledit.h"
#include "skedit.h"
#include "raceedit.h"
#include "olcstate.h"
#include "onlinecreation.h"
#include "security.h"

#include "dlscheduler.h"
#include "schedulertaskroundplugin.h"
#include "plugininitializer.h"
#include "logstream.h"
#include "so.h"
#include "plugin.h"
#include "xmlattributeplugin.h"
#include "mocregistrator.h"
#include "stringset.h"
#include "wiznet.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

bool save_xmlarea(struct area_file *af, Character *ch);

// Automatically save all areas marked as 'changed' every minute.
class SaveChangedAreasTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<SaveChangedAreasTask> Pointer;

    virtual void run( ) 
    {
        StringSet savedAreas;

        for(auto &pArea: areaIndexes) {
            if (pArea->changed) {
                if (save_xmlarea(pArea->area_file, 0))
                    savedAreas.insert(pArea->area_file->file_name);
            }
        }

        if (!savedAreas.empty()) {
            wiznet(WIZ_SECURE, 0, 0, "Autosaved %d area(s): %s", savedAreas.size(), savedAreas.toString().c_str());
        }
    }

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_MINUTE, Pointer( this ) );    
    }

    virtual int getPriority( ) const
    {
        return SCDP_IOWRITE + 10;
    }
};

extern "C"
{
    SO::PluginList initialize_olc( )
    {
        SO::PluginList ppl;
        
        OnlineCreation::registerPlugin(ppl);
        Plugin::registerPlugin<OLCInterpretLayer>( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateReligion> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRace> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSkill> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRoom> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateArea> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateHelp> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateMobile> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateObject> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateExtraExit> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateFile> >( ppl );
        Plugin::registerPlugin<MocRegistrator<XMLVnumRange> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeOLC> >( ppl );
        Plugin::registerPlugin<SaveChangedAreasTask>( ppl );

        return ppl;
    }
}
