/* $Id: genericskillloader.cpp,v 1.1.2.4.10.1 2007/06/26 07:15:12 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"

#include "genericskillloader.h"
#include "genericskill.h"

const DLString GenericSkillLoader::TABLE_NAME = "generic-skills";
const DLString GenericSkillLoader::NODE_NAME = "skill";

void GenericSkillLoader::initialization( )
{
    XMLTableLoaderPlugin::initialization( );
    resolveAll( );
}

void GenericSkillLoader::destruction( )
{
    unresolveAll( );
    XMLTableLoaderPlugin::destruction( );
}

void GenericSkillLoader::resolveAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
	GenericSkill *skill = e->getStaticPointer<GenericSkill>( );

	skill->resolve( );
    }
}

void GenericSkillLoader::unresolveAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
	GenericSkill *skill = e->getStaticPointer<GenericSkill>( );

	skill->unresolve( );
    }
}

extern "C"
{
	SO::PluginList initialize_genericskill_loader( )
	{
		SO::PluginList ppl;
		
		Plugin::registerPlugin<GenericSkillLoader>( ppl );
		
		return ppl;
	}
	
}
