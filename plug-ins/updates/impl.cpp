/* $Id: impl.cpp,v 1.1.2.5 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"

#include "update_params.h"
#include "update.h"
#include "weather.h"

#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "so.h"

#include "mercdb.h"
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
	    area_update( );
	}
    }
    virtual int getPriority( ) const
    {
	return SCDP_INITIAL;
    }
};

extern "C"
{
    SO::PluginList initialize_updates( )
    {
	SO::PluginList ppl;

	Plugin::registerPlugin<AnatoliaUpdate>( ppl );
	Plugin::registerPlugin<InitialUpdate>( ppl );
	Plugin::registerPlugin<CharacterParamsUpdateTask>( ppl );

	return ppl;
    }
}

