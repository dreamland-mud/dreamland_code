#include "so.h"
#include "logstream.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"
#include "mocregistrator.h"
#include "character.h"
#include "subprofession.h"
#include "craftattribute.h"
#include "craftskill.h"

TABLE_LOADER(CraftProfessionLoader, "craft-professions", "CraftProfession");
TABLE_LOADER(CraftSkillLoader, "craft-skills", "skill");


class CraftProfessionRegistrator : public Plugin {
public:
    virtual void initialization( )
    {
	Class::regXMLVar<CraftProfessionHelp>( );
	Class::regMoc<CraftProfession>( );
    }

    virtual void destruction( )
    {
	Class::unregXMLVar<CraftProfessionHelp>( );
	Class::unregMoc<CraftProfession>( );
    }
};


extern "C"
{
    SO::PluginList initialize_craft( ) {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<CraftProfessionManager>( ppl );
	Plugin::registerPlugin<CraftProfessionRegistrator>( ppl );
	Plugin::registerPlugin<CraftProfessionLoader>( ppl );
	Plugin::registerPlugin<MocRegistrator<CraftSkill> >( ppl );
	Plugin::registerPlugin<CraftSkillLoader>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCraft> >( ppl );
	
	return ppl;
    }
}


