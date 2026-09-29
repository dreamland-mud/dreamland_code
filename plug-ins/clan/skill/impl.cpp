/* $Id: impl.cpp,v 1.1.6.1.10.2 2008/02/24 17:23:06 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mocregistrator.h"

#include "clanskill.h"
#include "clanorgskill.h"
#include "skilllevelhook.h"

/** Plugs clan rank levels into skill_level_bonus while this library is loaded. */
class ClanSkillLevelHookPlugin : public Plugin {
public:
    typedef ::Pointer<ClanSkillLevelHookPlugin> Pointer;

    virtual void initialization( )
    {
        skill_level_bonus_hook = &ClanSkill::rankLevelBonus;
    }

    virtual void destruction( )
    {
        skill_level_bonus_hook = 0;
    }
};

extern "C"
{
    SO::PluginList initialize_clan_skill( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<ClanSkill> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ClanOrgSkill> >( ppl );
        Plugin::registerPlugin<ClanSkillLevelHookPlugin>( ppl );
        
        return ppl;
    }
}
