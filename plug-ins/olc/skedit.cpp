
#include "util/regexp.h"
#include "wrapperbase.h"
#include "stringset.h"
#include "pcharacter.h"
#include "skillhelp.h"
#include "skillgroup.h"
#include "skillmanager.h"
#include "defaultspell.h"
#include "basicskill.h"
#include "xmltableloader.h"

#include "skedit.h"
#include "hedit.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"

#include "merc.h"
#include "websocketrpc.h"
#include "gsn_plugin.h"
#include "arg_utils.h"
#include "weapontier.h"
#include "damageflags.h"
#include "commandflags.h"
#include "act.h"
#include "comm.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateSkill);

DLString print_damage_tiers(int tier, int level_step);

OLCStateSkill::OLCStateSkill() : isChanged(false)
{
}

OLCStateSkill::OLCStateSkill(Skill *skill) 
    : isChanged(false)
{
    if (!original)
        return;

    original = skill->getIndex();
}

OLCStateSkill::~OLCStateSkill() 
{
}

void OLCStateSkill::commit() 
{
    if (!isChanged)
        return;

    BasicSkill *original = getOriginal();
    if (!original)
        return;
    
    original->save();
    if (owner)
        owner->character->println("Изменения сохранены на диск.");
}

BasicSkill * OLCStateSkill::getOriginal()
{
    Skill *skill = skillManager->find(original->getIndex());
    if (!skill)
        throw Exception("Attached skill doesn't exist");

    BasicSkill *basicSkill = dynamic_cast<BasicSkill *>(skill);
    if (!basicSkill)
        throw Exception("Attached skill was unloaded");
    
    return basicSkill;
}

DefaultSpell* OLCStateSkill::getSpell(BasicSkill *skill)
{
    if (!skill)
        skill = getOriginal();

    if (skill->spell)
        return skill->spell.getDynamicPointer<DefaultSpell>();

    return 0;
}

bool OLCStateSkill::checkSpell(DefaultSpell *spell) 
{
    if (!spell) {
        if (owner)
            stc("Это поле определено только для заклинаний, создайте заклинание командой {hc{yspell create{x.\r\n", owner->character);
        return false;
    }

    return true;
}

void OLCStateSkill::statePrompt(Descriptor *d) 
{
    d->send( "Skill> " );
}

void OLCStateSkill::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateSkill::show( PCharacter *ch )
{
    BasicSkill *r = getOriginal();
    DefaultSpell *s = getSpell(r);

    ptc(ch, "Умение:      {C%s\r\n", r->getName().c_str());
    ptc(ch, "По-русски:   {C%s{x %s {D(russian help){x\r\n",
            r->getRussianName().c_str(), 
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Группы:      {C%s {D(group){x\r\n", r->getGroups().toString().c_str());
    ptc(ch, "Задержка:    {C%d{w пульсов {D(beats){x\r\n", r->getBeats());
    ptc(ch, "Мана и шаги: {C%d {D(mana) {C%d {D(move){x \r\n", r->getMana(), r->move.getValue());
    ptc(ch, "Характер:    {C%s {D(align) {C%s {D(ethos){x\r\n",
        r->align.getValue() != 0 ? r->align.names().c_str() : "-",
        r->ethos.getValue() != 0 ? r->ethos.names().c_str() : "-");

    if (r->help)
        ptc(ch, "Справка:     %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", r->help->getID()).c_str(),
            r->help->getID());
    else
        ptc(ch, "Справка:     нет {D(help create){x\r\n");

    ptc(ch, "Доступно:    {c%s{x %s {D(allow help){x\r\n",
            r->accessToString().c_str(),
            web_edit_button(ch, "allow", "").c_str());

    if (s) {
        ptc(ch, "Цели:        {Y%s {D(target){x\r\n", s->target.names().c_str());                        
        ptc(ch, "Позиция:     {Y%s {D(position){x\r\n", s->position.name().c_str());
        ptc(ch, "Тип:         {Y%s {D(type){x\r\n", s->type.name().c_str());
        ptc(ch, "Флаги:       {Y%s {D(flags){x\r\n", s->flags.names().c_str());
        ptc(ch, "Приказ:      {Y%s {D(order){x\r\n",
                s->order == 0 ? "-" : s->order.names().c_str());
                
        if (s->type == SPELL_OFFENSIVE) {
            ptc(ch, "Тип урона:   {Y%s {D(damtype){x\r\n", s->damtype.name().c_str());
            ptc(ch, "Флаги урона: {Y%s {D(damflags){x\r\n", s->damflags.names().c_str());
            ptc(ch, "Крутость:    {Y%d {D(tier){x\r\n", s->tier.getValue());

            if (s->target.isSet(TAR_CHAR_ROOM))
                ptc(ch, "Ранговое:    {Y%s {D(ranged){x\r\n", s->ranged ? "yes" : "no");
        }

        ptc(ch, "Триггера:    ");
        feniaTriggers->showAvailableTriggers(ch, s);
    }
    
    ptc(ch, "\r\n{WКоманды{x: {hc{yspell{x, {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

SKEDIT(spell, "заклинание", "создать заклинание для этого умения")
{
    DLString arg = argument;

    if (arg_oneof(arg, "create", "создать")) {
        if (getSpell()) {
            stc("Заклинание уже определено.\r\n", ch);
            return false;
        }

        DefaultSpell::Pointer spell(NEW);
        BasicSkill *skill = getOriginal();

        skill->spell.setPointer(spell);
        skill->spell->setSkill(BasicSkill::Pointer(skill));
        stc("Создано новое заклинание для этого умения.\r\n", ch);
        show(ch);
        return true;
    }

    if (arg_oneof(arg, "tiers", "крутость")) {
        DefaultSpell *s = getSpell();        
        if (!checkSpell(s))
            return false;

        ostringstream buf;
        int level_step = 10;

        StringList levels;
        for (int lev = 0; lev <= MAX_LEVEL; lev += level_step)
            levels.push_back(fmt(0, "{C%2d{x", lev));
        buf << "{cLevel {x: " << levels.join(", ") << endl;

        for (int tier = BEST_TIER; tier <= WORST_TIER; tier++) {
            buf << "{cTier {C" << tier << "{x: " << print_damage_tiers(tier, level_step) << endl;
        }

        ch->send_to(buf);
        return true;
    }

    stc("Использование: {y{hcspell create{x - создать заклинание\r\n", ch);
    stc("               {y{hcspell tiers{x  - показать таблицу повреждений\r\n", ch);
    return false;
}

SKEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

SKEDIT(fenia, "феня", "редактировать тригера")
{
    DLString args = argument;
    DefaultSpell *s = getSpell();
    if (!checkSpell(s))
        return false;

    DLString trigName = args.getOneArgument();
    bool clear = arg_is_clear(args);

    if (trigName.empty() || (!args.empty() && !clear)) {
        stc("Использование: fenia <триггер> - редактировать триггер.\r\n", ch);
        stc("               fenia <триггер> clear - очистить триггер.\r\n", ch);
        return false;
    }

    if (clear) {
        if (feniaTriggers->clearTrigger(s->wrapper, trigName))
            ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
        else
            ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
    } else {
        feniaTriggers->openEditor(ch, s, trigName);
    }
    
    return false;
}

SKEDIT(help, "справка", "создать или посмотреть справку по умению")
{
    DLString arg = argument;
    BasicSkill *r = getOriginal();

    if (arg.empty()) {
        if (!r->help || r->help->getID() < 1) {
            ptc(ch, "Справка не задана, используй help create для создания новой.");
            return false;
        }

        OLCStateHelp::Pointer hedit(NEW, r->help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }

    if (arg_oneof(arg, "create", "создать")) {
        if (r->help && r->help->getID() > 0) {
            ptc(ch, "Справка уже существует, используй команду help для редактирования.");
            return false;
        }

        if (!r->help)
            r->help.construct();
        r->help->setID(
            help_next_free_id()
        );
        r->help->setSkill(BasicSkill::Pointer(r));

        OLCStateHelp::Pointer hedit(NEW, r->help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }   

    ptc(ch, "Использование: help, help create\r\n");
    return false;
}

SKEDIT(beats, "задержка", "wait state в пульсах (секунды * 4)")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 60, r->beats);
}

SKEDIT(mana, "мана", "расход маны")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 500, r->mana);
}

SKEDIT(move, "шаги", "расход шагов")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 500, r->move);
}

SKEDIT(group, "группа", "группа умений (? practicer)")
{
    BasicSkill *r = getOriginal();
    return globalBitvectorEdit<SkillGroup>(r->getGroups());
}

SKEDIT(russian, "русское", "русское имя умения")
{
    BasicSkill *r = getOriginal();
    return editor(argument, r->nameRus, ED_NO_NEWLINE);
}

SKEDIT(allow, "доступно", "ограничения по классу, клану, расе")
{
    BasicSkill *r = getOriginal();
    DLString args(argument);
    DLString arg = args.getOneArgument();

    if (arg.empty()) {
        // Launch web editor.
        editorWeb(r->accessToString(), "allow paste");
        return false;
    }

    if (arg_is_help(arg)) {
        // Show usage.
        stc("Использование:\r\n", ch);
        stc("    allow - запустить веб-редактор ограничений\r\n", ch);
        stc("    allow paste - установить ограничения из буфера веб-редактора\r\n", ch);
        stc("    allow <string> - установить ограничения из строки\r\n", ch);
        return false;
    }

    DLString newValue;
    if (arg_is_paste(arg)) {
        // Grab value from the editor buffer.
        editorPaste(newValue, ED_NO_NEWLINE);
    } else {
        // Grab value from command argument.
        newValue = argument;
    }

    // Try to assign restrictions to the skill and report back errors.
    ostringstream errBuf;
    bool rc = r->accessFromString(newValue, errBuf);
    ch->send_to(errBuf);
    return rc;
}


SKEDIT(align, "натура", "ограничить по натуре")
{
    return flagBitsEdit(align_table, getOriginal()->align);
}

SKEDIT(ethos, "этос", "ограничить по этосу")
{
    return flagBitsEdit(ethos_table, getOriginal()->ethos);
}

SKEDIT(target, "цели", "цели заклинания (? target_table)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(target_table, s->target);
}

SKEDIT(flags, "флаги", "флаги заклинания (? spell_flags)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(spell_flags, s->flags);
}

SKEDIT(damtype, "уронтип", "вид повреждений (? damage_table)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagValueEdit(damage_table, s->damtype);
}

SKEDIT(damflags, "уронфлаги", "флаги урона, помимо DAMF_SPELL (? damage_flags)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(damage_flags, s->damflags);
}

SKEDIT(order, "приказ", "кому можно приказать колдовать (? order_flags)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(order_flags, s->order);
}

SKEDIT(tier, "крутость", "уровень повреждений от 1 до 5")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s) 
            && numberEdit(BEST_TIER, WORST_TIER, s->tier);
}

SKEDIT(position, "позиция", "мин. положение тела для заклинания (? position_table)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagValueEdit(position_table, s->position);
}

SKEDIT(type, "вид", "вид заклинания (? spell_types)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagValueEdit(spell_types, s->type);
}

SKEDIT(ranged, "ранговое", "можно ли колдовать на расстоянии")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && boolEdit(s->ranged);
}

SKEDIT(commands, "команды", "показать список встроенных команд")
{
    do_commands(ch);
    return false;
}

SKEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}


CMD(skedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online skill editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  skedit умение\r\n", ch);
        stc("Формат:  skedit create class|clan|race|other <название по-английски>\r\n", ch);
        stc("Формат:  skedit list [all|active|passive|magic|prayer|<group>]\r\n", ch);
        return;
    }

    // Creating new skill.
    if (arg_oneof(cmd, "create", "создать")) {
        DLString type = args.getOneArgument();
        DLString className;
        BasicSkill::Pointer newSkill;
        XMLTableLoader *loader = 0;

        // Figure out class name via 'reflection', without depending on corresponding plugin directly.
        // Figure out who loads that type of skills, by looking at a typical example.
        if (arg_oneof(type, "class", "класс")) {
            className = "GenericSkill";
            loader = dynamic_cast<BasicSkill *>(gsn_sanctuary.getElement())->getLoader();
        } else if (arg_oneof(type, "clan", "клан")) {
            className = "ClanSkill";
            loader = dynamic_cast<BasicSkill *>(gsn_transform.getElement())->getLoader();
        } else if (arg_oneof(type, "race", "раса")) {
            className = "RaceAptitude";
            loader = dynamic_cast<BasicSkill *>(gsn_rear_kick.getElement())->getLoader();
        } else if (arg_oneof(type, "other", "другое", "разное")) {
            className = "BasicSkill";
            loader = dynamic_cast<BasicSkill *>(gsn_kassandra.getElement())->getLoader();
        } else {
            stc("Укажи вид нового умения: class, clan, race или other.\r\n", ch);
            return;
        }

        static RegExp namePattern("^[a-z ]{2,}[a-z]$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("В названии умения могут быть только маленькие англ буквы и пробелы.\r\n", ch);
            return;
        }

        if (skillManager->findExisting(args)) {
            stc("Умение с таким названием уже существует.\r\n", ch);
            return;
        }

        try {
            AllocateClass::Pointer alloc = Class::allocateClass(className);
            newSkill = alloc.getDynamicPointer<BasicSkill>();
        }
        catch (const ExceptionClassNotFound &e) {
            LogStream::sendError() << "skedit create: " << e.what() << endl;
        }

        if (!newSkill || !loader) {
            stc("Не могу создать новое умение, проверьте логи.\r\n", ch);
            return;
        }

        newSkill->setName(args);
        newSkill->help.construct();
        newSkill->help->setID(
            help_next_free_id()
        );

        loader->loadElement(newSkill);
        loader->saveElement(newSkill);

        ptc(ch, "Создано новое умение под именем %s.\r\n", newSkill->getName().c_str());

        OLCStateSkill::Pointer skedit(NEW, *newSkill);
        skedit->attach(ch);
        skedit->show(ch);

        return;
    }

    // skedit list [all|active|passive|magic|prayer|<group>]
    if (arg_is_list(cmd)) {
        bool all = false, active = false, passive = false, magic = false, prayer = false;
        SkillGroup *group = 0;

        if (!args.empty()) {
            if (arg_is_all(args))
                all = true;
            else if (arg_oneof(args, "active", "активные"))
                active = true;
            else if (arg_oneof(args, "passive", "пассивные"))
                passive = true;
            else if (arg_oneof(args, "magic", "магия"))
                magic = true;
            else if (arg_oneof(args, "prayer", "молитвы"))
                prayer = true;
            else {
                group = skillGroupManager->findUnstrict(args);
                if (!group) {
                    stc("Неверный параметр или группа, используй {y{hcskedit list{x для справки.\r\n", ch);
                    return;
                }
            }
        } else {
            stc("Использование: skedit list [all|active|passive|magic|prayer|<group>]\r\n", ch);
            return;
        }

        list<BasicSkill *> skills;
        for (int sn = 0; sn < skillManager->size(); sn++) {
            Skill *skill = skillManager->find(sn);
            BasicSkill *s = dynamic_cast<BasicSkill *>(skill);
            if (!s)
                continue;

            DefaultSpell::Pointer spell = s->getSpell().getDynamicPointer<DefaultSpell>();

            if (all
                || (active && !s->isPassive())
                || (passive && s->isPassive())
                || (magic && spell && spell->flags.isSet(SPELL_MAGIC))
                || (prayer && spell && spell->flags.isSet(SPELL_PRAYER))
                || (group && skill->hasGroup(group->getIndex())))
            {
                skills.push_back(s);
            }                
        }

        ostringstream buf;
        buf << dlprintf("{C%-20s %-4s %4s %1s %1s{x\r\n", "Имя", "Тип", "Мана", "T", "F");
        const DLString lineFormat = 
            web_cmd(ch, "skedit $1", "%-20s") + " %-4s %4d %1s %1s{x\r\n";

        for (auto &s: skills) {
            DefaultSpell::Pointer spell = s->getSpell().getDynamicPointer<DefaultSpell>();
            DLString type;
            bool fenia = false;

            if (spell && spell->isCasted()) {
                type = spell->type == SPELL_OFFENSIVE ? "{ROFF{x" : spell->type == SPELL_DEFENSIVE ? "{GDEF{x" : "";
                WrapperBase *wrapper = spell->getWrapper();
                if (wrapper) {
                    StringSet triggers, misc;
                    wrapper->collectTriggers(triggers, misc);
                    fenia = !triggers.empty() || !misc.empty();
                }
            }
            
            buf << dlprintf(
                    lineFormat.c_str(),
                    s->getName().c_str(),
                    type.c_str(),
                    s->getMana(),
                    (spell && spell->isCasted() && spell->tier > 0 ? spell->tier.toString().c_str() :  ""),
                    (fenia ? "{g*{x" : ""));
        }

        buf << "T - крутость (tier), F - перекрыто ли из фени." << endl;
        page_to_char(buf.str().c_str(), ch);
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    int sn = skillManager->unstrictLookup(arg, 0);        
    if (sn < 0) {
        ch->printf("Умение '%s' не найдено.\r\n", arg.c_str());
        return;
    }

    Skill *skill = skillManager->find(sn);
    if (!dynamic_cast<BasicSkill *>(skill)) {
        ch->printf("Умение '%s' невозможно отредактировать.\r\n", skill->getName().c_str());
        return;
    }
    
    OLCStateSkill::Pointer ske(NEW, skill);
    ske->attach(ch);
    ske->show(ch);
}

