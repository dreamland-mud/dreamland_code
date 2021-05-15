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
#include "commandtemplate.h"
#include "xmlattributeplugin.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "dlscheduler.h"

#include "battlerager.h"
#include "shalafi.h"
#include "chaos.h"
#include "ruler.h"
#include "invader.h"
#include "knight.h"
#include "hunter.h"
#include "lion.h"
#include "outsider.h"
#include "ghost.h"
#include "flowers.h"

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
        
        Plugin::registerPlugin<MocRegistrator<KnightOrder> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DefaultClan> >( ppl );
    
        /*
         * hunter
         */
        Plugin::registerPlugin<AreaBehaviorRegistrator<ClanAreaHunter> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardHunter > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanHealerHunter> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterWeapon > >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterArmor > >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterBeaconTrap> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterSnareTrap> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterShovel> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterPitSteaks> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<HunterPitTrap> >( ppl );

        
        /*
         * battlerager
         */
        Plugin::registerPlugin<ObjectBehaviorRegistrator<BattleragerPoncho> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<PersonalBattleragerPoncho> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardBattlerager> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanHealerBattlerager> >( ppl );
        Plugin::registerPlugin<CChop>( ppl );

        /*
         * chaos
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardChaos> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ChaosBlade> >( ppl );
        
        /*
         * knight
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardKnight> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanItemKnight> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanAltarKnight> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<KnightWeapon> >( ppl );
        Plugin::registerPlugin<COrden>( ppl );
        
        /*
         * lion
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardLion> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<LionEyedSword> >( ppl );
        
        /*
         * ruler
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardRulerPre > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardRuler > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardRulerJailer > >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<RulerSpecialGuard> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<Stalker> >( ppl );
        
        /*
         * shalafi
         */
        Plugin::registerPlugin<MocRegistrator<ShalafiClan> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ShalafiFaculty> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardShalafi> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ShalafiDemon> >( ppl );
        
        /*
         * invader
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardInvader> >( ppl );
        Plugin::registerPlugin<CDarkLeague>( ppl );

        /*
         * other guards
         */
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardOutsider> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardFlowers> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanGuardGhost> >( ppl );
        
        /*
         * loader
         */
        Plugin::registerPlugin<ClanLoader>( ppl );
        Plugin::registerPlugin<ClanItemRefreshPlugin>( ppl );

        return ppl;
    }
}
