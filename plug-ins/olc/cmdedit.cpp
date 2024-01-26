
#include <character.h>
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

#include "defaultskillcommand.h"
#include "commandflags.h"
#include "merc.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateCommand);

OLCStateCommand::OLCStateCommand() : isChanged(false)
{
}

OLCStateCommand::OLCStateCommand(CommandPlugin *cmdPlugin) 
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

    CommandPlugin *original = getOriginal();
    if (!original)
        return;
    
    original->getLoader()->saveCommand(original);

    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

CommandPlugin * OLCStateCommand::getOriginal()
{
    ::Command::Pointer c = commandManager->find(cmdName);

    if (!c)
        throw Exception("Attached command doesn't exist");

    CommandPlugin *cmdPlugin = c.getDynamicPointer<CommandPlugin>();
    if (!cmdPlugin)
        throw Exception("Attached command is not editable");
    
    return cmdPlugin;
}

// Re-add command to the command manager, when a name or an alias is changing.
bool OLCStateCommand::commandUpdate(CommandPlugin *c)
{
    commandManager->unregistrate(CommandPlugin::Pointer(c));
    commandManager->registrate(CommandPlugin::Pointer(c));
    return true;
}

// Some command help articles are empty and contain 'refby' keyword
// pointing to a help article with the info about this command.
CommandHelp::Pointer OLCStateCommand::resolveHelp(CommandPlugin *c)
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
    CommandPlugin *c = getOriginal();

    ptc(ch, "Команда {W%s{x {x\r\n", c->getName().c_str());

    ptc(ch, "Синонимы:    {Y%s{x %s {D(aliases help){x\r\n",
            c->aliases.toList().toString().c_str(),
            web_edit_button(ch, "aliases", "").c_str());
    ptc(ch, "РуСинонимы:  {Y%s{x %s {D(rualiases help){x\r\n",
            c->russian.toList().toString().c_str(),
            web_edit_button(ch, "rualiases", "").c_str());
    ptc(ch, "Уровень:     {Y%d {D(level){x\r\n", c->level.getValue());
    ptc(ch, "Позиция:     {Y%s {D(position){x\r\n", c->position.name().c_str());
    ptc(ch, "Флаги:       {Y%s {D(flags){x\r\n", c->extra.names().c_str());
    ptc(ch, "Приказ:      {Y%s {D(order){x\r\n", 
            c->order == 0 ? "-" : c->order.names().c_str());
    ptc(ch, "Категории:   {Y%s {D(category){x\r\n", c->cat.names().c_str());
    ptc(ch, "Подсказка:   {Y%s{x %s {D(hint help){x\r\n",
            c->hint.c_str(),
            web_edit_button(ch, "hint", "web").c_str());        

    feniaTriggers->showTriggers(ch, c->getWrapper(), "command");

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
    CommandPlugin *c = getOriginal();

    DLString trigName = args.getOneArgument();
    bool clear = arg_is_clear(args);

    if (trigName.empty() || (!args.empty() && !clear)) {
        stc("Использование: fenia <триггер> - редактировать триггер.\r\n", ch);
        stc("               fenia <триггер> clear - очистить триггер.\r\n", ch);
        return false;
    }
    
    if (clear) {
        if (feniaTriggers->clearTrigger(c->wrapper, trigName))
            ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
        else
            ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
    } else {
        feniaTriggers->openEditor(ch, c, trigName);
    }
    
    return false;
}

CMDEDIT(help, "справка", "создать или посмотреть справку по команде")
{
    DLString arg = argument;
    CommandPlugin *c = getOriginal();

    if (arg.empty()) {
        if (!c->help || c->help->getID() < 1) {
            ptc(ch, "Справка не задана, используй help create для создания новой.");
            return false;
        }

        OLCStateHelp::Pointer hedit(NEW, *resolveHelp(c));
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }

    if (arg_oneof(arg, "create", "создать")) {
        if (c->help && c->help->getID() > 0) {
            ptc(ch, "Справка уже существует, используй команду help для редактирования.");
            return false;
        }

        if (!c->help)
            c->help.construct();
        c->help->setID(
            help_next_free_id()
        );
        c->help->setCommand(CommandPlugin::Pointer(c));

        OLCStateHelp::Pointer hedit(NEW, c->help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }   

    ptc(ch, "Использование: help, help create\r\n");
    return false;
}

CMDEDIT(hint, "подсказка", "краткое описание команды")
{
    CommandPlugin *c = getOriginal();
    return editor(argument, c->hint, ED_NO_NEWLINE);
}

CMDEDIT(aliases, "синонимы", "список англ синонимов для команды")
{
    CommandPlugin *c = getOriginal();
    return stringListEdit(c->aliases) && commandUpdate(c);
}

// Edit Russian aliases for the command, re-register with CommandManager if changed
CMDEDIT(rualiases, "русинонимы", "список русских синонимов для команды")
{
    CommandPlugin *c = getOriginal();
    return stringListEdit(c->russian) && commandUpdate(c);
}

CMDEDIT(level, "уровень", "уровень, с которого доступна команда")
{
    CommandPlugin *c = getOriginal();
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

CMD(cmdedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online religion editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  cmdedit название\r\n", ch);
        stc("         cmdedit list\r\n", ch);
        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(dlprintf("{C%-15s %-17s{x\r\n", "Команда", "Русское имя"));

        const DLString lineFormatCmdEdit = 
            "{W" + web_cmd(ch, "cmdedit $1", "%-15s") + "{w %-17s{x\r\n";

        const DLString lineFormatSkillEdit = 
            "{C" + web_cmd(ch, "skedit $1", "%-15s") + "{w (умение %s){x\r\n";

        const DLString lineFormatNoEdit = 
            "{w%-15s (не редактируется)\r\n";

        for (auto &c: commandManager->getCommands().getCommands()) {
            const DefaultSkillCommand *skillCmd = c.getDynamicPointer<DefaultSkillCommand>();
            const CommandPlugin *cmdPlugin = c.getDynamicPointer<CommandPlugin>();

            if (cmdPlugin)
                ch->send_to(dlprintf(lineFormatCmdEdit.c_str(),
                        cmdPlugin->getName().c_str(),
                        cmdPlugin->getRussianName().c_str()));
            else if (skillCmd)
                ch->send_to(dlprintf(lineFormatSkillEdit.c_str(),
                        skillCmd->getName().c_str(),
                        skillCmd->getSkill()->getName().c_str()));
            else
                ch->send_to(dlprintf(lineFormatNoEdit.c_str(),
                        c->getName().c_str()));
                

        }
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    
    Command::Pointer c = commandManager->findExact(arg);
    CommandPlugin *cmdPlugin = 0;

    if (!c || !c.getDynamicPointer<CommandPlugin>()) {
        c = commandManager->findUnstrict(arg);
    }

    if (c)
        cmdPlugin = c.getDynamicPointer<CommandPlugin>();
    
    if (!cmdPlugin) {
        if (c)
            ptc(ch, "Команду '%s' невозможно отредактировать с помощью cmdedit.\r\n", c->getName().c_str());
        else
            ptc(ch, "Команда '%s' не найдена.\r\n", arg.c_str());
        return;
    }

    OLCStateCommand::Pointer ce(NEW, cmdPlugin);
    ce->attach(ch);
    ce->show(ch);
}

