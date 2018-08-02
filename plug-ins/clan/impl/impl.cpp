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

#include "commandtemplate.h"
#include "xmlattributeplugin.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "defaultclan.h"
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

class ClanImplRegistrator : public Plugin {
public:
    typedef ::Pointer<ClanImplRegistrator> Pointer;

    virtual void initialization( )
    {
	Class::regMoc<KnightOrder>( );
	Class::regMoc<DefaultClan>( );
    }

    virtual void destruction( )
    {
	Class::unregMoc<DefaultClan>( );
	Class::unregMoc<KnightOrder>( );
    }
};


extern "C"
{
    SO::PluginList initialize_clan_impl( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<ClanImplRegistrator>( ppl );
    
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

	return ppl;
    }
}
