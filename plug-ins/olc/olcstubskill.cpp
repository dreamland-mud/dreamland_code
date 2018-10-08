/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "mocregistrator.h"

#include "olcstubskill.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

const DLString OLCStubSkill::CATEGORY = "Умения, доступные из OLC"; 

OLCStubSkill::OLCStubSkill( )
{
}

OLCStubSkill::~OLCStubSkill( )
{
}

SkillGroupReference & OLCStubSkill::getGroup( ) 
{
    return group;
}
const DLString & OLCStubSkill::getCategory( ) const
{
    return CATEGORY;
}
bool OLCStubSkill::visible( Character * ) const
{
    return true;
}
bool OLCStubSkill::available( Character * ) const
{
    return true;
}
bool OLCStubSkill::usable( Character *, bool ) const
{
    return true;
}

int OLCStubSkill::getLevel( Character * ) const
{
    return 1;
}

void OLCStubSkill::show( PCharacter *ch, std::ostream & buf ) 
{
    buf << (spell && spell->isCasted( ) ? "Заклинание" : "Умение")
        << " '{W" << getName( ) << "{x'"
        << " '{W" << getRussianName( ) << "{x', "
        << "входит в группу '{W" << getGroup( )->getName( ) << "{x'"
        << endl;
}

TABLE_LOADER(OLCStubSkillLoader, "olc-skills", "skill");

extern "C"
{
        SO::PluginList initialize_olc_skills( )
        {
            SO::PluginList ppl;

            Plugin::registerPlugin<MocRegistrator<OLCStubSkill> >( ppl );
            Plugin::registerPlugin<OLCStubSkillLoader>( ppl );

            return ppl;
        }
        
}

