/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "defaultskillcommand.h"
#include "commandflags.h"
#include "commandmanager.h"
#include "def.h"

DefaultSkillCommand::DefaultSkillCommand( )
{
}

void DefaultSkillCommand::setSkill( SkillPointer skill )
{
    this->skill = skill;

    if (!extra.isSet(CMD_NO_INTERPRET))
        commandManager->registrate( Pointer( this ) );
}

void DefaultSkillCommand::unsetSkill( )
{
    if (!extra.isSet(CMD_NO_INTERPRET))
        commandManager->unregistrate( Pointer( this ) );

    skill.clear( );
}

SkillPointer DefaultSkillCommand::getSkill( ) const
{
    return skill;
}

const DLString & DefaultSkillCommand::getName( ) const
{
    return DefaultCommand::getName( );
}

const DLString & DefaultSkillCommand::getRussianName( ) const
{
    return DefaultCommand::getRussianName( );
}

bool DefaultSkillCommand::visible( Character *ch ) const
{
    if (!DefaultCommand::visible( ch ))
        return false;

    return getSkill( )->visible( ch );
}

void DefaultSkillCommand::run( Character *ch, const DLString &args )
{
    DefaultCommand::run( ch, args );
}

void DefaultSkillCommand::run( Character *ch, char *args )
{
    DefaultCommand::run( ch, args );
}

bool DefaultSkillCommand::run( Character *ch1, Character *ch2 )
{
    return SkillCommand::run( ch1, ch2 );
}

bool DefaultSkillCommand::run( Character *ch1 )
{
    return SkillCommand::run( ch1 );
}

bool DefaultSkillCommand::run( Character *ch, int value )
{
    return SkillCommand::run( ch, value );
}

void DefaultSkillCommand::run( Character *ch1, Character *ch2, Character *&ch3 )
{
    SkillCommand::run( ch1, ch2, ch3 );
}

void DefaultSkillCommand::run( Character *ch1, Character *ch2, int &value )
{
    SkillCommand::run( ch1, ch2, value );
}

