/* $Id: impl.cpp,v 1.1.6.1.10.2 2008/02/24 17:23:06 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mocregistrator.h"

#include "clanskill.h"
#include "clanorgskill.h"
#include "skilllevelhook.h"
#include "clanownshook.h"
#include "clanrecords.h"
#include "clantreasury.h"
#include "character.h"

static bool char_owns( Character *ch, const DLString &id )
{
    Clan &clan = *ch->getClan( );
    return clan_is_reformed( clan ) && clan_owns( clan, id );
}

/** Plugs clan rank levels and catalog purchases into fight_core while this library is loaded. */
class ClanSkillLevelHookPlugin : public Plugin {
public:
    typedef ::Pointer<ClanSkillLevelHookPlugin> Pointer;

    virtual void initialization( )
    {
        skill_level_bonus_hook = &ClanSkill::rankLevelBonus;
        clan_owns_hook = &char_owns;
    }

    virtual void destruction( )
    {
        skill_level_bonus_hook = 0;
        clan_owns_hook = 0;
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
