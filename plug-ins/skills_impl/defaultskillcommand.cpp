/* $Id$
 *
 * ruffina, 2004
 */
#include "fenia/exceptions.h"
#include "skill.h"
#include "character.h"
#include "defaultskillcommand.h"
#include "commandflags.h"
#include "skillsflags.h"
#include "commandmanager.h"
#include "def.h"

DefaultSkillCommand::DefaultSkillCommand( )
                         : argtype(ARG_STRING, &argtype_table)
{
}

long long DefaultSkillCommand::getID() const
{
    int myId = 0;

    if (getSkill()->getSkillHelp())
        myId = getSkill()->getSkillHelp()->getID();

    if (myId <= 0)
        throw Scripting::Exception(getSkill()->getName() + ": skill command ID not found or zero");

    return (myId << 4) | 7;
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

// Main run method called by interpreters. Calls legacy run() by default.
void DefaultSkillCommand::run( Character *ch, const DLString &args )
{
    DefaultCommand::run( ch, args );
}

// Legacy method defined for most commands.
void DefaultSkillCommand::run( Character *ch, char *args )
{
    DefaultCommand::run( ch, args );
}


// TODO: helper run methods, will be renamed to applyChar or applyVict etc

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

