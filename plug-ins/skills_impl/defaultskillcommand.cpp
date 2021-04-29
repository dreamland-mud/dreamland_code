/* $Id$
 *
 * ruffina, 2004
 */
#include "fenia/exceptions.h"
#include "skill.h"
#include "character.h"
#include "core/object.h"
#include "defaultskillcommand.h"
#include "commandflags.h"
#include "skillsflags.h"
#include "feniaskillaction.h"
#include "commandmanager.h"
#include "loadsave.h"
#include "def.h"

CommandTarget::CommandTarget() 
                : vict(0), obj(0)
{    
}

DefaultSkillCommand::DefaultSkillCommand( )
                         : argtype(ARG_UNDEF, &argtype_table)
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

bool DefaultSkillCommand::parseArguments(Character *actor, const DLString &constArgs, CommandTarget &target, ostringstream &errbuf) 
{
    DLString argAll = constArgs;
    DLString argRest = constArgs;
    DLString argOne = argRest.getOneArgument();

    target.argAll = argAll;
    target.argOne = argOne;
    target.argTwo = argRest;

    switch (argtype.getValue()) {
        case ARG_UNDEF:
            return true;

        case ARG_STRING:
            return true;

        case ARG_OBJ_CARRY:
            target.obj = get_obj_list(actor, argOne.c_str(), actor->carrying);
            errbuf << "Ты не видишь такого предмета у себя в инвентаре или экипировке.";
            return target.obj != 0;

        case ARG_OBJ_HERE:
            target.obj = get_obj_here(actor, argOne.c_str());
            errbuf << "Ты не видишь здесь такого предмета.";
            return target.obj != 0;

        case ARG_CHAR_ROOM:
            target.vict = get_char_room(actor, argOne.c_str());
            errbuf << "Рядом с тобой нет никого с таким именем.";
            return target.vict != 0;

        case ARG_CHAR_FIGHT:
            if (argOne.empty() && !actor->fighting) {
                errbuf << "Сейчас ты не сражаешься!";
                return false;
            }

            if (actor->fighting) {
                target.vict = actor->fighting;
                return true;
            }

            target.vict = get_char_room(actor, argOne.c_str());
            errbuf << "Рядом с тобой нет никого с таким именем.";
            return target.vict != 0;

        case ARG_CHAR_SELF:
            if (argOne.empty())
                target.vict = actor;
            else {
                target.vict = get_char_room(actor, argOne.c_str());
                errbuf << "Рядом с тобой нет никого с таким именем.";
            }
            return target.vict != 0;

        case ARG_EXIT:
            // TODO
            return true;

        default:
            return false;                        
    }
}

// Main run method called by interpreters. Calls legacy run() by default.
void DefaultSkillCommand::run( Character *ch, const DLString &args )
{
    CommandTarget target;
    ostringstream errbuf;

    // If argtype was defined in skill profile, enforce argument parsing.
    if (!parseArguments(ch, args, target, errbuf)) {
        ch->pecho(errbuf.str());
        return;
    }

    // See if there is 'run' method override in Fenia. 
    if (FeniaSkillActionHelper::executeCommand(this, ch, target))
        return;

    // Fall back to the old implementation.
    DefaultCommand::run( ch, args );
}

// Legacy method defined for most commands.
void DefaultSkillCommand::run( Character *ch, char *args )
{
    DefaultCommand::run( ch, args );
}


// TODO: helper run methods, will be renamed to applyChar or applyVict etc

bool DefaultSkillCommand::apply( Character *ch, Character *victim, int level )
{
    return SkillCommand::apply( ch, victim, level );
}

