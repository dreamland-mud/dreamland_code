/* $Id: impl.cpp,v 1.1.6.7.6.11 2009/08/10 01:06:51 rufina Exp $
 *
 * ruffina, 2004
 */
#include "class.h"
#include "so.h"

#include "objectbehaviorplugin.h"
#include "mobilebehaviorplugin.h"
#include "roombehaviorplugin.h"
#include "areabehaviorplugin.h"
#include "mocregistrator.h"

#include "schedulertaskroundplugin.h"
#include "clantasks.h"
#include "commandtemplate.h"
#include "xmlattributeplugin.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "dlscheduler.h"

#include "defaultclan.h"
#include "battlerager.h"
#include "ruler.h"
#include "knight.h"

TABLE_LOADER(ClanLoader, "clans", "Clan");

/** A task responsible for setting up itemID inside clan data. */
class ClanItemRefreshPlugin : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<ClanItemRefreshPlugin> Pointer;

    virtual int getPriority( ) const
    {
        return SCDP_INITIAL + 10; // Called immediately after the first area update
    }

    virtual void run( )
    {
        LogStream::sendNotice() << "Refreshing clan item IDs:" << endl;
    
        for (Object *obj = object_list; obj; obj = obj->next) {
            if (!obj->behavior)
                continue;

            ClanItem::Pointer clanItem = obj->behavior.getDynamicPointer<ClanItem>();
            if (!clanItem)
                continue;

            if (!clanItem->clan->getData()) {
                warn("...clan item (%lld) w/o clan data for %s", 
                     obj->getID(), clanItem->clan.getName().c_str());

            } else if (obj->in_obj && obj->in_obj->behavior && obj->in_obj->behavior.getDynamicPointer<ClanAltar>()) {
                clanItem->clan->getData()->setItem(obj);
                notice("...assigned item [%d] (%lld) to clan %s", 
                        obj->pIndexData->vnum, obj->getID(), clanItem->clan.getName().c_str());

            } else {
                notice("...clan item [%d] (%lld) for clan %s is elsewhere", 
                        obj->pIndexData->vnum, obj->getID(), clanItem->clan.getName().c_str());
            }
        }
    }
};

extern "C"
{
    SO::PluginList initialize_clan_impl( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<DefaultClan> >( ppl );
    
        /*
         * battlerager
         */
        Plugin::registerPlugin<ObjectBehaviorRegistrator<BattleragerPoncho> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<PersonalBattleragerPoncho> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanHealerBattlerager> >( ppl );

        /*
         * chaos
         */
        
        /*
         * knight
         */
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanItemKnight> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanAltarKnight> >( ppl );
        
        /*
         * lion
         */
        
        /*
         * ruler
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardRulerPre > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardRulerJailer > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<RulerSpecialGuard> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<Stalker> >( ppl );
        
        /*
         * shalafi
         */
        
        /*
         * invader
         */

        /*
         * other guards
         */
        
        /*
         * loader
         */
        Plugin::registerPlugin<ClanLoader>( ppl );
        Plugin::registerPlugin<ClanItemRefreshPlugin>( ppl );
        Plugin::registerPlugin<ClanTenureTask>( ppl );
        Plugin::registerPlugin<ClanDecayTask>( ppl );
        Plugin::registerPlugin<ClanLoginListener>( ppl );

        return ppl;
    }
}
