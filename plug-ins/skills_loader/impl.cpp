#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"

#include "raceaptitude.h"
#include "clanskill.h"
#include "cardskill.h"
#include "craftskill.h"
#include "genericskill.h"
#include "defaultbehavior.h"

TABLE_LOADER(ClanSkillLoader, "clan-skills", "skill");

TABLE_LOADER(RaceAptitudeLoader, "race-aptitudes", "skill");

TABLE_LOADER(CardSkillLoader, "card-skills", "skill");

TABLE_LOADER(CraftSkillLoader, "craft-skills", "skill");

TABLE_LOADER(ClassSkillLoader, "generic-skills", "skill");

TABLE_LOADER(OtherSkillsLoader, "other-skills", "skill");

TABLE_LOADER(BehaviorLoader, "behaviors", "behavior");

extern "C"
{
        SO::PluginList initialize_skills_loader( )
        {
                SO::PluginList ppl;
                Plugin::registerPlugin<ClassSkillLoader>( ppl );
                Plugin::registerPlugin<CardSkillLoader>( ppl );
                Plugin::registerPlugin<CraftSkillLoader>( ppl );
                Plugin::registerPlugin<ClanSkillLoader>( ppl );
                Plugin::registerPlugin<RaceAptitudeLoader>( ppl );
                Plugin::registerPlugin<OtherSkillsLoader>( ppl );
                Plugin::registerPlugin<BehaviorLoader>(ppl);
                return ppl;
        }
        
}
