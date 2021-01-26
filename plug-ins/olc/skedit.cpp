
#include <pcharacter.h>
#include "skillhelp.h"
#include "skillgroup.h"
#include "skillmanager.h"
#include "defaultspell.h"
#include "basicskill.h"

#include "skedit.h"
#include "hedit.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"

#include "merc.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "weapontier.h"
#include "damageflags.h"
#include "commandflags.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateSkill);

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
            stc("Это поле определено только для заклинаний.\r\n", owner->character);
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

    ptc(ch, "Умение:      {C%s{\r\n", r->getName().c_str());
    ptc(ch, "По-русски:   {C%s{x %s {D(russian help){x\r\n",
            r->getRussianName().c_str(), 
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Группа:      {C%s{x {D(group){x\r\n", r->getGroup()->getName().c_str());
    ptc(ch, "Задержка:    {C%d{w пульсов {D(beats){x\r\n", r->getBeats());
    ptc(ch, "Мана:        {C%d{w {D(mana){x\r\n", r->getMana());

    if (r->help)
        ptc(ch, "Справка:     %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", r->help->getID()).c_str(),
            r->help->getID());
    else
        ptc(ch, "Справка:     нет {D(help create){x\r\n");


    if (s) {
        ptc(ch, "Цели:        {Y%s{w {D(target){x\r\n", s->target.names().c_str());                        
        ptc(ch, "Позиция:     {Y%s{w {D(position){x\r\n", s->position.name().c_str());
        ptc(ch, "Тип:         {Y%s{w {D(type){x\r\n", s->type.name().c_str());
        ptc(ch, "Приказ:      {Y%s{x {D(order){x\r\n",
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
    
    ptc(ch, "\r\n{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

SKEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

SKEDIT(fenia, "феня", "редактировать тригера")
{
    DefaultSpell *s = getSpell();
    if (checkSpell(s))
        feniaTriggers->openEditor(ch, s, argument);
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

SKEDIT(group, "группа", "группа умений (? practicer)")
{
    BasicSkill *r = getOriginal();
    return globalReferenceEdit<SkillGroupManager, SkillGroup>(r->getGroup());
}

SKEDIT(russian, "русское", "русское имя умения")
{
    BasicSkill *r = getOriginal();
    return editor(argument, r->nameRus, ED_NO_NEWLINE);
}

SKEDIT(target, "цели", "цели заклинания (? target_table)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(target_table, s->target);
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
    return numberEdit(BEST_TIER, WORST_TIER, s->tier);
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

