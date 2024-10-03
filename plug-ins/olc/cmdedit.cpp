
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "room.h"
#include "areahelp.h"
#include "util/regexp.h"

#include "cmdedit.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"
#include "hedit.h"
#include "skedit.h"

#include "commandelement.h"
#include "commandtableloader.h"
#include "defaultskillcommand.h"
#include "commandflags.h"
#include "merc.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "comm.h"
#include "act.h"

#include "def.h"

OLC_STATE(OLCStateCommand);

OLCStateCommand::OLCStateCommand() : isChanged(false)
{
}

OLCStateCommand::OLCStateCommand(::Command *cmdPlugin) 
    : isChanged(false)
{
    if (!cmdPlugin)
        return;

    cmdName = cmdPlugin->getName();
}

OLCStateCommand::~OLCStateCommand() 
{
}

void OLCStateCommand::commit() 
{
    if (!isChanged)
        return;

    ::Command *original = getOriginal();
    if (!original)
        return;
    
    bool success = original->saveCommand();

    if (owner) {
        if (success)
            owner->character->pecho("Изменения сохранены на диск.");
        else
            owner->character->pecho("{RИзменения по команде не сохранились, проверь логи!{x");
    }
}

::Command * OLCStateCommand::getOriginal()
{
    ::Command::Pointer c = commandManager->find(cmdName);

    if (!c)
        throw Exception("Attached command doesn't exist");
    
    return *c;
}

// Re-add command to the command manager, when a name or an alias is changing.
bool OLCStateCommand::commandUpdate(::Command *c)
{
    commandManager->unregistrate(::Command::Pointer(c));
    commandManager->registrate(::Command::Pointer(c));
    return true;
}

// Some command help articles are empty and contain 'refby' keyword
// pointing to a help article with the info about this command.
CommandHelp::Pointer OLCStateCommand::resolveHelp(::Command *c)
{
    if (!c->help)
        return CommandHelp::Pointer();
    
    CommandHelp::Pointer refBy = c->help->getReferencedBy();
    if (refBy)
        return refBy;

    return c->help;
}


void OLCStateCommand::statePrompt(Descriptor *d) 
{
    d->send( "Command> " );
}

void OLCStateCommand::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateCommand::show( PCharacter *ch )
{
    ::Command *c = getOriginal();
    WrappedCommand *wcmd = dynamic_cast<WrappedCommand *>(c);

    ptc(ch, "Команда {C%s{x {x\r\n", c->getName().c_str());
    ptc(ch, "УкрИмя:      {Y%s{x %s {D(ukrname help){x\r\n",
            c->name[UA].c_str(),
            web_edit_button(ch, "ukrname", "web").c_str());
    ptc(ch, "РуИмя:       {Y%s{x %s {D(runame help){x\r\n",
            c->name[RU].c_str(),
            web_edit_button(ch, "runame", "web").c_str());
    ptc(ch, "Синонимы:    {Y%s{x %s {D(aliases help){x\r\n",
            c->aliases[EN].c_str(),
            web_edit_button(ch, "aliases", "web").c_str());
    ptc(ch, "УкрСинонимы: {Y%s{x %s {D(ukraliases help){x\r\n",
            c->aliases[UA].c_str(),
            web_edit_button(ch, "ukraliases", "web").c_str());
    ptc(ch, "РуСинонимы:  {Y%s{x %s {D(rualiases help){x\r\n",
            c->aliases[RU].c_str(),
            web_edit_button(ch, "rualiases", "web").c_str());
    ptc(ch, "Уровень:     {Y%d {D(level){x\r\n", c->level.getValue());
    ptc(ch, "Позиция:     {Y%s {D(position){x\r\n", c->position.name().c_str());
    ptc(ch, "Флаги:       {Y%s {D(flags){x\r\n", c->extra.names().c_str());
    ptc(ch, "Приказ:      {Y%s {D(order){x\r\n", 
            c->order == 0 ? "-" : c->order.names().c_str());
    ptc(ch, "Категории:   {Y%s {D(category){x\r\n", c->cat.names().c_str());
    ptc(ch, "Подсказка:   {Y%s{x %s {D(hint help){x\r\n",
            c->hint[RU].c_str(),
            web_edit_button(ch, "hint", "web").c_str());        

    // Can edit all commands but allow to override 'runFunc' only for CommandElement and CommandPlugin.
    if (wcmd)
        feniaTriggers->showTriggers(ch, wcmd->getWrapper(), "command");

    CommandHelp::Pointer help = resolveHelp(c);
    if (help) {
        ptc(ch, "Справка: %s {D(hedit %d",
            web_edit_button(ch, "hedit", help->getID()).c_str(),
            help->getID());

        if (c->help != help)
            ptc(ch, "{D, %s", help->getCommand()->getName().c_str());

        ptc(ch, "){x\r\n");
    }

    ptc(ch, "{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

CMDEDIT(fenia, "феня", "редактировать тригера")
{
    DLString args = argument;
    ::Command *c = getOriginal();
    WrappedCommand *wcmd = dynamic_cast<WrappedCommand *>(c);

    if (!wcmd) {
        stc("На этот тип команды невозможно присвоить феневый триггер.\r\n", ch);
        return false;
    }

    DLString trigName = args.getOneArgument();
    bool clear = arg_is_clear(args);

    if (trigName.empty() || (!args.empty() && !clear)) {
        stc("Использование: fenia <триггер> - редактировать триггер.\r\n", ch);
        stc("               fenia <триггер> clear - очистить триггер.\r\n", ch);
        return false;
    }
    
    if (clear) {
        if (feniaTriggers->clearTrigger(wcmd->wrapper, trigName))
            ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
        else
            ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
    } else {
        feniaTriggers->openEditor(ch, wcmd, trigName);
    }
    
    return false;
}

CMDEDIT(help, "справка", "создать или посмотреть справку по команде")
{
    ::Command *cmd = getOriginal();

    auto postCreateAction = [cmd](XMLPointerNoEmpty<CommandHelp> &help) {
        help->setCommand(::Command::Pointer(cmd));
    };

    return help_subcommand(ch, argument, cmd->help, postCreateAction);
}

CMDEDIT(hint, "подсказка", "краткое описание команды")
{
    ::Command *c = getOriginal();
    return editor(argument, c->hint[RU], ED_NO_NEWLINE);
}

CMDEDIT(ukrname, "укримя", "украинское имя команды")
{
    ::Command  *c = getOriginal();
    return editor(argument, c->name[UA], ED_NO_NEWLINE) && commandUpdate(c);
}

CMDEDIT(runame, "руимя", "русское имя команды")
{
    ::Command  *c = getOriginal();
    return editor(argument, c->name[RU], ED_NO_NEWLINE) && commandUpdate(c);
}

CMDEDIT(aliases, "синонимы", "список англ синонимов для команды")
{
    ::Command  *c = getOriginal();
    return editor(argument, c->aliases[EN], ED_NO_NEWLINE) && commandUpdate(c);
}

CMDEDIT(ukraliases, "укрсинонимы", "список украинских синонимов для команды")
{
    ::Command  *c = getOriginal();
    return editor(argument, c->aliases[UA], ED_NO_NEWLINE) && commandUpdate(c);
}

// Edit Russian aliases for the command, re-register with CommandManager if changed
CMDEDIT(rualiases, "русинонимы", "список русских синонимов для команды")
{
    ::Command  *c = getOriginal();
    return editor(argument, c->aliases[RU], ED_NO_NEWLINE) && commandUpdate(c);
}

CMDEDIT(level, "уровень", "уровень, с которого доступна команда")
{
    ::Command  *c = getOriginal();
    return numberEdit(-1, MAX_LEVEL, (int &)c->level);
}

CMDEDIT(flags, "флаги", "флаги команды (? command_flags)")
{
    return flagBitsEdit(getOriginal()->extra);
}

CMDEDIT(category, "категории", "флаги категорий (? command_category_flags)")
{
    return flagBitsEdit(getOriginal()->cat);
}

CMDEDIT(order, "приказ", "кому можно приказать выполнять команду (? order_flags)")
{
    return flagBitsEdit(getOriginal()->order);
}

CMDEDIT(position, "позиция", "мин. положение тела для команды (? position_table)")
{
    return flagValueEdit(getOriginal()->position);
}

CMDEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

CMDEDIT(commands, "команды", "показать список встроенных команд cmdedit")
{
    do_commands(ch);
    return false;
}

CMDEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

CMD(cmdedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online command editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  cmdedit название\r\n", ch);
        stc("         cmdedit list\r\n", ch);
        stc("         cmdedit ua\r\n", ch);
        stc("         cmdedit create название\r\n", ch);
        return;
    }

    if (arg_oneof(cmd, "ua")) {
        ostringstream buf;

        for (auto &c: commandManager->getCommands().getCommands()) {
            Command::Pointer cmd = commandManager->findExact(c->getName());

            if (cmd->name[RU].empty())
                continue;

//            cmd->saveCommand();
            DLString format = DLString("{Y") + web_cmd(ch, "cmdedit $1", "%-13.13s") + " {G%-13.13s{x {R%-13.13s{x [{y%-15.15s{x] [{g%-15.15s{x] [{r%-15.15s{x]\r\n";
            buf << fmt(0, format.c_str(), 
                    cmd->name[EN].c_str(), 
                    cmd->name[RU].c_str(), 
                    cmd->name[UA].c_str(), 
                    cmd->aliases[EN].c_str(), 
                    cmd->aliases[RU].c_str(),
                    cmd->aliases[UA].c_str());
        }

        page_to_char(buf.str().c_str(), ch);

        return;
    }

    // Creating new command
    if (arg_oneof(cmd, "create", "создать")) {
        DLString name = args.getOneArgument().toLower();
        CommandElement::Pointer newCommand;        

        static RegExp namePattern("^[a-z]{2,}$", true);
        if (name.empty() || !namePattern.match(name)) {
            stc("В названии команды могут быть только маленькие англ буквы.\r\n", ch);
            return;
        }

        Command::Pointer oldCommand = commandManager->findExact(name);
        if (oldCommand) {
            ptc(ch, "Команда '%s' уже существует.\r\n", oldCommand->getName().c_str());
            return;
        } 

        newCommand.construct();

        newCommand->setName(name);
        newCommand->help.construct();
        newCommand->help->setID(
            help_next_free_id()
        );

        CommandTableLoader::getThis()->loadElement(newCommand);
        CommandTableLoader::getThis()->saveElement(newCommand);

        ptc(ch, "Создана новая команда %s.\r\n", newCommand->getName().c_str());

        OLCStateCommand::Pointer cmdedit(NEW, *newCommand);
        cmdedit->attach(ch);
        cmdedit->show(ch);

        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(fmt(0, "{C%-15s %-17s{x\r\n", "Команда", "Русское имя"));

        const DLString lineFormatCmdEdit = 
            "{W" + web_cmd(ch, "cmdedit $1", "%-15s") + "{w %-17s{x\r\n";

        const DLString lineFormatSkillEdit = 
            "{C" + web_cmd(ch, "skedit $1", "%-15s") + "{w (умение %s){x\r\n";

        for (auto &c: commandManager->getCommands().getCommands()) {
            const DefaultSkillCommand *skillCmd = c.getDynamicPointer<DefaultSkillCommand>();

            if (skillCmd)
                ch->send_to(fmt(0, lineFormatSkillEdit.c_str(),
                        skillCmd->getName().c_str(),
                        skillCmd->getSkill()->getName().c_str()));
            else
                ch->send_to(fmt(0, lineFormatCmdEdit.c_str(),
                        c->getName().c_str(),
                        c->getRussianName().c_str()));

        }
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    
    Command::Pointer c = commandManager->findExact(arg);

    if (!c) {
        c = commandManager->findUnstrict(arg);
    }

    if (!c) {
        ptc(ch, "Команда '%s' не найдена.\r\n", arg.c_str());
        return;
    }

    DefaultSkillCommand *skillCmd = c.getDynamicPointer<DefaultSkillCommand>();
    if (skillCmd) {
        ptc(ch, "Запускаю 'skedit' для команды '%s' умения '%s'.\r\n", 
                 c->getName().c_str(),
                 skillCmd->getSkill()->getName().c_str());

        OLCStateSkill::Pointer ske(NEW, skillCmd->getSkill().getPointer());
        ske->attach(ch);
        ske->show(ch);
        return;
    }

    OLCStateCommand::Pointer ce(NEW, *c);
    ce->attach(ch);
    ce->show(ch);
}

