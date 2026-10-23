/* $Id: impl.cpp,v 1.1.2.2 2007/09/11 00:33:55 rufina Exp $
 *
 * ruffina, 2004
 */
#include "class.h"
#include "so.h"

#include "objectbehaviorplugin.h"
#include "mobilebehaviorplugin.h"
#include "areabehaviorplugin.h"

#include "clanarea.h"
#include "clanobjects.h"
#include "clanmobiles.h"

extern "C"
{
    SO::PluginList initialize_clan_behavior( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<AreaBehaviorRegistrator<ClanArea> >( ppl );

        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanItem> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ClanAltar> >( ppl );


        Plugin::registerPlugin<MobileBehaviorRegistrator<ClanSummonedCreature> >( ppl );

        return ppl;
    }
}
