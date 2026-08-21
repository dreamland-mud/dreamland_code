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
    // Delegate to the lang_t overload so DefaultSkillCommand's multilingual
    // override is honoured -- otherwise a UA viewer fell back to the Russian
    // name (e.g. practice list showed "толчок" instead of "штовх").
    return getNameFor( ch ? viewerLang( ch ) : LANG_EN );
}

const DLString& SkillCommand::getNameFor(lang_t lang) const
{
    // Base commands only carry EN + RU; DefaultSkillCommand overrides this
    // with the full multilingual name.
    if (lang != LANG_EN)
        return getRussianName( );
    else
        return getName( );
}

