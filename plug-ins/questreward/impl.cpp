/* $Id: impl.cpp,v 1.1.2.5.10.1 2007/05/02 02:32:37 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "questreward.h"
#include "personalquestreward.h"
#include "questgirth.h"
#include "questring.h"
#include "questweapon.h"
#include "questbag.h"
#include "valentineprise.h"
#include "ownercoupon.h"

extern "C"
{
        SO::PluginList initialize_questreward( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestReward> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<PersonalQuestReward> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestGirth> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestRing> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestWeapon> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<QuestBag> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<ValentinePrise> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<OwnerCoupon> >( ppl );
                
                return ppl;
        }
}
