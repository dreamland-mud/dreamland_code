/* $Id: impl.cpp,v 1.1.2.1.6.1 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "so.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "xmlattributeplugin.h"
#include "gangstersinfo.h"
#include "objects.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"

extern "C"
{
    SO::PluginList initialize_gquest_gangsters( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<GangMember> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<GangChef> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<GangKey> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<GangPortal> >( ppl );
        Plugin::registerPlugin<GangstersInfo>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeGangsters> >( ppl );
        
        return ppl;
    }
}
