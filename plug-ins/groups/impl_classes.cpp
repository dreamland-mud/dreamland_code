/* $Id: impl_classes.cpp,v 1.1.6.6 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "xmlattributeplugin.h"
#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "commandtemplate.h"

#include "class_antipaladin.h"
#include "class_ranger.h"
#include "class_samurai.h"
#include "class_vampire.h"
#include "class_warlock.h"
#include "class_universal.h"
#include "so.h"

extern "C"
{
        SO::PluginList initialize_class_skills( )
        {
            SO::PluginList ppl;

            Plugin::registerPlugin<ObjectBehaviorRegistrator<ShadowBlade> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<AntipaladinGuildmaster> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<RangerCreature> >( ppl );
            Plugin::registerPlugin<ObjectBehaviorRegistrator<RangerStaff> >( ppl );
            Plugin::registerPlugin<ObjectBehaviorRegistrator<EnergyShield> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<SamuraiGuildmaster> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<SamuraiUniclassAdept> >( ppl );
            Plugin::registerPlugin<ObjectBehaviorRegistrator<Katana> >( ppl );
            Plugin::registerPlugin<ObjectBehaviorRegistrator<OwnedKatana> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<VampireGuildmaster> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<UniclassAdept> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<UniclassAdeptAndShopTrader> >( ppl );
            Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeUniclass> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<DwarkinAdept> >( ppl );
            Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeEnlight> >( ppl );
        
            
            return ppl;
        }
        
}

