/* $Id: impl_groups.cpp,v 1.1.2.9.6.9 2008/05/27 21:30:03 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "xmlattributeplugin.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"

#include "group_beguiling.h"
#include "group_necromancy.h"
#include "group_vampiric.h"
#include "exoticskill.h"
#include "so.h"

class GroupSkillsRegistrator : public Plugin {
public:
    typedef ::Pointer<GroupSkillsRegistrator> Pointer;

    virtual void initialization( )
    {
        Class::regMoc<VampireSkill>( );
        Class::regMoc<ExoticSkill>( );
    }

    virtual void destruction( )
    {
        Class::unregMoc<ExoticSkill>( );
        Class::unregMoc<VampireSkill>( );
    }
};


extern "C"
{
        SO::PluginList initialize_group_skills( )
        {
            SO::PluginList ppl;
            
            Plugin::registerPlugin<GroupSkillsRegistrator>( ppl );

            Plugin::registerPlugin<ObjectBehaviorRegistrator<MagicJar> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<NecroCreature> >( ppl );
            Plugin::registerPlugin<MobileBehaviorRegistrator<AdamantiteGolem> >( ppl );
            
            return ppl;
        }
        
}

