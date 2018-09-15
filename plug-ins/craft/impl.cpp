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

TABLE_LOADER(SubProfessionLoader, "subprofessions", "SubProfession");
TABLE_LOADER(CraftSkillLoader, "craft-skills", "skill");


class SubProfessionRegistrator : public Plugin {
public:
    virtual void initialization( )
    {
	Class::regXMLVar<SubProfessionHelp>( );
	Class::regMoc<SubProfession>( );
    }

    virtual void destruction( )
    {
	Class::unregXMLVar<SubProfessionHelp>( );
	Class::unregMoc<SubProfession>( );
    }
};


extern "C"
{
    SO::PluginList initialize_craft( ) {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<SubProfessionRegistrator>( ppl );
	Plugin::registerPlugin<SubProfessionLoader>( ppl );
	Plugin::registerPlugin<MocRegistrator<CraftSkill> >( ppl );
	Plugin::registerPlugin<CraftSkillLoader>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCraft> >( ppl );
	
	return ppl;
    }
}


