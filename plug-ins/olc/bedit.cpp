#include <algorithm>

#include "bedit.h"

#include "util/regexp.h"
#include "olc.h"
#include "hedit.h"
#include "feniatriggers.h"
#include "pcharacter.h"
#include "room.h"
#include "behaviorloader.h"
#include "json_utils.h"
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
    ptc(ch, "Описание:        %s {D(desc help){x\r\n{c%s{x\r\n",
            web_edit_button(ch, "desc", "web").c_str(),
            bhv->description.c_str());
    ptc(ch, "Цель:            {c%s {D(target){x\r\n", bhv->target.name().c_str());
    ptc(ch, "Подкоманды:      {c%s {D(subcommand){x\r\n", bhv->cmd.c_str());

    if (bhv->help) {
        ptc(ch, "Справка:         %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", bhv->help->getID()).c_str(),
            bhv->help->getID());
    } else {
        ptc(ch, "Справка:         нет {D({hchelp create{x)\r\n");
    }

    ptc(ch, "Свойства:        %s {D(props help){x\r\n",
            web_edit_button(ch, "props", "web").c_str());
    DLString propsString = JsonUtils::toString(bhv->props);
    ch->desc->send(propsString.c_str());

    feniaTriggers->showTriggers(ch, bhv->getWrapper(), "behavior", bhv->target.name());    

    ptc(ch, "\r\nКоманды: {y{hccommands{x, {y{hcshow{x, {y{hclist{x, {y{hcdone{x\r\n");
}

BEDIT(fenia, "феня", "редактировать тригера")
{
    feniaTriggers->openEditor(ch, getOriginal(), argument);
    return false;
}

BEDIT(nameRus, "название", "установить название с падежами")
{
    return editor(argument, getOriginal()->nameRus, (editor_flags)(ED_NO_NEWLINE));
}

BEDIT(description, "описание", "установить описание")
{
    return editor(argument, getOriginal()->description, (editor_flags)(ED_NO_NEWLINE));
}

BEDIT(target, "цель", "установить цель поведения")
{
    return flagValueEdit(getOriginal()->target);
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

BEDIT(help, "справка", "создать или посмотреть справку")
{
    DefaultBehavior *bhv = getOriginal();

    auto postCreateAction = [bhv](XMLPointerNoEmpty<BehaviorHelp> &help) {
        help->setBehavior(DefaultBehavior::Pointer(bhv));
    };

    return help_subcommand(ch, argument, bhv->help, postCreateAction);
}

// Return a map from behavior name to the list of obj/mob/room vnums that use it.
static map<DLString, set<int> > behavior_usage()
{
    map<DLString, set<int> > usage;

    for (int i = 0; i < MAX_KEY_HASH; i++)
        for (auto *pObj = obj_index_hash[i]; pObj; pObj = pObj->next)
            for (auto b: pObj->behaviors.toArray())
                usage[behaviorManager->find(b)->getName()].insert(pObj->vnum);

    for (int i = 0; i < MAX_KEY_HASH; i++)
        for (auto *pMob = mob_index_hash[i]; pMob; pMob = pMob->next)
            for (auto b: pMob->behaviors.toArray())
                usage[behaviorManager->find(b)->getName()].insert(pMob->vnum);

    for (auto r: roomIndexMap)
        for (auto b: r.second->behaviors.toArray())
            usage[behaviorManager->find(b)->getName()].insert(r.first);

    return usage;
}


BEDIT(list, "список", "показать все сущности с этим поведением")
{
    ostringstream buf;
    DefaultBehavior *bhv = getOriginal();
    auto usage = behavior_usage();
    auto myVnums = usage[bhv->getName()];
    auto target = bhv->target.getValue();
    DLString cmd = (target == INDEX_MOB ? "medit" : (target == INDEX_ROOM ? "redit" : "oedit"));
    DLString lineFormat = "[" + web_cmd(ch, cmd + " $1", "%6d") + "] %-18.18N1   ";
    int lineCount = 0;

    for (auto vnum: myVnums) {
        DLString name = (target == INDEX_MOB ? get_mob_index(vnum)->short_descr 
            : (target == INDEX_ROOM ? get_room_index(vnum)->name 
                : get_obj_index(vnum)->short_descr));

        buf << fmt(0, lineFormat.c_str(), vnum, name.c_str());

        if (++lineCount % 2 == 0)
            buf << endl;
    }

    buf << endl;

    ptc(ch, "Это поведение присвоено: \r\n");

    if (lineCount == 0)
        ptc(ch, "   (пока ничему)\r\n");
    else 
        ch->send_to(buf);

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

static bool bhv_cmp_name(DefaultBehavior *a, DefaultBehavior *b)
{
    return a->getName().compare(b->getName()) < 0;
}

CMD(bedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online behavior editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();
    Integer vnum;

    if (cmd.empty()) {
        stc("Формат:  bedit имя\r\n", ch);
        stc("         bedit list\r\n", ch);
        stc("         bedit create новое имя\r\n", ch);
        return;
    }

    if (arg_oneof(cmd, "create", "создать")) {
        static RegExp namePattern("^[a-z_ ]{2,}$", true);
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
        bhv->target = INDEX_OBJ;
        bhv->help.construct();
        bhv->help->setID(
            help_next_free_id()
        );

        BehaviorLoader::getThis()->loadElement(bhv);
        BehaviorLoader::getThis()->saveElement(bhv);

        ptc(ch, "Создано новое поведение под именем %s.\r\n", bhv->getName().c_str());

        OLCStateBehavior::Pointer be(NEW, *bhv);
        be->attach(ch);
        be->show(ch);
        return;
    }

    if (arg_is_list(cmd)) {
        auto usage = behavior_usage();

        ch->send_to(
            fmt(0, "{C%-20s %-25s %4s %5s{x\r\n", "Название", "", "Тип", "Всего"));

        const DLString lineFormat = 
            "{W" + web_cmd(ch, "bedit $1", "%-20s") + "{w %-25.25s %4s %5d{x\r\n";

        list<DefaultBehavior *> output;

        for (int r = 0; r < behaviorManager->size(); r++) {
            DefaultBehavior *bhv = dynamic_cast<DefaultBehavior *>(behaviorManager->find(r));
            if (bhv)
                output.push_back(bhv);
        }

        output.sort(bhv_cmp_name);

        for (auto *bhv: output) {
            ch->send_to(fmt(0, lineFormat.c_str(),
                    bhv->getName().c_str(),
                    bhv->getRussianName().ruscase('1').c_str(),
                    bhv->target.name().c_str(),
                    usage[bhv->getName()].size()));
        }

        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    Behavior *bhv = behaviorManager->findExisting(arg);
    if (!bhv)
        bhv = behaviorManager->findUnstrict(arg);
        
    DefaultBehavior *defaultBhv;

    if (!bhv || (defaultBhv = dynamic_cast<DefaultBehavior *>(bhv)) == NULL) {
        stc("Поведение с таким названием не найдено, используйте {y{hcbedit list{x для списка.\r\n", ch);
        return;
    }

    OLCStateBehavior::Pointer be(NEW, defaultBhv);
    be->attach(ch);
    be->show(ch);
}
    
    

