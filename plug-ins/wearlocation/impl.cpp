/* $Id: impl.cpp,v 1.1.2.5 2008/03/26 10:57:28 rufina Exp $
 *
 * ruffina, 2006
 */

#include <algorithm>

#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"
#include "xmlattributeplugin.h"
#include "commandtemplate.h"
#include "defaultwearlocation.h"
#include "misc_wearlocs.h"
#include "tohit.h"

TABLE_LOADER(WearlocationLoader, "wearlocations", "Wearlocation");

#define _WEARLOC_( name ) WearlocationReference wear_##name( #name );
#include "wearloc_utils.h"

static int wearloc_armor_coef( Wearlocation *loc )
{
    DefaultWearlocation *dloc = dynamic_cast<DefaultWearlocation *>( loc );
    return dloc ? dloc->getArmorCoef( ) : 0;
}

static int wear_flags_armor_coef( int wearFlags )
{
    int best = 0;

    for (int i = 0; i < wearlocationManager->size( ); i++) {
        DefaultWearlocation *dloc = dynamic_cast<DefaultWearlocation *>( wearlocationManager->find( i ) );

        if (dloc && dloc->givesAffects( ) && (wearFlags & dloc->getItemWear( )) != 0)
            best = std::max( best, dloc->getArmorCoef( ) );
    }

    return best;
}

/** Lends armorCoef to loadsave, which cannot link this plugin; cleared on unload
 *  so loadsave never calls into an unloaded plugin. */
class ArmorCoefHooks : public Plugin {
public:
    virtual void initialization( )
    {
        wearloc_armor_coef_fn = &wearloc_armor_coef;
        wear_flags_armor_coef_fn = &wear_flags_armor_coef;
    }
    virtual void destruction( )
    {
        wearloc_armor_coef_fn = 0;
        wear_flags_armor_coef_fn = 0;
    }
};

extern "C"
{
        SO::PluginList initialize_wearlocation( )
        {
            SO::PluginList ppl;
            
            Plugin::registerPlugin<MocRegistrator<DefaultWearlocation> >( ppl );
            Plugin::registerPlugin<MocRegistrator<StuckInWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<HairWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<TailWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<ShieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<SheathWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<WieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<HorseWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<SecondWieldWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<TattooWearloc> >( ppl );
            Plugin::registerPlugin<MocRegistrator<PersonalWearloc> >( ppl );
            Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeWearslot> >( ppl );

            Plugin::registerPlugin<WearlocationLoader>( ppl );
            Plugin::registerPlugin<ArmorCoefHooks>( ppl );
            
            return ppl;
        }
        
}

