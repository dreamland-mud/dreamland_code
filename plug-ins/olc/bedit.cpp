#include <algorithm>

#include "bedit.h"

#include "util/regexp.h"
#include "olc.h"
#include "feniatriggers.h"
#include "pcharacter.h"
#include "defaultbehavior.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "behaviorloader.h"
#include "act.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

OLC_STATE(OLCStateBehavior);

OLCStateBehavior::OLCStateBehavior() : isChanged(false)
{
}

OLCStateBehavior::OLCStateBehavior(DefaultBehavior *bhv) 
    : isChanged(false)
{
    if (!bhv)
        return;

    original = bhv->getIndex();
}

OLCStateBehavior::~OLCStateBehavior() 
{
}

void OLCStateBehavior::commit() 
{
    if (!isChanged)
        return;

    DefaultBehavior *original = getOriginal();
    if (!original)
        return;
    
    original->save();

    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

DefaultBehavior * OLCStateBehavior::getOriginal()
{
    Behavior *behavior = behaviorManager->find(original->getIndex());
    if (!behavior)
        throw ::Exception("Attached behavior doesn't exist");

    DefaultBehavior *defaultBehavior = dynamic_cast<DefaultBehavior *>(behavior);
    if (!defaultBehavior)
        throw ::Exception("Attached behavior was unloaded");
    
    return defaultBehavior;
}

void OLCStateBehavior::statePrompt(Descriptor *d) 
{
    d->send( "Bhv> " );
}

void OLCStateBehavior::changed(PCharacter *ch)
{
    isChanged = true;
}

void OLCStateBehavior::show( PCharacter *ch )
{
    DefaultBehavior *bhv = getOriginal();

    ptc(ch, "Поведение {C%s{x\r\n", bhv->getName().c_str());

    ptc(ch, "Название:        {C%s{x %s {D(nameRus help){x\r\n",
            bhv->getRussianName().c_str(),
            web_edit_button(ch, "nameRus", "web").c_str());
    ptc(ch, "Цель:            {c%s {D(target){x\r\n", bhv->target.c_str());
    ptc(ch, "Подкоманды:      {c%s {D(subcommand){x\r\n", bhv->cmd.c_str());

    Json::FastWriter writer;
    DLString propsString = writer.write(bhv->props);

    ptc(ch, "Свойства:         %s {D(props help){x\r\n",
            web_edit_button(ch, "props", "web").c_str());
    ch->desc->send(propsString.c_str());

    feniaTriggers->showTriggers(ch, bhv->getWrapper(), "behavior", bhv->target);    

    ptc(ch, "\r\nКоманды: {y{hccommands{x, {y{hcshow{x, {y{hcdone{x\r\n");
}


BEDIT(fenia, "феня", "редактировать тригера")
{
    feniaTriggers->openEditor(ch, getOriginal(), argument);
    return false;
}

BEDIT(nameRus, "название", "установить названиес падежами")
{
    return editor(argument, getOriginal()->nameRus, (editor_flags)(ED_NO_NEWLINE));
}

BEDIT(target, "цель", "установить цель поведения")
{
    DLString args = argument;
    DLString target = args.getOneArgument();
    DefaultBehavior *bhv = getOriginal();

    if (arg_oneof(target, "room", "комната")) {
        ptc(ch, "Цель поведения теперь {Wкомната{x.\r\n");
        bhv->target = "room";
    } else if (arg_oneof(target, "mob", "моб")) {
        ptc(ch, "Цель поведения теперь {Wмоб{x.\r\n");
        bhv->target = "mob";
    } else if (arg_oneof(target, "obj", "item", "предмет")) {
        ptc(ch, "Цель поведения теперь {Wпредмет{x.\r\n");
        bhv->target = "obj";
    } else {
        ptc(ch, "Использование: target mob|room|obj\r\n");
        return false;
    }

    return true;
}

BEDIT(props, "свойства", "json свойства поведения")
{
    DefaultBehavior *bhv = getOriginal();
    return editor(argument, bhv->props, (editor_flags)(ED_JSON));
}

BEDIT(subcommands, "подкоманды", "установить подкоманды для use")
{
    DLString args = argument;
    DefaultBehavior *bhv = getOriginal();

    bhv->cmd = args.stripWhiteSpace();
    ptc(ch, "Подкоманды установлены в %s\r\n", bhv->cmd.c_str());
    // TODO: command name lookup and validation
    return true;
}

BEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

BEDIT(commands, "команды", "показать список встроенных команд cmdedit")
{
    do_commands(ch);
    return false;
}

BEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

CMD(bedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online behavior editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();
    Integer vnum;

    if (cmd.empty()) {
        stc("Формат:  bedit имя\r\n", ch);
        stc("         bedit list\r\n", ch);
        stc("         bedit create новое_имя\r\n", ch);
        return;
    }

    if (arg_oneof(cmd, "create", "создать")) {
        static RegExp namePattern("^[a-z ]{2,}$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("Укажите английское название поведения маленькими буквами.\r\n", ch);
            return;
        }

        if (behaviorManager->findUnstrict(args)) {
            stc("Поведение с похожим ключевым словом уже существует.\r\n", ch);
            return;
        }

        DefaultBehavior::Pointer bhv(NEW);
        bhv->setName(args);
        bhv->id = behaviorManager->getNextId();
        bhv->target = "obj";

        BehaviorLoader::getThis()->loadElement(bhv);
        BehaviorLoader::getThis()->saveElement(bhv);

        ptc(ch, "Создано новое поведение под именем %s.\r\n", bhv->getName().c_str());

        OLCStateBehavior::Pointer be(NEW, *bhv);
        be->attach(ch);
        be->show(ch);
        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(
            fmt(0, "{C%-15s %-17s %-4s{x\r\n", "Название", "", "Цель"));

        const DLString lineFormat = 
            "{W" + web_cmd(ch, "bedit $1", "%-15s") + "{w %-17s %-4s{x\r\n";

        for (int r = 0; r < behaviorManager->size(); r++) {
            DefaultBehavior *bhv = dynamic_cast<DefaultBehavior *>(behaviorManager->find(r));
            if (!bhv)
                continue;

            ch->send_to(fmt(0, lineFormat.c_str(),
                    bhv->getName().c_str(),
                    bhv->getRussianName().ruscase('1').c_str(),
                    bhv->target.c_str()));
        }

        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    Behavior *bhv = behaviorManager->findExisting(arg);
    if (!bhv)
        bhv = behaviorManager->findUnstrict(arg);
        
    DefaultBehavior *defaultBhv;

    if (!bhv || (defaultBhv = dynamic_cast<DefaultBehavior *>(bhv)) == NULL) {
        stc("Поведение с таким названием не найдено, используйте bedit list для списка.\r\n", ch);
        return;
    }

    OLCStateBehavior::Pointer be(NEW, defaultBhv);
    be->attach(ch);
    be->show(ch);
}
    
    

