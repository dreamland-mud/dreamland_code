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
#include "socedit.h"
#include "cmdedit.h"
#include "qedit.h"
#include "bedit.h"
#include "olcstate.h"
#include "onlinecreation.h"
#include "security.h"

#include "dreamland.h"
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

#include "merc.h"
#include "def.h"

bool save_xmlarea(struct area_file *af, Character *ch);
void save_xmlarea_list( );

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

/*
 * Round-trip gate: with the 'roundtrip' boot option the server loads the
 * world, writes every area file back once and shuts down. A diff against
 * the loaded files shows what the serializer does not give back. Runs at
 * boot only, after the areas are loaded and fixed (SCDP_BOOT + 5, + 6) and
 * the item economy pass (+ 7), before players and saved rooms (+ 10, + 20).
 */
class RoundTripSaveTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<RoundTripSaveTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis( )->getCurrentTick( ) != 0)
            return;

        if (!dreamland->hasOption( DL_ROUNDTRIP ))
            return;

        int total = 0, failed = 0;

        save_xmlarea_list( );

        // One broken area must not stop the rest: the gate wants every file.
        for (struct area_file *af = area_file_list; af; af = af->next) {
            total++;
            try {
                if (!save_xmlarea( af, 0 ))
                    failed++;
            } catch (const std::exception &e) {
                LogStream::sendError( )
                    << "Round trip: " << af->file_name << ": " << e.what( ) << endl;
                failed++;
            }
        }

        LogStream::sendNotice( )
            << "Round trip: saved " << (total - failed) << " of " << total
            << " area files, " << failed << " failed. Shutting down." << endl;

        dreamland->shutdown( );
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 8;
    }
};

extern "C"
{
    SO::PluginList initialize_olc( )
    {
        SO::PluginList ppl;
        
        OnlineCreation::registerPlugin(ppl);
        Plugin::registerPlugin<OLCInterpretLayer>( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateBehavior> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateAreaQuest> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSkillGroup> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateCommand> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSocial> >( ppl );
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
        Plugin::registerPlugin<RoundTripSaveTask>( ppl );

        return ppl;
    }
}
