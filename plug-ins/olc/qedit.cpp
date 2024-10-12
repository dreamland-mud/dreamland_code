#include <algorithm>

#include "qedit.h"

#include "util/regexp.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"
#include "codesource.h"
#include "pcharacter.h"
#include "room.h"

#include "areautils.h"
#include "areaquestutils.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "act.h"

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
    static const DLString ON("on");
    static const DLString POST("post");

    DLString id = "step " + step.toString() + " " + beginOrEnd + " trigger" ;

    StringList commands;
    for (auto trig: feniaTriggers->getTriggersForTarget(stepType, "queststep")) {
        // Show all triggers for 'begin' stage of the step 0
        if (step == 0 && beginOrEnd == "begin") {
            commands.push_back("$ " + trig);
            continue;
        }

        // Only show async triggers for 'begin' stage starting from the 1st step
        if (beginOrEnd == "begin" && step > 0 && POST.strPrefix(trig)) {
            commands.push_back("$ " + trig);
            continue;
        }

        // Only show sync triggers for 'end' stages
        if (beginOrEnd == "end" && ON.strPrefix(trig)) {
            commands.push_back("$ " + trig);
            continue;
        }
    }
        
    DLString label = stepTrig;

    buf << "{w[{y" << web_menu(commands, id, label) << "{w]";
    return buf.str();
}

static void show_active_triggers(PCharacter *ch, ostringstream& buf, const Integer &s, const DLString &methodId,
                                    const DLString &type, const DLString &vnum, bool isBegin)
{
    Register wrapper = get_wrapper_for_index_data(vnum.toInt(), type);
    Register method = feniaTriggers->findMethodOnWrapper(wrapper, methodId);

    if (method.type == Register::NONE)
        return;

    const CodeSource::Pointer &cs = method.toFunction()->getFunction()->source.source;

    DLString cmd = "cs web " + DLString(cs->getId());
    DLString seeFmt = methodId;
    buf << "{g" << web_cmd(ch, cmd, seeFmt) << "{w ";
}


static void show_available_triggers(PCharacter *ch, ostringstream& buf, const Integer &s, const DLString &methodId,
                                    const DLString &type, const DLString &vnum, bool isBegin)
{
    Register wrapper = get_wrapper_for_index_data(vnum.toInt(), type);
    Register method = feniaTriggers->findMethodOnWrapper(wrapper, methodId);

    if (method.type == Register::FUNCTION)
        return;

    DLString cmd = "step " + s.toString() + " " + (isBegin ? "begin" : "end") + " fenia";
    DLString seeFmt = methodId;
    buf << "{W" << web_cmd(ch, cmd, seeFmt) << "{w ";
}

static DLString aquest_title(const Integer &questId)
{
    if (questId == 0)
        return DLString::emptyString;

    AreaQuest *q = get_area_quest(questId.toString());
    if (q)
        return q->title;

    return "{Rне найден{x";
}

static DLString show_step_reward(PCharacter *ch, const QuestStep::Pointer &step)
{
    ostringstream buf;

    buf << step->rewardExp << " опыта, " << step->rewardGold << " золота, " 
        << step->rewardQp << " кп, предмет ";

    if (step->rewardVnum == 0) {
        buf << step->rewardVnum;

    } else if (step->rewardVnum > 0) {
        OBJ_INDEX_DATA *pObj = get_obj_index(step->rewardVnum);
        if (!pObj)
            buf << step->rewardVnum << " {Rне найден{x";
        else
            buf << "{w[{W" << web_cmd(ch, "oedit $1", step->rewardVnum.toString()) << "{w] "
                << russian_case(pObj->short_descr, '1');
    }

    return buf.str();
}

// Display any warnings for this step, such as mismatched begin/end triggers
static DLString show_step_errors(PCharacter *ch, AreaQuest *q, int s)
{ 
    ostringstream buf;
    auto &step = q->steps[s];

    if (s == q->steps.size() - 1)
        return DLString::emptyString;

    auto &nextStep = q->steps[s+1];

    // Check that triggers are on the same mob/obj
    if (step->endType != nextStep->beginType || step->endValue != nextStep->beginValue)
        buf << "{RОшибка: не совпадает цель у тригеров конца этого шага и начала следующего." << endl;

    // Check for onSpeech/postSpeech pairs
    if (step->endTrigger.substr(2) != nextStep->beginTrigger.substr(4))
        buf << "{RОшибка: не совпадает тип тригеров конца этого шага (" << step->endTrigger << ") " 
            << "и начала следующего (" << nextStep->beginTrigger << ")." << endl;

    return buf.str();
}

static void show_step(PCharacter *ch, AreaQuest *q, int s)
{
    ostringstream buf;
    auto &step = q->steps[s];
    DLString stepNum(s);

    buf << endl
        << "{CШаг " << s << "{x   ";
        
    if (s > 0)
        buf << "{D[" << web_cmd(ch, "step " + stepNum + " up", "вверх") << "]{w ";
    if (s != q->steps.size() - 1)
        buf << "{D[" << web_cmd(ch, "step  " + stepNum + " down", "вниз") << "]{w ";

    buf << endl
        << "{WИнфо{w:    " << step->info << " " << web_edit_button(ch, "step " + stepNum + " info", "web") << endl
        << "{WНаграда{w: " << show_step_reward(ch, step) << endl
        << "{WНачало{w:  тип " << menu_step_type(s, step->beginType, "begin") << "{w, "
        << "триггер " << menu_step_trigger(s, step->beginType, step->beginTrigger, "begin") << "{w, "
        << show_step_target(ch, step->beginType, step->beginValue)
        << endl;

    buf << "{WКонец{w:   тип " << menu_step_type(s, step->endType, "end") << "{w, "
        << "триггер " << menu_step_trigger(s, step->endType, step->endTrigger, "end") << "{w, "
        << show_step_target(ch, step->endType, step->endValue)
        << endl;

    DLString beginMethodId = aquest_method_id(q, s, true, step->beginTrigger);
    DLString endMethodId = aquest_method_id(q, s, false, step->endTrigger);

    {
        ostringstream trigBuf;            
        show_active_triggers(ch, trigBuf, s, beginMethodId, step->beginType, step->beginValue, true);
        show_active_triggers(ch, trigBuf, s, endMethodId, step->endType, step->endValue, false);

        if (!trigBuf.str().empty())
            buf << "{gУстановлены{x: " << trigBuf.str() << endl;
    }

    {
        ostringstream trigBuf;            
        show_available_triggers(ch, trigBuf, s, beginMethodId, step->beginType, step->beginValue, true);
        show_available_triggers(ch, trigBuf, s, endMethodId, step->endType, step->endValue, false);

        if (!trigBuf.str().empty())
            buf << "{WДоступны{x:    " << trigBuf.str() << endl;
    }

    buf << show_step_errors(ch, q, s);

    ch->send_to(buf);
}

static void show_steps(PCharacter *ch, AreaQuest *q)
{
    ostringstream buf;
    buf <<  "{CШаги{w:          [{y" << web_cmd(ch, "step add", "добавить") << "{w] {D(step help){x" << endl;
    ch->send_to(buf);

    for (unsigned int s = 0; s < q->steps.size(); s++)
        show_step(ch, q, s);
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
    ptc(ch, "Натура:        {c%s{x {D(? align){x\r\n",
        q->align.getValue() != 0 ? q->align.names().c_str() : "-");
    ptc(ch, "Классы:        {c%s{x {D(? classes){x\r\n",
            q->classes.empty() ? "-": q->classes.toString().c_str());
    ptc(ch, "Города:        {c%s{x {D(? hometowns){x\r\n",
            q->hometowns.empty() ? "-": q->hometowns.toString().c_str());
    ptc(ch, "Раз за жизнь:  {c%d {D(perlife){x\r\n", q->limitPerLife.getValue());
    ptc(ch, "Раз в день:    {c%s {D(perday){x\r\n", (q->oncePerDay ? "да" : "нет"));
    ptc(ch, "Флаги:         {c%s {D(flags){x\r\n", q->flags.names().c_str());
    ptc(ch, "Предыдущий:    {c%d{x %s {D(prereq){x\r\n", 
             q->prereq.getValue(), aquest_title(q->prereq).c_str());
    
    feniaTriggers->showTriggers(ch, q->getWrapper(), "areaquest");    

    show_steps(ch, q);

    ptc(ch, "\r\nКоманды: {y{hcstep help{x, {y{hccommands{x, {y{hcshow{x, {y{hcdone{x\r\n");
}

static void qedit_step_usage(PCharacter *ch)
{
    ch->pecho("Формат:");
    ch->pecho("step add - добавить новый шаг с установками по умолчанию");
    ch->pecho("step <num> begin|end type mob|obj|room - задать тип начала или завершения шага");
    ch->pecho("step <num> begin|end vnum <vnum> - задать vnum для начала или завершения шага");
    ch->pecho("step <num> begin|end trigger <onXXX> - задать название тригера для начала или завершения шага");
    ch->pecho("step <num> begin|end trigger <onXXX> clear - удалить тригер для начала или завершения шага");
    ch->pecho("step <num> begin|end fenia - открыть редактор феневого сценария");
    ch->pecho("step <num> info <строка> - установить описание шага, видное по 'квест инфо'");
    ch->pecho("step <num> qp|gold|exp|item <число> - награда за выполнение шага в кп/золоте/экспе или внум предмета");
    ch->pecho("step <num> up  - передвинуть шаг выше");
    ch->pecho("step <num> down - передвинуть шаг ниже");
    ch->pecho("step <num> del - удалить шаг");
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

AQEDIT(fenia, "феня", "редактировать тригера")
{
    feniaTriggers->openEditor(ch, getOriginal(), argument);
    return false;
}

// Clears existing trigger begin before assigning new one or changing target type/vnum.
static void clear_step_begin_trigger(PCharacter *ch, AreaQuest *q, int step)
{
    feniaTriggers->openEditor(ch, q, step, true, "kill");
}

// Clears existing trigger end before assigning new one or changing target type/vnum.
static void clear_step_end_trigger(PCharacter *ch, AreaQuest *q, int step)
{
    feniaTriggers->openEditor(ch, q, step, false, "kill");
}

// Clears all triggers for this step, when the steps are swapped.
static void clear_step_triggers(PCharacter *ch, AreaQuest *q, int step)
{
    clear_step_begin_trigger(ch, q, step);
    clear_step_end_trigger(ch, q, step);
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
    if (arg_is(cmd, "add")) {
        QuestStep::XMLPointer newStep(NEW);
        MOB_INDEX_DATA *pMob = AreaUtils::findFirstMob(q->pAreaIndex);
        RoomIndexData *pRoom = AreaUtils::findFirstRoom(q->pAreaIndex);

        if (q->steps.empty()) {
            // Setup defaults for the first step
            newStep->beginType = pMob ? "mob" : pRoom ? "room" : "";
            newStep->beginValue = DLString(pMob ? pMob->vnum : pRoom ? pRoom->vnum : 0);
            newStep->beginTrigger = "postGreet";

        } else {
            // Previous step's settings affect this step defaults.
            auto lastStep = q->steps.back();

            newStep->beginType = lastStep->endType;
            newStep->beginValue = lastStep->endValue;
            newStep->beginTrigger = DLString(lastStep->endTrigger).replaces("on", "post");
        }

        newStep->endType = newStep->beginType;
        newStep->endValue = newStep->beginValue;
        newStep->endTrigger = "onSpeech";

        q->steps.push_back(newStep);
        ch->pecho("Добавлен новый шаг с настройками по умолчанию.");
        show_steps(ch, q);
        return true;
    }

    stepArg = cmd;
    if (!parseStepNumber(ch, stepArg, step))
        return false;

    auto &thisStep = q->steps[step];   
    cmd = args.getOneArgument();     

    // 'step 3 del'
    if (arg_is(cmd, "del")) {
        q->steps.erase(q->steps.begin() + step);
        ch->pecho("Шаг %d удален. Не забудь почистить старые тригера ({y{hccs search %d{x).", 
                  step.getValue(), q->vnum.getValue());
        show_steps(ch, q);
        return true;
    }

    // 'step 3 up'
    if (arg_is(cmd, "up")) {
        if (step == 0) {
            ch->pecho("Шаг %d и так самый первый в списке.", step.getValue());
            return false;
        }

        clear_step_triggers(ch, q, step-1);
        clear_step_triggers(ch, q, step);

        std::swap(q->steps[step-1], q->steps[step]);
        ch->pecho("Шаги %d и %d поменялись местами.", step.getValue(), (int)step-1);
        show_steps(ch, q);
        return true;
    }

    // 'step 3 down'
    if (arg_is(cmd, "down")) {
        if (step == q->steps.size() - 1) {
            ch->pecho("Шаг %d и так самый последний в списке.", step.getValue());
            return false;
        }

        clear_step_triggers(ch, q, step);
        clear_step_triggers(ch, q, step+1);

        std::swap(q->steps[step], q->steps[step+1]);
        ch->pecho("Шаги %d и %d поменялись местами.", step.getValue(), (int)step+1);
        show_steps(ch, q);
        return true;
    }

    // 'step 3 info <string>'
    if (arg_is(cmd, "info")) {
        if (args.empty()) {
            ch->pecho("Какое описание ты хочешь присвоить?");
            return false;
        }

        // Execute 'step 3 paste' rather than 'step paste' when web editor returns
        lastCmd << " " << stepArg;
        return editor(args.c_str(), thisStep->info, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
    }

    // 'step 3 paste'
    if (arg_is_paste(cmd)) {
        editorPaste(thisStep->info, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
        show_step(ch, q, step);
        return true;
    }

    // 'step 3 qp <num>'
    if (arg_is(cmd, "qp")) {
        Integer::tryParse(thisStep->rewardQp, args);
        ch->pecho("Награда за шаг %d установлена в %d qp.", 
                   step.getValue(), thisStep->rewardQp.getValue());
        show_step(ch, q, step);
        return true;
    }

    // 'step 3 gold <num>'
    if (arg_is(cmd, "gold")) {
        Integer::tryParse(thisStep->rewardGold, args);
        ch->pecho("Награда за шаг %d установлена в %d золота.", 
                   step.getValue(), thisStep->rewardGold.getValue());
        show_step(ch, q, step);
        return true;
    }

    // 'step 3 exp <num>'
    if (arg_is(cmd, "exp")) {
        Integer::tryParse(thisStep->rewardExp, args);
        ch->pecho("Награда за шаг %d установлена в %d опыта.", 
                   step.getValue(), thisStep->rewardExp.getValue());
        show_step(ch, q, step);
        return true;
    }

    // 'step 3 vnum <num>'
    if (arg_is(cmd, "item")) {
        Integer::tryParse(thisStep->rewardVnum, args);
        ch->pecho("Награда за шаг %d установлена в предмет %d.", 
                   step.getValue(), thisStep->rewardVnum.getValue());
        show_steps(ch, q);
        return true;
    }

    // 'step 3 begin|end type|vnum|trigger <value>'

    DLString beginOrEnd = cmd;
    bool isBegin;
    if (arg_is(beginOrEnd, "begin")) {
        isBegin = true;
        beginOrEnd = "begin";
    } else if (arg_is(beginOrEnd, "end")) {
        isBegin = false;
        beginOrEnd = "end";
    } else {
        ch->pecho("Укажи, начало это или конец шага (begin, end).");
        return false;        
    }

    DLString action = args.getOneArgument();

    // 'step 3 begin|end type mob|obj|room'
    if (arg_is(action, "type")) {
        DLString type = args.getOneArgument();

        if (arg_is(type, "mob"))
            type = "mob";
        else if (arg_is(type, "obj"))
            type = "obj";
        else if (arg_is(type, "room"))
            type = "room";
        else {
            ch->pecho("Допустимые значения для типа шага: mob, obj, room.");
            return false;
        }

        if (isBegin) {
            if (thisStep->beginType != type)
                clear_step_begin_trigger(ch, q, step);

            thisStep->beginType = type;
            ch->pecho("Началу шага %d присвоен тип %s.", step.getValue(), type.c_str());

        } else {
            if (thisStep->endType != type)
                clear_step_end_trigger(ch, q, step);

            thisStep->endType = type;
            ch->pecho("Концу шага %d присвоен тип %s.", step.getValue(), type.c_str());
        }

        show_steps(ch, q);
        return true;
    }

    // 'step 3 begin|end vnum 4321'
    if (arg_is(action, "vnum")) {
        DLString vnumArg = args.getOneArgument();
        Integer vnum;
        if (!vnumArg.isNumber() || !Integer::tryParse(vnum, vnumArg)) {
            ch->pecho("Укажи численный vnum.");
            return false;
        }

        if (isBegin) {
            if (thisStep->beginValue != vnum.toString())
                clear_step_begin_trigger(ch, q, step);

            thisStep->beginValue = vnum.toString();
            ch->pecho("Началу шага %d присвоен vnum %d.", step.getValue(), vnum.getValue());

        } else {
            if (thisStep->endValue != vnum.toString())
                clear_step_end_trigger(ch, q, step);

            thisStep->endValue = vnum.toString();
            ch->pecho("Концу шага %d присвоен vnum %d.", step.getValue(), vnum.getValue());
        }

        show_steps(ch, q);
        return true;
    }

    // 'step 3 begin|end trigger onSpeech'
    if (arg_is(action, "triger")) {
        DLString trigName = args.getOneArgument();        
        auto allTriggers = feniaTriggers->getTriggersForTarget(isBegin ? thisStep->beginType : thisStep->endType, "queststep");

        if (allTriggers.count(trigName) == 0) {
            ch->pecho("Триггер %s не найден в списке допустимых тригеров: %s", 
                      trigName.c_str(), allTriggers.toString().c_str());
            return false;
        }

        if (isBegin) {
            if (thisStep->beginTrigger != trigName)
                clear_step_begin_trigger(ch, q, step);

            thisStep->beginTrigger = trigName;
            ch->pecho("Началу шага %d присвоен триггер %s.", step.getValue(), trigName.c_str());

        } else {
            if (thisStep->endTrigger != trigName)
                clear_step_end_trigger(ch, q, step);

            thisStep->endTrigger = trigName;
            ch->pecho("Концу шага %d присвоен триггер %s.", step.getValue(), trigName.c_str());
        }
        
        show_steps(ch, q);
        return true;
    }

    // 'step 3 begin|end fenia [clear]'
    if (arg_is(action, "fenia")) {
        feniaTriggers->openEditor(ch, q, step, isBegin, args);
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

AQEDIT(align, "натура", "ограничить по натуре")
{
    return flagBitsEdit(getOriginal()->align);
}

AQEDIT(classes, "классы", "ограничить по классам")
{
    return globalBitvectorEdit<Profession>(getOriginal()->classes);
}

AQEDIT(hometowns, "города", "ограничить по домашним городам")
{
    return globalBitvectorEdit<Hometown>(getOriginal()->hometowns);
}

AQEDIT(perlife, "зажизнь", "ограничить выполнение за один реморт, -1 без ограничений")
{
    AreaQuest *q = getOriginal();
    return numberEdit(-1, 100, (int &)q->limitPerLife);
}

AQEDIT(perday, "задень", "ограничить выполнение одним разом за сутки")
{
    AreaQuest *q = getOriginal();
    return boolEdit(q->oncePerDay);
}

AQEDIT(desc, "описание", "редактор описания квеста (desc help)")
{
    AreaQuest *q = getOriginal();

    return editor(argument, q->description);
}

AQEDIT(flags, "флаги", "флаги квеста (? areaquest_flags)")
{
    return flagBitsEdit(getOriginal()->flags);
}

AQEDIT(prereq, "предыдущий", "номер предыдущего квест в цепочке")
{
    AreaQuest *q = getOriginal();
    Integer prereqId;

    numberEdit(0, 900000, prereqId);

    if (prereqId == 0) {
        ch->pecho("Предыдущий квест очищен.");
        q->prereq.setValue(prereqId);
        return true;
    }

    AreaQuest *prereq = get_area_quest(prereqId);

    if (!prereq) {
        ch->pecho("Квест с номером %d не существует.", prereqId.getValue());
        return false;
    }
    
    ch->pecho("Предыдущий квест в цепочке теперь [%d] %s.", prereqId.getValue(), prereq->title.c_str());
    q->prereq.setValue(prereqId);
    return true;
}


AQEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

AQEDIT(commands, "команды", "показать список встроенных команд qedit")
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
    if (arg_is(cmd, "create")) {
        ch->pecho("Зайди в редактор зоны ({y{hcaedit{x) и набери {y{hcquest create{x.");
        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(fmt(0, "{C%-6s %-7s %-3s %1s %s (%s){x\r\n", "Номер", "Уровни", "Нат", "!", "Квест", "Зона"));

        const DLString lineFormat = 
            "{C" + web_cmd(ch, "qedit $1", "%-6d") + "{w %-7s %-3s{x %1s %s%s {W({x%s{W){x";

        for (auto q: areaQuests) {
            AreaQuest *aq = q.second;

            DLString levels;
            if (aq->maxLevel > 0 || aq->minLevel > 0)
                levels << aq->minLevel << "-" << aq->maxLevel;

            DLString align;
            if (aq->align != 0) {
                align << (aq->align.isSet(F_ALIGN_GOOD) ? "{YG" : " ");
                align << (aq->align.isSet(F_ALIGN_NEUTRAL) ? "{wN" : " ");
                align << (aq->align.isSet(F_ALIGN_EVIL) ? "{RE" : " ");
            }

            DLString limits;
            if (!aq->classes.empty() || !aq->hometowns.empty())
                limits = "*";
            if (aq->getWrapper() && aq->getWrapper()->hasTrigger("CanStart"))
                limits = "*";

            DLString qnameColour = aq->flags.isSet(AQUEST_HIDDEN) ? "{D" : "{w";
            DLString qname =  aq->flags.isSet(AQUEST_HIDDEN) ? aq->title.colourStrip() : aq->title;

            ch->pecho(lineFormat.c_str(),
                       q.first,
                       levels.c_str(),
                       align.c_str(),
                       limits.c_str(),
                       qnameColour.c_str(),
                       qname.c_str(),
                       aq->pAreaIndex->getName().c_str());
        }

        return;
    }

    if (cmd.isNumber() && Integer::tryParse(vnum, cmd)) {
        auto q = areaQuests.find(vnum);

        if (q == areaQuests.end()) {
            ch->pecho("Квест под номером %d не задан.\r\n", vnum.getValue());
            return;
        }

        OLCStateAreaQuest::Pointer qe(NEW, q->second);
        qe->attach(ch);
        qe->show(ch);
        return;
    }

    ch->pecho("Неверная подкоманда. Формат: qedit номер, qedit list.");
}
    
    

