/* $Id: cardskillloader.cpp,v 1.1.2.4.10.2 2008/02/24 17:22:37 rufina Exp $
 *
 * ruffina, 2005
 */

#include "cardskillloader.h"
#include "cardskill.h"

#include "class.h"

const DLString CardSkillLoader::TABLE_NAME = "card-skills";
const DLString CardSkillLoader::NODE_NAME = "skill";


void CardSkillLoader::initialization( )
{
    Class::regMoc<CardSkill>( );
    XMLTableLoaderPlugin::initialization( );
}

void CardSkillLoader::destruction( )
{
    XMLTableLoaderPlugin::destruction( );
    Class::unregMoc<CardSkill>( );
}

