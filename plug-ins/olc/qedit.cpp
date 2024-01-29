#include <algorithm>

#include "qedit.h"

#include "util/regexp.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"
#include "codesource.h"
#include "pcharacter.h"
#include "room.h"

#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

OLC_STATE(OLCStateAreaQuest);

OLCStateAreaQuest::OLCStateAreaQuest() : isChanged(false)
{
}

OLCStateAreaQuest::OLCStateAreaQuest(AreaQuest *q) 
    : isChanged(false)
{
    if (!q)
        return;

    vnum = q->vnum;
}

OLCStateAreaQuest::~OLCStateAreaQuest() 
{
}

void OLCStateAreaQuest::commit() 
{
    if (!isChanged)
        return;

    AreaQuest *q = getOriginal();
    if (!q)
        return;
    
    q->pAreaIndex->changed = true;

    if (owner) {
        owner->character->pecho("Зона '%s' помечена для сохранения.", q->pAreaIndex->getName().c_str());
    }
}

AreaQuest * OLCStateAreaQuest::getOriginal()
{
    auto q = areaQuests.find(vnum);

    if (q == areaQuests.end())
        throw ::Exception("Attached area quest doesn't exist");
    
    return q->second;
}

void OLCStateAreaQuest::statePrompt(Descriptor *d) 
{
    d->send( "Quest> " );
}

void OLCStateAreaQuest::changed( PCharacter *ch )
{
    isChanged = true;
}

// Output step target's name with a clickable link to OLC editor.
static DLString show_step_target(PCharacter *ch, const DLString &type, const Integer &vnum)
{
    ostringstream buf;

    if (type == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (!pMob) {
            buf << "{Rнеизвестный моб " << vnum << "{x";
            return buf.str();
        }

        buf << "{w[{W" << web_cmd(ch, "medit $1", vnum.toString()) << "{w] "
            << russian_case(pMob->short_descr, '1');
        return buf.str();
    }

    if (type == "obj") {
        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        if (!pObj) {
            buf << "{Rнеизвестный предмет " << vnum << "{x";
            return buf.str();
        }

        buf << "{w[{W" << web_cmd(ch, "oedit $1", vnum.toString()) << "{w] "
            << russian_case(pObj->short_descr, '1');
        return buf.str();
    }

    if (type == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        if (!pRoom) {
            buf << "{Rнеизвестная комната " << vnum << "{x";
            return buf.str();
        }

        buf << "{w[{W" << web_cmd(ch, "redit $1", vnum.toString()) << "{w] "
            << pRoom->name;

        return buf.str();
    }

    buf << "{Rнеизвестный тип{x";
    return buf.str();
}

static DLString menu_step_type(Integer step, const DLString &stepType, DLString beginOrEnd)
{
    ostringstream buf;

    DLString id = "step " + step.toString() + " " + beginOrEnd + " type" ;
    StringList commands;
    commands.push_back("$ mob");
    commands.push_back("$ obj");
    commands.push_back("$ room");
    DLString label = stepType;

    buf << "{w[{y" << web_menu(commands, id, label) << "{w]";
    return buf.str();
}

static DLString menu_step_trigger(Integer step, const DLString &stepType, const DLString &stepTrig, DLString beginOrEnd)
{
    ostringstream buf;

    DLString id = "step " + step.toString() + " " + beginOrEnd + " trigger" ;
    StringList commands;
    for (auto trig: feniaTriggers->getQuestTriggers(stepType))
        commands.push_back("$ " + trig);
    DLString label = stepTrig;

    buf << "{w[{y" << web_menu(commands, id, label) << "{w]";
    return buf.str();
}

Register find_function(const DLString &type, const Integer &vnum, const DLString &trigName)
{
    Register wrapper = get_wrapper_for_index_data(vnum, type);

    if (wrapper.type == Register::NONE)
        return Register();

    WrapperBase *wbase = get_wrapper(wrapper.toObject());
    if (!wbase)
        return Register();

    Scripting::IdRef methodId(trigName);
    Register retval = wbase->getField(methodId);
    if (retval.type != Register::FUNCTION)
        return Register();

    return retval;
}

// Checks method body for quoted quest ID - as a hint that it's a quest trigger
bool trigger_is_active(Register method, AreaQuest *q)
{
    if (method.type == Register::NONE)
        return false;

    DLString methodBody = method.repr();
    if (methodBody.find("\"" + q->vnum.toString() + "\"") == DLString::npos)
        return false;

    return true;
}

static DLString cs_label(const DLString &type, const Integer &vnum, const DLString &trigName) 
{
    return type + "/" + vnum.toString() + "/" + trigName;
}

static void show_active_triggers(PCharacter *ch, ostringstream& buf, AreaQuest *q, const DLString &type, const Integer &vnum, const DLString &trigName)
{
    Register method = find_function(type, vnum, trigName);

    if (!trigger_is_active(method, q))
        return;

    const CodeSource::Pointer &cs = method.toFunction()->getFunction()->source.source;

    DLString cmd = "cs web " + DLString(cs->getId());
    DLString seeFmt = cs_label(type, vnum, trigName);
    buf << "{g" << web_cmd(ch, cmd, seeFmt) << "{w ";
}

static void show_available_triggers(PCharacter *ch, ostringstream& buf, AreaQuest *q, const DLString &type, 
                                    const Integer &vnum, const DLString &trigName, const Integer &step, bool isBegin)
{
    Register method = find_function(type, vnum, trigName);

    if (trigger_is_active(method, q))
        return;

    DLString cmd = "step " + step.toString() + " " + (isBegin ? "begin" : "end") + " fenia";
    DLString seeFmt = cs_label(type, vnum, trigName);
    buf << "{W" << web_cmd(ch, cmd, seeFmt) << "{w ";
}

void OLCStateAreaQuest::show( PCharacter *ch )
{
    AreaQuest *q = getOriginal();
    AreaIndexData *area = q->pAreaIndex;

    ptc(ch, "Квест {C%d{x зоны {W%s{x {x\r\n", 
             q->vnum.getValue(), area->getName().c_str());

    ptc(ch, "Титул:         {C%s{x %s {D(title help){x\r\n",
            q->title.c_str(),
            web_edit_button(ch, "title", "web").c_str());
    ptc(ch, "Описание:      %s {D(desc help){x\r\n%s",
            web_edit_button(ch, "desc", "web").c_str(),
            q->description.c_str());
    ptc(ch, "Мин. уровень:  {c%d {D(minlevel){x\r\n", q->minLevel.getValue());
    ptc(ch, "Макс. уровень: {c%d {D(maxlevel){x\r\n", q->maxLevel.getValue());
    ptc(ch, "Раз за жизнь:  {c%d {D(perlife){x\r\n", q->limitPerLife.getValue());

    {
        ostringstream buf;
        buf <<  "{CШаги{w:          [{y" << web_cmd(ch, "step add", "добавить") << "{w] {D(step help){x" << endl;
        ch->send_to(buf);
    }

    for (unsigned int s = 0; s < q->steps.size(); s++) {
        ostringstream buf;
        auto &step = q->steps[s];

        buf << endl
            << "{CШаг " << s << "{x   ";
            
        if (s > 0)
            buf << "{D[" << web_cmd(ch, "step up $1", "вверх") << "]{w ";
        if (s != q->steps.size() - 1)
            buf << "{D[" << web_cmd(ch, "step down $1", "вниз") << "]{w ";

        buf << endl
            << "{WНачало{w: тип " << menu_step_type(s, step->beginType, "begin") << "{w, "
            << "триггер " << menu_step_trigger(s, step->beginType, step->beginTrigger, "begin") << "{w, "
            << show_step_target(ch, step->beginType, step->beginValue)
            << endl;

        buf << "{WКонец{w:  тип " << menu_step_type(s, step->endType, "end") << "{w, "
            << "триггер " << menu_step_trigger(s, step->endType, step->endTrigger, "end") << "{w, "
            << show_step_target(ch, step->endType, step->endValue)
            << endl;

        {
            ostringstream trigBuf;            
            show_active_triggers(ch, trigBuf, q, step->beginType, step->beginValue, step->beginTrigger);
            show_active_triggers(ch, trigBuf, q, step->endType, step->endValue, step->endTrigger);

            if (!trigBuf.str().empty())
                buf << "{gУстановлены{x: " << trigBuf.str() << endl;
        }

        {
            ostringstream trigBuf;            
            show_available_triggers(ch, trigBuf, q, step->beginType, step->beginValue, step->beginTrigger, s, true);
            show_available_triggers(ch, trigBuf, q, step->endType, step->endValue, step->endTrigger, s, false);

            if (!trigBuf.str().empty())
                buf << "{WДоступны{x:    " << trigBuf.str() << endl;
        }

        ch->send_to(buf);
    }

    ptc(ch, "\r\nКоманды: {y{hcstep help{x, {y{hccommands{x, {y{hcshow{x, {y{hcdone{x\r\n");
}

static void qedit_step_usage(PCharacter *ch)
{
    ch->pecho("Формат:");
    ch->pecho("step add - добавить новый шаг с установками по умолчанию");
    ch->pecho("step <num> begin|end type mob|obj|room - задать тип начала или завершения шага");
    ch->pecho("step <num> begin|end vnum <vnum> - задать vnum для начала или завершения шага");
    ch->pecho("step <num> begin|end trigger <onXXX> - задать название тригера для начала или завершения шага");
    ch->pecho("step <num> begin|end fenia - открыть редактор феневого сценария");
    ch->pecho("step del <num>  - удалить шаг");
    ch->pecho("step up <num>   - передвинуть шаг выше");
    ch->pecho("step down <num> - передвинуть шаг ниже");
}

bool OLCStateAreaQuest::parseQuestVnum(PCharacter *ch, const DLString &arg, Integer &vnum)
{
    return false;

}

bool OLCStateAreaQuest::parseStepNumber(PCharacter *ch, const DLString &arg, Integer &step)
{
    AreaQuest *q = getOriginal();

    if (arg.isNumber() && Integer::tryParse(step, arg)) {
        if (step < 0) {
            ch->pecho("Номер шага не может быть меньше нуля.");
            return false;
        }

        if (step >= q->steps.size()) {
            ch->pecho("У этого квеста нету столько шагов.");
            return false;
        }

        return true;
    }

    ch->pecho("Укажи номер квеста (см. {y{hcstep help{x).");
    return false;
}


AQEDIT(step, "шаг", "редактор шагов квеста")
{
    AreaQuest *q = getOriginal();
    DLString args = argument;
    DLString cmd = args.getOneArgument();
    DLString stepArg;
    Integer step;

    if (arg_is_help(cmd)) {
        qedit_step_usage(ch);
        return false;
    }

    // 'step add'
    if (arg_oneof(cmd, "add", "добавить")) {
        QuestStep::XMLPointer newStep(NEW);

        newStep->beginType = "mob";
        newStep->beginValue = DLString(q->pAreaIndex->min_vnum);
        newStep->beginTrigger = "onGive";
        newStep->endType = newStep->beginType;
        newStep->endValue = newStep->beginValue;
        newStep->endTrigger = newStep->beginTrigger;

        q->steps.push_back(newStep);
        ch->pecho("Добавлен новый шаг с настройками по умолчанию.");
        return true;
    }

    // 'step del 3'
    if (arg_oneof(cmd, "delete", "удалить")) {
        stepArg = args.getOneArgument();
        if (!parseStepNumber(ch, stepArg, step))
            return false;

        q->steps.erase(q->steps.begin() + step);
        ch->pecho("Шаг %d удален. Не забудь почистить старые тригера ({y{hccs search %d{x).", 
                  step.getValue(), q->vnum.getValue());
        return true;
    }

    // 'step up 3'
    if (arg_oneof(cmd, "up", "вверх")) {
        stepArg = args.getOneArgument();
        if (!parseStepNumber(ch, stepArg, step))
            return false;

        if (step == 0) {
            ch->pecho("Шаг %d и так самый первый в списке.", step.getValue());
            return false;
        }

        std::swap(q->steps[step-1], q->steps[step]);
        ch->pecho("Шаги %d и %d поменялись местами.", step.getValue(), (int)step-1);
        return true;
    }

    // 'step down 3'
    if (arg_oneof(cmd, "down", "вниз")) {
        stepArg = args.getOneArgument();
        if (!parseStepNumber(ch, stepArg, step))
            return false;

        if (step == q->steps.size() - 1) {
            ch->pecho("Шаг %d и так самый последний в списке.", step.getValue());
            return false;
        }

        std::swap(q->steps[step], q->steps[step+1]);
        ch->pecho("Шаги %d и %d поменялись местами.", step.getValue(), (int)step+1);
        return true;
    }

    // 'step 3 begin|end type|vnum|trigger <value>'
    stepArg = cmd;
    if (!parseStepNumber(ch, stepArg, step))
        return false;

    auto &thisStep = q->steps[step];        

    DLString beginOrEnd = args.getOneArgument();
    bool isBegin;
    if (arg_oneof(beginOrEnd, "begin", "начало")) {
        isBegin = true;
        beginOrEnd = "begin";
    } else if (arg_oneof(beginOrEnd, "end", "конец")) {
        isBegin = false;
        beginOrEnd = "end";
    } else {
        ch->pecho("Укажи, начало это или конец шага (begin, end).");
        return false;        
    }

    DLString action = args.getOneArgument();

    // 'step 3 begin|end type mob|obj|room'
    if (arg_oneof(action, "type", "тип")) {
        DLString type = args.getOneArgument();

        if (arg_oneof(type, "mob", "моб"))
            type = "mob";
        else if (arg_oneof(type, "obj", "обж", "предмет"))
            type = "obj";
        else if (arg_oneof(type, "room", "комната"))
            type = "room";
        else {
            ch->pecho("Допустимые значения для типа шага: mob, obj, room.");
            return false;
        }

        if (isBegin) {
            thisStep->beginType = type;
            ch->pecho("Началу шага %d присвоен тип %s.", step.getValue(), type.c_str());
        } else {
            thisStep->endType = type;
            ch->pecho("Концу шага %d присвоен тип %s.", step.getValue(), type.c_str());
        }

        return true;
    }

    // 'step 3 begin|end vnum 4321'
    if (arg_oneof(action, "vnum", "внум")) {
        DLString vnumArg = args.getOneArgument();
        Integer vnum;
        if (!vnumArg.isNumber() || !Integer::tryParse(vnum, vnumArg)) {
            ch->pecho("Укажи численный vnum.");
            return false;
        }

        if (isBegin) {
            thisStep->beginValue = vnum.toString();
            ch->pecho("Началу шага %d присвоен vnum %d.", step.getValue(), vnum.getValue());
        } else {
            thisStep->endValue = vnum.toString();
            ch->pecho("Концу шага %d присвоен vnum %d.", step.getValue(), vnum.getValue());
        }
        return true;
    }

    // 'step 3 begin|end trigger onSpeech'
    if (arg_oneof(action, "triger", "trigger", "тригер", "триггер")) {
        DLString trigName = args.getOneArgument();        
        auto allTriggers = feniaTriggers->getQuestTriggers(isBegin ? thisStep->beginType : thisStep->endType);

        if (allTriggers.count(trigName) == 0) {
            ch->pecho("Триггер %s не найден в списке допустимых тригеров: %s", 
                      trigName.c_str(), allTriggers.toString().c_str());
            return false;
        }

        if (isBegin) {
            thisStep->beginTrigger = trigName;
            ch->pecho("Началу шага %d присвоен триггер %s.", step.getValue(), trigName.c_str());
        } else {
            thisStep->endTrigger = trigName;
            ch->pecho("Концу шага %d присвоен триггер %s.", step.getValue(), trigName.c_str());
        }
         
        return true;
    }

    // 'step 3 begin|end fenia'
    if (arg_oneof(action, "fenia", "феня")) {
        feniaTriggers->openEditor(ch, q, step, isBegin);
        return false;
    }

    qedit_step_usage(ch);
    return false;
}

AQEDIT(title, "титул", "название квеста")
{
    AreaQuest *q = getOriginal();
    return editor(argument, q->title, ED_NO_NEWLINE);
}

AQEDIT(minlevel, "минуровень", "нижний уровень игрока")
{
    AreaQuest *q = getOriginal();
    return numberEdit(-1, MAX_LEVEL, (int &)q->minLevel);
}

AQEDIT(maxlevel, "максуровень", "верхний уровень игрока")
{
    AreaQuest *q = getOriginal();
    return numberEdit(-1, MAX_LEVEL, (int &)q->maxLevel);
}

AQEDIT(perlife, "зажизнь", "ограничить выполнение за один реморт, -1 без ограничений")
{
    AreaQuest *q = getOriginal();
    return numberEdit(-1, 100, (int &)q->limitPerLife);
}

AQEDIT(desc, "описание", "редактор описания квеста (desc help)")
{
    AreaQuest *q = getOriginal();

    return editor(argument, q->description);
}


AQEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

AQEDIT(commands, "команды", "показать список встроенных команд cmdedit")
{
    do_commands(ch);
    return false;
}

AQEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

CMD(qedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online area quest editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();
    Integer vnum;

    if (cmd.empty()) {
        stc("Формат:  qedit номер\r\n", ch);
        stc("         qedit list\r\n", ch);
        return;
    }

    // Creating new quest - only via aedit
    if (arg_oneof(cmd, "create", "создать")) {
        ch->pecho("Зайди в редактор зоны ({y{hcaedit{x) и набери {y{hcquest create{x.");
        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(dlprintf("{C%-6s %-15s %s{x\r\n", "Номер", "Квест", "Зона"));

        const DLString lineFormat = 
            "{C" + web_cmd(ch, "qedit $1", "%-6d") + "{w %-15s %s{x";

        for (auto q: areaQuests) {
            ch->pecho(lineFormat.c_str(),
                       q.first,
                       q.second->title.c_str(),
                       q.second->pAreaIndex->getName().c_str());
        }

        return;
    }

    if (cmd.isNumber() && Integer::tryParse(vnum, cmd)) {
        auto q = areaQuests.find(vnum);

        if (q == areaQuests.end()) {
            ch->pecho("Квест под номером %d не задан.\r\n", vnum.getValue());
            return;
        }

        OLCStateAreaQuest::Pointer ce(NEW, q->second);
        ce->attach(ch);
        ce->show(ch);
        return;
    }

    ch->pecho("Неверная подкоманда. Формат: qedit номер, qedit list.");
}
    
    

