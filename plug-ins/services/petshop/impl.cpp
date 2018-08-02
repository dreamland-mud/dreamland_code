/* $Id: impl.cpp,v 1.1.2.2.6.4 2009/01/01 20:08:53 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "roombehaviorplugin.h"

#include "pet.h"
#include "petshoproom.h"
#include "petshopstorage.h"
#include "mixedpetshop.h"

extern "C"
{
    SO::PluginList initialize_services_petshop( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<MobileBehaviorRegistrator<Pet> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<LevelAdaptivePet> >( ppl );
	Plugin::registerPlugin<MobileBehaviorRegistrator<RideablePet> >( ppl );
	Plugin::registerPlugin<RoomBehaviorRegistrator<PetShopStorage> >( ppl );
	Plugin::registerPlugin<RoomBehaviorRegistrator<PetShopRoom> >( ppl );
	Plugin::registerPlugin<RoomBehaviorRegistrator<MixedPetShopRoom> >( ppl );
	    
	return ppl;
    }
}
