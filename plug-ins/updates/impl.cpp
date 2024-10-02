/* $Id: impl.cpp,v 1.1.2.5 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"

#include "update_params.h"
#include "update_areas.h"
#include "update.h"
#include "weather.h"
#include "profiler.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "so.h"
#include "core/object.h"
#include "npcharacter.h"


#include "merc.h"
#include "def.h"

class AnatoliaUpdate : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<AnatoliaUpdate> Pointer;
    
    virtual void run( )
    {
        update_handler( );
        DLScheduler::getThis( )->putTaskInitiate( Pointer( this ) );
    }
    
    virtual int getPriority( ) const
    {
        return SCDP_AUTO;
    }
};

class InitialUpdate : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<InitialUpdate> Pointer;

    virtual void run( )
    {
        if(DLScheduler::getThis()->getCurrentTick( ) == 0) {
            weather_init( );
            area_update(FRESET_ALWAYS);
        }
    }
    virtual int getPriority( ) const
    {
        return SCDP_INITIAL;
    }
};

class HourlyUpdate : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<HourlyUpdate> Pointer;

    virtual void run()
    {
        ProfilerBlock profiler("hourly", 20);

        Object *obj, *obj_next;
        for (obj = object_list; obj; obj = obj_next) {
            obj_next = obj->next;
            if (obj->behavior)
                obj->behavior->hourly();
        }

        Character *wch, *wch_next;
        for (wch = char_list; wch; wch = wch_next) {
            wch_next = wch->next;
            if (wch->is_npc() && wch->getNPC()->behavior)
                wch->getNPC()->behavior->hourly();
        }
    }

    virtual int getPriority( ) const
    {
        return SCDP_AUTO + 10;
    }

    virtual void after()
    {
        DLScheduler::getThis()->putTaskInSecond(Date::SECOND_IN_HOUR, Pointer(this));
    }
};

extern "C"
{
    SO::PluginList initialize_updates( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<AnatoliaUpdate>( ppl );
        Plugin::registerPlugin<HourlyUpdate>( ppl );
        Plugin::registerPlugin<InitialUpdate>( ppl );
        Plugin::registerPlugin<CharacterParamsUpdateTask>( ppl );

        return ppl;
    }
}

