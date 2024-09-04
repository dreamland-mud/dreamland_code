/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "schedulertaskroundplugin.h"
#include "mocregistrator.h"
#include "dlscheduler.h"
#include "objectbehaviorplugin.h"
#include "mobilebehaviorplugin.h"
#include "pcharactermanager.h"
#include "objectbehaviormanager.h"
#include "xmlattributeareaquest.h"
#include "xmlattributeplugin.h"
#include "defaultbehavior.h"

#include "save.h"
#include "merc.h"
#include "def.h"

void load_creatures( );
void limit_purge( );

class PlayerLoadTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<PlayerLoadTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis()->getCurrentTick( ) == 0) 
            PCharacterManager::loadPlayers( );
    }
    
    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 10;
    }
};

class DropsLoadTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<DropsLoadTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis()->getCurrentTick( ) == 0) {
            load_drops( );
            load_dropped_mobs( );
            load_creatures( );
        }
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 20;
    }
};

class LimitedItemsPurgeTask: public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<LimitedItemsPurgeTask> Pointer;

    virtual void run( ) 
    {
        limit_purge( );
    }
    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_MINUTE, Pointer( this ) );    
    }
    virtual int getPriority( ) const
    {
        return SCDP_ROUND + 90;
    }
};

extern "C" {
    
    SO::PluginList initialize_loadsave( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<PlayerLoadTask>( ppl );
        Plugin::registerPlugin<DropsLoadTask>( ppl );
        Plugin::registerPlugin<LimitedItemsPurgeTask>( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<BasicObjectBehavior> >(ppl);
        Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeAreaQuest> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DefaultBehavior> >(ppl);

        return ppl;
    }
}

