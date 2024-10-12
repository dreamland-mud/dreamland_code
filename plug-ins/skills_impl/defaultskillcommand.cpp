/* $Id$
 *
 * ruffina, 2004
 */
#include "fenia/exceptions.h"
#include "skill.h"
#include "pcharacter.h"
#include "core/object.h"
#include "room.h"
#include "defaultskillcommand.h"
#include "commandflags.h"
#include "skillsflags.h"
#include "feniaskillaction.h"
#include "commandmanager.h"
#include "fight_position.h"
#include "directions.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

CommandTarget::CommandTarget() 
                : vict(0), obj(0), door(-1)
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

bool DefaultSkillCommand::saveCommand() const
{
    if (skill) {
        const XMLTableElement *element = skill.getDynamicPointer<XMLTableElement>();
        if (element) {
            element->save();
            return true;
        }
    }

    return false;
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
    return Command::getName( );
}

const DLString & DefaultSkillCommand::getRussianName( ) const
{
    return Command::getRussianName( );
}

bool DefaultSkillCommand::visible( Character *ch ) const
{
    if (!Command::visible( ch ))
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
            target.vict = get_char_room(actor, argOne);
            errbuf << "Рядом с тобой нет никого с таким именем.";
            return target.vict != 0;

        case ARG_CHAR_FIGHT:
            if (argOne.empty()) {
                if (!actor->fighting) {
                	errbuf << "Без указания цели это умение можно применять только в бою.";
                	return false;
                }
                else {
                	target.vict = actor->fighting;
                	return true;                  
                }
            }

            target.vict = get_char_room(actor, argOne);
            errbuf << "Рядом с тобой нет никого с таким именем.";
            return target.vict != 0;

        case ARG_CHAR_SELF:
            if (argOne.empty())
                target.vict = actor;
            else {
                target.vict = get_char_room(actor, argOne);
                errbuf << "Рядом с тобой нет никого с таким именем.";
            }
            return target.vict != 0;

        case ARG_EXIT:
            // Find exit or extra exit visible to the character.            
            if (argOne.empty()) {
                errbuf << "Укажи название двери или выхода, например: {yворота{x, {yвосток{x или {yю{x.";
                return false;

            } else {                
                EXTRA_EXIT_DATA *peexit = actor->in_room->extra_exits.find(argAll.c_str());

                if (peexit && actor->can_see(peexit)) {
                    target.extraExit = peexit->keyword;
                    target.doorOrExtraExit = target.extraExit;
                    return true;
                }

                target.door = find_exit(actor, argAll.c_str(), FEX_NO_INVIS | FEX_NO_EMPTY);
                if (target.door < 0) {
                    errbuf << "Ты не видишь здесь выхода с таким названием.";
                    return false;
                }

                target.doorOrExtraExit = dirs[target.door].name;
                return true;
            }
     
            break;

        default:
            return false;                        
    }
}

// Main run method called by interpreters. Calls legacy run() by default.
void DefaultSkillCommand::run( Character *ch, const DLString &args )
{
    CommandTarget target;
    ostringstream errbuf;

    // Do skill availability check early. TODO: custom message overrides.
    if (!skill->usable(ch)) {
        if (IS_CHARMED(ch))
            ch->master->pecho("%^C1 не владеет навыком {y{hh%s{x.", ch, skill->getNameFor(ch->master).c_str());
        ch->pecho("Ты не владеешь навыком {y{hh%s{x.", skill->getNameFor(ch).c_str());
        return;
    }

    // If argtype was defined in skill profile, enforce argument parsing.
    if (!parseArguments(ch, args, target, errbuf)) {
        ch->pecho(errbuf.str());
        return;
    }

    // Do mana and move checks early. TODO: show skill name somehow.
    int mana = skill->getMana(ch);
    if (mana > 0 && ch->mana < mana) {
        if (ch->is_npc() && IS_CHARMED(ch)) 
            say_fmt("Хозя%2$Gин|ин|йка, у меня мана кончилась!", ch, ch->master);
        else 
            ch->pecho("У тебя не хватает энергии.");

        return;
    }

    int moves = skill->getMoves(ch);
    if (moves > 0 && ch->move < moves) {
        if (ch->is_npc() && IS_CHARMED(ch))
            say_fmt("Хозя%2$Gин|ин|йка, я слишком устал%1$Gо||а!", ch, ch->master);
        else
            ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);

        return;
    }

    // Apply penalties early.
    if (mana > 0)
        ch->mana -= mana;
    if (moves > 0)
        ch->move -= moves;

    ch->setWait(skill->getBeats(ch));

    // See if there is 'run' method override in Fenia. 
    bool feniaOverride = FeniaSkillActionHelper::executeCommandRun(this, ch, target);

    // Fall back to the old implementation.
    if (!feniaOverride)
        Command::run( ch, args );

    // Potentially aggressive command, mark player as actively fighting.
    // Happens after 'run' because some skills (p.ex. camouflage) depend on last fight time delay.
    if (argtype == ARG_CHAR_FIGHT) {
        ch->setLastFightTime();
        UNSET_DEATH_TIME(ch);
    }
}

// Legacy method still defined for some commands.
void DefaultSkillCommand::run( Character *ch, char *args )
{
    Command::run( ch, args );
}

// An 'apply' method is called from objprogs or various places in the code.
// Usually just 'does the job' after all the checks were successful.
bool DefaultSkillCommand::apply( Character *ch, Character *victim, int level )
{
    bool feniaRc;

    // See if there is 'apply' method override in Fenia. Pass return code from Fenia further upstream.
    if (FeniaSkillActionHelper::executeCommandApply(this, ch, victim, level, feniaRc))
        return feniaRc;

    // Fall back to the old implementation.
    return applyLegacy( ch, victim, level );
}

bool DefaultSkillCommand::applyLegacy(Character * ch, Character *victim, int level)
{
    return false;
}
