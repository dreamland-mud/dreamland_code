/* $Id$
 *
 * ruffina, 2004
 */
#include "skillcommand.h"
#include "character.h"
#include "lang.h"

SkillCommand::~SkillCommand( )
{
}


const DLString & SkillCommand::getName( ) const
{
    return DLString::emptyString;
}


const DLString & SkillCommand::getRussianName( ) const
{
    return DLString::emptyString;
}

const DLString& SkillCommand::getNameFor(Character *ch) const
{
    if (ch && viewerLang( ch ) != LANG_EN)
        return getRussianName( );
    else
        return getName( );
}

