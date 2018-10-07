/* $Id: impl.cpp,v 1.1.2.9.6.3 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "xmlattributeplugin.h"
#include "areabehaviorplugin.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"

#include "npcharacter.h"

#include "midgaardfountain.h"
#include "hierophant.h"
#include "pocketwatch.h"
#include "masquerade.h"
#include "rats.h"
#include "ofcolguards.h"
#include "anatolia_limits.h"
#include "moneychanger.h"
#include "petquestor.h"

extern struct spec_type local_spec_table[];

class SpecialProgsPlugin : public Plugin {
public:
    virtual void initialization( )
    {
	spec_table = local_spec_table;

	for (Character *ch = char_list; ch; ch = ch->next) {
	    NPCharacter *wch = ch->getNPC();
	    
	    if (wch && wch->spec_fun.func == 0 && !wch->spec_fun.name.empty( ))
		wch->spec_fun.func = spec_lookup( wch->spec_fun.name.c_str( ) );
	}
	
	for (int i = 0; i < MAX_KEY_HASH; i++)
	    for (MOB_INDEX_DATA *m = mob_index_hash[i]; m; m = m->next) 
		if (m->spec_fun.func == 0 && !m->spec_fun.name.empty( )) {
		    m->spec_fun.func = spec_lookup( m->spec_fun.name.c_str( ) );
		}
    }

    virtual void destruction( )
    {
	for (Character *ch = char_list; ch; ch = ch->next) {
	    NPCharacter *wch = ch->getNPC();

	    if (wch && wch->spec_fun.func) {
		wch->spec_fun.name = spec_name( wch->spec_fun.func );
		wch->spec_fun.func = 0;
	    }
	}
	
	for (int i = 0; i < MAX_KEY_HASH; i++)
	    for (MOB_INDEX_DATA *m = mob_index_hash[i]; m; m = m->next) 
		if (m->spec_fun.func) {
		    m->spec_fun.name = spec_name( m->spec_fun.func );
		    m->spec_fun.func = 0;
		}

	spec_table = zero_spec_table;
    }

};


extern "C"
{
    SO::PluginList initialize_misc_behaviors( ) {
	SO::PluginList ppl;

	Plugin::registerPlugin<SpecialProgsPlugin>( ppl );

	Plugin::registerPlugin<ObjectBehaviorRegistrator<MidgaardFountain> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<Hierophant> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<PocketWatch> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<MoneyChanger> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<PetQuestor> >( ppl );
	
	Plugin::registerPlugin<MobileBehaviorRegistrator<Masquer> >( ppl );
	Plugin::registerPlugin<AreaBehaviorRegistrator<Masquerade> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<RoamingPortal> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<CatsEye> >( ppl );
		
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeRats> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<Rat> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<RatGod> >( ppl );

	Plugin::registerPlugin<MobileBehaviorRegistrator<OfcolMarshal> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<OfcolGuard> >( ppl );

	Plugin::registerPlugin<ObjectBehaviorRegistrator<Excalibur> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<HasteBracers> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<SubissueWeapon> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<TwoSnakeWhip> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<Thunderbolt> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<FireGauntlets> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<VolcanoeArmbands> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<DemonfireShield> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<SwordOfSun> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<FlyingBoots> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<GiantStrengthArmor> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<RoseShield> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<LionClaw> >( ppl );
	Plugin::registerPlugin<ObjectBehaviorRegistrator<RingOfRa> >( ppl );

	return ppl;
    }
}

