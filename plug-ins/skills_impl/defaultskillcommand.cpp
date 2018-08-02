/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "defaultskillcommand.h"
#include "commandmanager.h"

DefaultSkillCommand::DefaultSkillCommand( )
                        : interp( true )
{
}

void DefaultSkillCommand::setSkill( SkillPointer skill )
{
    this->skill = skill;

    if (interp)
	commandManager->registrate( Pointer( this ) );
}

void DefaultSkillCommand::unsetSkill( )
{
    if (interp)
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

