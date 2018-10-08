/* $Id: impl.cpp,v 1.1.6.1.10.2 2008/02/24 17:23:06 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"

#include "clanskill.h"
#include "clanorgskill.h"

TABLE_LOADER(ClanSkillLoader, "clan-skills", "skill");

extern "C"
{
    SO::PluginList initialize_clan_skill( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<ClanSkill> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ClanOrgSkill> >( ppl );
        Plugin::registerPlugin<ClanSkillLoader>( ppl );
        
        return ppl;
    }
}
