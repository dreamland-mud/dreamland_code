#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"


TABLE_LOADER(OtherSkillsLoader, "other-skills", "skill");

extern "C"
{
        SO::PluginList initialize_other_skills( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<OtherSkillsLoader>( ppl );
                
                return ppl;
        }
        
}
