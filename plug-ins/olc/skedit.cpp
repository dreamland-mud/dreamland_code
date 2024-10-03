
#include "util/regexp.h"
#include "grammar_entities_impl.h"
#include "wrapperbase.h"
#include "structwrappers.h"
#include "stringset.h"
#include "pcharacter.h"
#include "skillhelp.h"
#include "defaultskillgroup.h"
#include "skillmanager.h"
#include "defaultspell.h"
#include "basicskill.h"
#include "mobskilldata.h"
#include "defaultaffecthandler.h"
#include "defaultskillcommand.h"
#include "commandmanager.h"
#include "xmltableloader.h"
#include "religion.h"
#include "skill_alloc.h"

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
#include "comm.h"

#include "def.h"

OLC_STATE(OLCStateSkill);
OLC_STATE(OLCStateSkillGroup);

DLString print_damage_tiers(int tier, int level_step);

OLCStateSkill::OLCStateSkill() : isChanged(false)
{
}

OLCStateSkill::OLCStateSkill(Skill *skill) 
    : isChanged(false)
{
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
        owner->character->pecho("Изменения сохранены на диск.");
}

BasicSkill * OLCStateSkill::getOriginal()
{
    Skill *skill = original.getElement();
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

DefaultAffectHandler* OLCStateSkill::getAffect(BasicSkill *skill) 
{
    if (!skill)
        skill = getOriginal();

    if (skill->affect)    
        return skill->affect.getDynamicPointer<DefaultAffectHandler>();

    return 0;
}

DefaultSkillCommand * OLCStateSkill::getCommand(BasicSkill *skill)
{
    if (!skill)
        skill = getOriginal();

    if (skill->command)
        return skill->command.getDynamicPointer<DefaultSkillCommand>();

    return 0;
}

// Re-add command to the command manager, when a name or a flag is changing.
bool OLCStateSkill::commandUpdate(DefaultSkillCommand *cmd)
{
    SkillPointer skill = cmd->getSkill();
    cmd->unsetSkill();
    cmd->setSkill(skill);
    return true;
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

bool OLCStateSkill::checkAffect(DefaultAffectHandler *ah) 
{
    if (!ah) {
        if (owner)
            stc("Это поле определено только для аффектов, создайте аффект командой {hc{yaffect create{x.\r\n", owner->character);
        return false;
    }

    return true;
}

bool OLCStateSkill::checkCommand(DefaultSkillCommand *cmd) 
{
    if (!cmd) {
        if (owner)
            stc("Это поле определено только для команд, создайте новую команду с помощью {yaction create <name>{x.\r\n", owner->character);
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

static void show_one_message(const char *prefix, PCharacter *ch, const DLString &message, const char *cmd)
{
    ostringstream buf;
    buf << "{G" << (message.empty() ? "-" : message)
        << " " << web_edit_button(ch, cmd, "web") << " "
        << "{D(" << cmd << ")";
    ptc(ch, "%s: %s\r\n", prefix, buf.str().c_str());
}

void OLCStateSkill::show( PCharacter *ch )
{
    BasicSkill *r = getOriginal();
    DefaultSpell *s = getSpell(r);
    DefaultAffectHandler *a = getAffect(r);
    DefaultSkillCommand *c = getCommand(r);
    MobSkillData *mob = r->getMobSkillData();

    ptc(ch, "Умение:      {C%s\r\n", r->getName().c_str());
    ptc(ch, "По-русски:   {C%s{x %s {D(russian help){x\r\n",
            r->getRussianName().c_str(), 
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Группы:      {C%s {D(group){x\r\n", r->getGroups().toString().c_str());
    ptc(ch, "Задержка:    {C%d{w пульсов {D(beats){x\r\n", r->getBeats());
    ptc(ch, "Цена, очки:  {C%d мана {D(mana) {C%d шаги {D(move){x \r\n", r->getMana(), r->move.getValue());
    ptc(ch, "Цена, %%:     {C%d%% мана {Dpmana) {C%d%% шаги {D(pmove) {C%d%% здоровье {D(phealth){x\r\n",
        r->manaPenalty.getValue(), r->movesPenalty.getValue(), r->healthPenalty.getValue());
    ptc(ch, "Характер:    {C%s {D(align) {C%s {D(ethos){x\r\n",
        r->align.getValue() != 0 ? r->align.names().c_str() : "-",
        r->ethos.getValue() != 0 ? r->ethos.names().c_str() : "-");
    ptc(ch, "Сообщение:   {C%s{x %s {D(dammsg) {C%s {D(damgender){x\r\n",
        r->getDammsg().getFullForm().c_str(),
        web_edit_button(ch, "dammsg", "web").c_str(),
        r->getDammsg().getMultiGender().toString());

    if (r->help)
        ptc(ch, "Справка:     %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", r->help->getID()).c_str(),
            r->help->getID());
    else
        ptc(ch, "Справка:     нет {D(help create){x\r\n");

    ptc(ch, "Доступно:    {c%s{x %s {D(allow help){x\r\n",
            r->accessToString().c_str(),
            web_edit_button(ch, "allow", "").c_str());

    if (mob) {
        ostringstream mobBuf;
        MobProfSkillData *pmob = dynamic_cast<MobProfSkillData *>(mob);

        mobBuf  << "разучено {c" << mob->dice << "{x*level+{c" << mob->bonus 
                << "{x, атаки {c" << (mob->offense.getValue() == 0 ? "нет" : mob->offense.names()) << "{x";
        if (pmob)
            mobBuf << ", классовое {c" << (pmob->professional ? "да" : "нет") << "{x";
        
        ptc(ch, "Мобы:        %s {D({hcmob{hx){x\r\n", mobBuf.str().c_str());
    }

    if (s) {
        ptc(ch, ".............{YЗаклинание{x.............\r\n");
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

            if (s->targetIsRanged())
                ptc(ch, "Ранговое:    {Y%s {D(ranged){x\r\n", s->ranged ? "yes" : "no");
        }

        ptc(ch, "Сообщения:   %s  %s {D(messages){x\r\n",
               s->messages.toList().join(" {Y|{x ").c_str(), web_edit_button(ch, "messages", "web").c_str());

        feniaTriggers->showTriggers(ch, s);
    }

    if (c) {
        ptc(ch, ".............{YКоманда %s{x..........\r\n", c->getName().c_str());
        ptc(ch, "Синонимы:    {Y%s{x %s {D(aliases help){x\r\n",
                c->aliases.toList().toString().c_str(),
                web_edit_button(ch, "aliases", "").c_str());
        ptc(ch, "РуСинонимы:  {Y%s{x %s {D(rualiases help){x\r\n",
                c->russian.toList().toString().c_str(),
                web_edit_button(ch, "rualiases", "").c_str());
        ptc(ch, "Аргумент:    {Y%s {D(argtype){x\r\n", c->argtype.name().c_str());
        ptc(ch, "Позиция:     {Y%s {D(position){x\r\n", c->position.name().c_str());
        ptc(ch, "Флаги:       {Y%s {D(flags){x\r\n", c->extra.names().c_str());
        ptc(ch, "Приказ:      {Y%s {D(order){x\r\n", 
                c->order == 0 ? "-" : c->order.names().c_str());
        ptc(ch, "Подсказка:   {Y%s{x %s {D(hint help){x\r\n",
                c->hint.c_str(),
                web_edit_button(ch, "hint", "web").c_str());        

        feniaTriggers->showTriggers(ch, c->getWrapper(), "skillcommand");
    }

    if (a) {
        ptc(ch, ".............{GАффект{x.................\r\n");
        ptc(ch, "Отменяется:  {G%s {D(cancelled){x\r\n", a->cancelled ? "yes" : "no");
        ptc(ch, "Снимается:   {G%s {D(dispelled){x\r\n", a->dispelled ? "yes" : "no");
        show_one_message("Спадает с тебя    ", ch, a->removeCharSelf, "removeCharSelf");
        show_one_message("Спадает с соседа  ", ch, a->removeCharOthers, "removeCharOthers");
        show_one_message("Спадает с предмета", ch, a->removeObj, "removeObj");
        show_one_message("Спадает с комнаты ", ch, a->removeRoom, "removeRoom");

        feniaTriggers->showTriggers(ch, a->getWrapper(), "affect");
    }
    
    ptc(ch, "\r\n{WКоманды{x: {hc{yspell{x, {hc{yaffect{x, {hc{yaction{x, {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

SKEDIT(affect, "аффект", "создать обработчик аффекта для этого умения")
{
    DLString arg = argument;

    if (arg_oneof(arg, "create", "создать")) {
        if (getAffect()) {
            stc("Обработчик аффекта уже определен.\r\n", ch);
            return false;
        }

        DefaultAffectHandler::Pointer ah(NEW);
        BasicSkill *skill = getOriginal();

        skill->affect.setPointer(ah);
        skill->affect->setSkill(BasicSkill::Pointer(skill));
        stc("Создан новый обработчик аффекта для этого умения.\r\n", ch);
        show(ch);
        return true;
    }

    if (arg_oneof(arg, "delete", "удалить")) {
        DefaultAffectHandler *ah = getAffect();     
        if (!ah) {
            stc("У этого умения и так нету аффекта.\r\n", ch);
            return false;
        }

        if (feniaTriggers->clearTriggers(ah->wrapper))
            stc("Феневые триггера аффекта очищены.\r\n", ch);

        BasicSkill *skill = getOriginal();
        skill->affect->unsetSkill();
        skill->affect.clear();

        stc("Аффект удален.\r\n", ch);
        return true;
    }

    stc("Использование:\r\n"
        "{y{hcaffect create{x - создать обработчик аффекта\r\n"
        "{y{hcaffect delete{x - удалить обработчик\r\n", ch);
    return false;
}

SKEDIT(action, "действие", "создать команду для этого умения")
{
    DLString args = argument;
    DLString argOne = args.getOneArgument();

    if (arg_oneof(argOne, "create", "создать")) {
        if (getCommand()) {
            stc("Команда для этого умения уже существует.\r\n", ch);
            return false;
        }

        static RegExp namePattern("^[a-z]{2,}$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("Укажи английское имя для новой команды, маленькими буквами без пробелов.\r\n", ch);
            return false;
        }

        if (commandManager->findExact(args)) {
            stc("Команда с таким именем уже существует.\r\n", ch);
            return false;
        }
        
        DefaultSkillCommand::Pointer cmd(NEW);
        BasicSkill *skill = getOriginal();

        cmd->name[EN] = args;
        skill->command.setPointer(cmd);
        skill->command->setSkill(BasicSkill::Pointer(skill));

        ptc(ch, "Создана новая команда '%s'.\r\n", cmd->getName().c_str());
        show(ch);
        return true;
    }

    if (arg_oneof(argOne, "delete", "удалить")) {
        DefaultSkillCommand *cmd = getCommand();
        if (!cmd) {
            stc("У этого умения и так нету команды.\r\n", ch);
            return false;
        }

        if (feniaTriggers->clearTriggers(cmd->wrapper))
            stc("Феневые триггера команды очищены.\r\n", ch);

        BasicSkill *skill = getOriginal();
        skill->command->unsetSkill();
        skill->command.clear();

        stc("Команда удалена.\r\n", ch);
        return true;
    }

    stc("Использование:\r\n"
        "{yaction create <name>{x - создать новую команду\r\n"
        "{y{hcaction delete{x        - удалить команду\r\n", ch);
    return false;
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

    if (arg_oneof(arg, "delete", "удалить")) {
        DefaultSpell *s = getSpell();        
        if (!s) {
            stc("У этого умения и так нету заклинаний.\r\n", ch);
            return false;
        }

        if (feniaTriggers->clearTriggers(s->wrapper))
            stc("Феневые триггера заклинания очищены.\r\n", ch);

        BasicSkill *skill = getOriginal();
        skill->spell->unsetSkill();
        skill->spell.clear();

        stc("Заклинание удалено.\r\n", ch);
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
    stc("               {y{hcspell delete{x - удалить заклинание\r\n", ch);
    stc("               {y{hcspell tiers{x  - показать таблицу повреждений\r\n", ch);
    return false;
}

SKEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

SKEDIT(mob, "моб", "доступность умения для мобов")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();
    BasicSkill *skill = getOriginal();
    MobSkillData *mob = skill->getMobSkillData();

    if (!mob) {
        stc("У этого типа умений нету секции конфигурации для мобов.\r\n", ch);
        return false;
    }

    if (cmd.empty() || arg_is_help(cmd)) {
        stc("Использование: {y{hcmob off{x   - установить флаги из off_flags, делающие это умение доступным мобу\r\n", ch);
        stc("               {y{hcmob dice{x  - установить dice для уровня раскачки (dice*level+bonus)\r\n", ch);
        stc("               {y{hcmob bonus{x - установить bonus для уровня раскачки (dice*level+bonus)\r\n", ch);
        stc("               {y{hcmob class{x - нужны ли act флаги классов для владения умением\r\n", ch);
        return false;
    }

    // Override cached command and arguments so that flag&number editors work correctly with subcommand syntax.
    lastCmd.setValue(lastCmd + " " + cmd);
    lastArgs = args;

    if (arg_oneof(cmd, "offense", "атаки")) {
        return flagBitsEdit(mob->offense);
    }

    if (arg_oneof(cmd, "dice", "дайс")) {
        return numberEdit(0, 100, mob->dice);
    }

    if (arg_oneof(cmd, "bonus", "бонус")) {
        return numberEdit(0, 100, mob->bonus);
    }

    if (arg_oneof(cmd, "class", "класс")) {
        MobProfSkillData *pmob = dynamic_cast<MobProfSkillData *>(mob);
        if (!pmob) {
            stc("У этой конфигурации мобов нету поля для классов.\r\n", ch);
            return false;
        }

        return boolEdit(pmob->professional);
    }

    stc("Неверная подкоманда, смотри {y{hcmob{x для списка.\r\n", ch);
    return false;
}

SKEDIT(fenia, "феня", "редактировать тригера заклинаний и аффектов")
{
    DLString args = argument;
    DefaultSpell *s = getSpell();
    DefaultAffectHandler *a = getAffect();
    DefaultSkillCommand *c = getCommand();

    DLString trigName = args.getOneArgument();
    bool clear = arg_is_clear(args);

    if (trigName.empty() || (!args.empty() && !clear)) {
        stc("Использование: fenia <триггер> - редактировать триггер.\r\n", ch);
        stc("               fenia <триггер> clear - очистить триггер.\r\n", ch);
        return false;
    }
    
    if (trigName == "run" || trigName == "apply") {
        // Handle 'run' and 'apply' skill command overrides.
        if (!checkCommand(c))
            return false;

        if (clear) {
            if (feniaTriggers->clearTrigger(c->wrapper, trigName))
                ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
            else
                ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
        } else {
            feniaTriggers->openEditor(ch, c, trigName);
        }

    } else if (DLString("run").strPrefix(trigName)) {
        // Handle spell triggers with runXXX names.        
        if (!checkSpell(s))
            return false;

        if (clear) {
            if (feniaTriggers->clearTrigger(s->wrapper, trigName))
                ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
            else
                ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
        } else {
            feniaTriggers->openEditor(ch, s, trigName);
        }

    } else {
        // Handle affect triggers.
        if (!checkAffect(a))
            return false;

        if (clear) {
            if (feniaTriggers->clearTrigger(a->wrapper, trigName))
                ptc(ch, "Триггер %s успешно удален.\r\n", trigName.c_str());
            else
                ptc(ch, "Триггер %s не найден.\r\n", trigName.c_str());        
        } else {
            feniaTriggers->openEditor(ch, a, trigName);
        }
    }

    
    return false;
}

SKEDIT(help, "справка", "создать или посмотреть справку по умению")
{
    BasicSkill *skill = getOriginal();

    auto postCreateAction = [skill](XMLPointerNoEmpty<SkillHelp> &help) {
        help->setSkill(BasicSkill::Pointer(skill));
    };

    return help_subcommand(ch, argument, skill->help, postCreateAction);
}

SKEDIT(beats, "задержка", "wait state в пульсах (секунды * 4)")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 60, r->beats);
}

SKEDIT(mana, "мана", "расход маны, очки")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 500, r->mana);
}

SKEDIT(move, "шаги", "расход шагов, очки")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 500, r->move);
}

SKEDIT(pmana, "пмана", "расход маны, проценты")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 100, r->manaPenalty);
}

SKEDIT(pmove, "пшаги", "расход шагов, проценты")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 100, r->movesPenalty);
}

SKEDIT(phealth, "пздоровье", "расход здоровья, проценты")
{
    BasicSkill *r = getOriginal();
    return numberEdit(0, 100, r->healthPenalty);
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

SKEDIT(hint, "подсказка", "краткое описание команды")
{
    DefaultSkillCommand *c = getCommand();
    return checkCommand(c) && editor(argument, c->hint, ED_NO_NEWLINE);
}

SKEDIT(aliases, "синонимы", "список англ синонимов для команды")
{
    DefaultSkillCommand *c = getCommand();
    return checkCommand(c) && stringListEdit(c->aliases);
}

// Edit Russian aliases for the command, re-register with CommandManager if Russian name (first alias) has changed.
SKEDIT(rualiases, "русинонимы", "список русских синонимов для команды")
{
    DefaultSkillCommand *c = getCommand();
    return checkCommand(c) && stringListEdit(c->russian) && commandUpdate(c);
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

SKEDIT(dammsg, "уронимя", "сообщение об уроне ('струя кислоты')")
{
    BasicSkill *r = getOriginal();
    return editor(argument, r->dammsg, ED_NO_NEWLINE);
}

SKEDIT(damgender, "уронрод", "грамматический род сообщения об уроне (n, m, f, p)")
{
    BasicSkill *r = getOriginal();

    if (argument[0] == '\0') {
        stc("Формат:  damgender m|f|n|p\n\r", ch);
        return false;
    }

    r->dammsg.setGender(Grammar::MultiGender(argument));
    ptc(ch, "Род сообщения об уроне установлен в '{g%s{x'.\n\r", r->dammsg.getMultiGender().toString());
    return true;
}


SKEDIT(align, "натура", "ограничить по натуре")
{
    return flagBitsEdit(getOriginal()->align);
}

SKEDIT(ethos, "этос", "ограничить по этосу")
{
    return flagBitsEdit(getOriginal()->ethos);
}

SKEDIT(messages, "сообщения", "сообщения при произнесении заклинания")
{    
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && editor(argument, s->messages, ED_NO_NEWLINE);
}

SKEDIT(target, "цели", "цели заклинания (? target_table)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagBitsEdit(s->target);
}

SKEDIT(argtype, "аргумент", "тип аргумента у команды (? argtype_table)")
{
    DefaultSkillCommand *c = getCommand();
    return checkCommand(c) && flagValueEdit(c->argtype);
}

SKEDIT(flags, "флаги", "флаги заклинания или команды (? spell_flags, ? extra_flags)")
{
    DefaultSkillCommand *c = getCommand();
    if (c)
        return flagBitsEdit(c->extra) && commandUpdate(c);

    DefaultSpell *s = getSpell();
    return checkSpell(s) && flagBitsEdit(s->flags);
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
            && flagBitsEdit(s->damflags);
}

SKEDIT(order, "приказ", "кому можно приказать колдовать или выполнять команду (? order_flags)")
{
    DefaultSkillCommand *c = getCommand();
    if (c)
        return flagBitsEdit(c->order);

    DefaultSpell *s = getSpell();
    return checkSpell(s) && flagBitsEdit(s->order);
}

SKEDIT(tier, "крутость", "уровень повреждений от 1 до 5")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s) 
            && numberEdit(BEST_TIER, WORST_TIER, s->tier);
}

SKEDIT(position, "позиция", "мин. положение тела для заклинания или команды (? position_table)")
{
    DefaultSkillCommand *c = getCommand();
    if (c)
        return flagValueEdit(c->position);

    DefaultSpell *s = getSpell();
    return checkSpell(s) && flagValueEdit(s->position);
}

SKEDIT(type, "вид", "вид заклинания (? spell_types)")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && flagValueEdit(s->type);
}

SKEDIT(ranged, "ранговое", "можно ли колдовать на расстоянии")
{
    DefaultSpell *s = getSpell();
    return checkSpell(s)
            && boolEdit(s->ranged);
}

SKEDIT(cancelled, "отменяется", "можно ли сбить аффект заклинанием отмены")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a)
            && boolEdit(a->cancelled);
}

SKEDIT(dispelled, "снимается", "можно ли сбить аффект заклинанием снятия воздействий")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a)
            && boolEdit(a->dispelled);
}

SKEDIT(removeCharSelf, "", "сообщение персонажу при спадании аффекта с него самого")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a) && editor(argument, a->removeCharSelf, ED_NO_NEWLINE);
}

SKEDIT(removeCharOthers, "", "сообщение в комнату при спадании аффекта с персонажа")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a) && editor(argument, a->removeCharOthers, ED_NO_NEWLINE);
}

SKEDIT(removeRoom, "", "сообщение при спадании аффекта с комнаты")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a) && editor(argument, a->removeRoom, ED_NO_NEWLINE);
}

SKEDIT(removeObj, "", "сообщение при спадании аффекта с предмета")
{
    DefaultAffectHandler *a = getAffect();
    return checkAffect(a) && editor(argument, a->removeObj, ED_NO_NEWLINE);
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
        stc("Формат:  skedit create class|clan|orden|race|other <название по-английски>\r\n", ch);
        stc("Формат:  skedit list [all|active|passive|magic|prayer|<group>]\r\n", ch);
        return;
    }

    // Creating new skill.
    if (arg_oneof(cmd, "create", "создать")) {
        DLString type = args.getOneArgument();
        BasicSkill::Pointer newSkill;

        static RegExp namePattern("^[a-z ]{2,}[a-z]$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("В названии умения могут быть только маленькие англ буквы и пробелы.\r\n", ch);
            return;
        }

        Skill *oldSkill = skillManager->findExisting(args);
        if (oldSkill) {
            if (oldSkill->isValid()) {
                ptc(ch, "Умение '%s' уже существует.\r\n", oldSkill->getName().c_str());
                return;

            } 

            // Repopulate invalid skill.
            ptc(ch, "Умение '%s' существовало в виде 'затычки', уничтожаем ее.\r\n", oldSkill->getName().c_str());
            skillManager->unregistrate(Skill::Pointer(oldSkill));
        }

        if (arg_oneof(type, "class", "класс")) {
            newSkill = SkillAlloc::newClassSkill(args);
        } else if (arg_oneof(type, "clan", "клан")) {
            newSkill = SkillAlloc::newClanSkill(args);
        } else if (arg_oneof(type, "orden", "орден")) {
            newSkill = SkillAlloc::newOrdenSkill(args);
        } else if (arg_oneof(type, "race", "раса")) {
            newSkill = SkillAlloc::newRaceSkill(args);
        } else if (arg_oneof(type, "other", "другое", "разное")) {
            newSkill = SkillAlloc::newOtherSkill(args);
        } else {
            stc("Укажи вид нового умения: class, clan, race или other.\r\n", ch);
            return;
        }

        if (!newSkill) {
            stc("Не могу создать новое умение, проверьте логи.\r\n", ch);
            return;
        }

        ptc(ch, "Создано новое умение под именем %s.\r\n", newSkill->getName().c_str());

        OLCStateSkill::Pointer skedit(NEW, *newSkill);
        skedit->attach(ch);
        skedit->show(ch);

        return;
    }

    // skedit list [all|active|passive|magic|prayer|<group>]
    if (arg_is_list(cmd)) {
        bool all = false, active = false, passive = false, magic = false, 
             prayer = false, invalid = false;
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
            else if (arg_oneof(args, "invalid"))
                invalid = true;
            else {
                group = skillGroupManager->findUnstrict(args);
                if (!group) {
                    stc("Неверный параметр или группа, используй {y{hcskedit list{x для справки.\r\n", ch);
                    return;
                }
            }
        } else {
            stc("Использование: skedit list [all|active|passive|magic|prayer|invalid|<group>]\r\n", ch);
            return;
        }

        list<Skill *> skills;
        for (int sn = 0; sn < skillManager->size(); sn++) {
            Skill *skill = skillManager->find(sn);

            if (all 
                || (invalid && !skill->isValid()))
                skills.push_back(skill);

            BasicSkill *s = dynamic_cast<BasicSkill *>(skill);
            if (!s)
                continue;

            DefaultSpell::Pointer spell = s->getSpell().getDynamicPointer<DefaultSpell>();
            bool isSpell = spell && spell->isCasted();

            if (   (active && !s->isPassive() && !isSpell)
                || (passive && s->isPassive())
                || (magic && isSpell && spell->flags.isSet(SPELL_MAGIC))
                || (prayer && isSpell && spell->flags.isSet(SPELL_PRAYER))
                || (group && skill->hasGroup(group->getIndex())))
            {
                skills.push_back(s);
            }                
        }

        ostringstream buf;
        buf << fmt(0, "{C%-20s %-4s %4s %4s %4s %1s %1s{x\r\n", "Имя", "Тип", "Мана", "Шаги", "Wait", "T", "F");
        const DLString lineFormat = 
            "{W" + web_cmd(ch, "skedit $1", "%-20s") + "{w %-4s %4d %4d %4d %1s %1s{x\r\n";

        for (auto &s: skills) {
            DefaultSpell::Pointer spell = s->getSpell().getDynamicPointer<DefaultSpell>();
            DefaultSkillCommand::Pointer cmd = s->getCommand().getDynamicPointer<DefaultSkillCommand>();
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

            if (cmd) {
                if (cmd->getWrapper())
                    fenia = true;
            }
            
            buf << fmt(0, 
                    lineFormat.c_str(),
                    s->getName().c_str(),
                    type.c_str(),
                    s->getMana(),
                    s->getMoves(),
                    s->getBeats(),
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
        ch->pecho("Умение '%s' не найдено.", arg.c_str());
        return;
    }

    Skill *skill = skillManager->find(sn);
    if (!dynamic_cast<BasicSkill *>(skill)) {
        ch->pecho("Умение '%s' невозможно отредактировать.", skill->getName().c_str());
        return;
    }
    
    OLCStateSkill::Pointer ske(NEW, skill);
    ske->attach(ch);
    ske->show(ch);
}

CMD(gredit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online skill group editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  gredit группа умений\r\n", ch);
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    SkillGroup *group = skillGroupManager->findUnstrict(arg);
    if (!group) {
        ch->pecho("Группа умений '%s' не найдена.", arg.c_str());
        return;
    }

    if (!dynamic_cast<DefaultSkillGroup *>(group)) {
        ch->pecho("Группу '%s' невозможно отредактировать.", group->getName().c_str());
        return;
    }
    
    OLCStateSkillGroup::Pointer gre(NEW, group);
    gre->attach(ch);
    gre->show(ch);
}

OLCStateSkillGroup::OLCStateSkillGroup() : isChanged(false)
{
}

OLCStateSkillGroup::OLCStateSkillGroup(SkillGroup *group) 
    : isChanged(false)
{
    original = group->getIndex();
}

OLCStateSkillGroup::~OLCStateSkillGroup() 
{
}

void OLCStateSkillGroup::commit() 
{
    if (!isChanged)
        return;

    DefaultSkillGroup *original = getOriginal();
    if (!original)
        return;
    
    original->save();
    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

DefaultSkillGroup * OLCStateSkillGroup::getOriginal()
{
    DefaultSkillGroup *group = dynamic_cast<DefaultSkillGroup *>(original.getElement());
    if (!group)
        throw Exception("Attached group was unloaded");

    return group;
}


void OLCStateSkillGroup::changed( PCharacter * )
{
    isChanged = true;
}

void OLCStateSkillGroup::statePrompt(Descriptor *d) 
{
    d->send( "SkillGroup> " );    
}

void OLCStateSkillGroup::show(PCharacter *ch)
{
    DefaultSkillGroup *g = getOriginal();

    ptc(ch, "Группа:      {C%s\r\n", g->getName().c_str());
    ptc(ch, "Русское:     {C%s{x  %s {D(russian help){x\r\n",
            g->getRussianName().c_str(), 
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Скрыта:      {C%s {D(hidden){x\r\n", 
            g->hidden ? "yes" : "no");

    MOB_INDEX_DATA *pMob = get_mob_index(g->getPracticer());
    ptc(ch, "Учитель:     {g%s{x [%d] {D(practicer){x\r\n",
             (pMob ? russian_case(pMob->short_descr, '1').c_str() : "-"),
             g->getPracticer());
    ptc(ch, "Боги:        {g%s {D(gods){x\r\n",
        g->gods.empty() ? "-": g->gods.toString().c_str());
    ptc(ch, "Путь:        {g%s{x  %s {D(path help){x\r\n",
            g->path.c_str(), 
            web_edit_button(ch, "path", "web").c_str());

    if (g->help)
        ptc(ch, "Справка:     %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", g->help->getID()).c_str(),
            g->help->getID());
    else
        ptc(ch, "Справка:     нет {D(help create){x\r\n");

    stc("{YСообщения{x:\r\n", ch);
    ptc(ch, "Себе:        %s %s {D(self){x\r\n",
              g->msgSelf.toList().join(" {Y|{x ").c_str(),
              web_edit_button(ch, "self", "web").c_str());
    ptc(ch, "Жертве:      %s %s {D(victim){x\r\n",
              g->msgVict.toList().join(" {Y|{x ").c_str(),
              web_edit_button(ch, "victim", "web").c_str());
    ptc(ch, "Комнате:     %s %s {D(room){x\r\n",
              g->msgRoom.toList().join(" {Y|{x ").c_str(),
              web_edit_button(ch, "room", "web").c_str());


    ptc(ch, "\r\n{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");
}


GREDIT(commands, "команды", "показать список встроенных команд")
{
    do_commands(ch);
    return false;
}

GREDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

GREDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

GREDIT(help, "справка", "создать или посмотреть справку по группе")
{
    DefaultSkillGroup *group = getOriginal();

    auto postCreateAction = [group](XMLPointer<SkillGroupHelp> &help) {
        help->setSkillGroup(DefaultSkillGroup::Pointer(group));
    };

    return help_subcommand(ch, argument, group->help, postCreateAction);
}

GREDIT(russian, "русское", "русское название группы")
{
    return editor(argument, getOriginal()->nameRus, ED_NO_NEWLINE);
}

GREDIT(self, "себе", "сообщения кастеру при произнесении заклинаний этой группы")
{    
    return editor(argument, getOriginal()->msgSelf, ED_NO_NEWLINE);
}

GREDIT(victim, "жертве", "сообщения жертве при произнесении заклинаний этой группы")
{    
    return editor(argument, getOriginal()->msgVict, ED_NO_NEWLINE);
}

GREDIT(room, "комнате", "сообщения в комнату при произнесении заклинаний этой группы")
{    
    return editor(argument, getOriginal()->msgRoom, ED_NO_NEWLINE);
}

GREDIT(practicer, "учитель", "задать vnum моба-учителя группы")
{
    DefaultSkillGroup *g = getOriginal();
    Integer vnum;
    if (!Integer::tryParse(vnum, argument)) {
        stc("Укажи vnum моба-учителя или 0 для сброса.\r\n", ch);
        return false;
    }

    if (vnum == 0) {
        g->practicer = 0;
        stc("Учитель очищен.\r\n", ch);
        return true;
    }

    MOB_INDEX_DATA *pMob = get_mob_index(vnum);
    if (!pMob) {
        stc("Моба с таким номером не существует.\r\n", ch);
        return false;
    }

    g->practicer.setValue(vnum);
    ch->pecho("Учитель этой группы теперь %N1 из зоны '%s'.", pMob->short_descr, pMob->area->getName().c_str());
    return true;
}

GREDIT(hidden, "скрыта", "видима ли группа смертным")
{
    return boolEdit(getOriginal()->hidden);
}

GREDIT(gods, "боги", "задать богов-покровителей группы")
{
    return globalBitvectorEdit<Religion>(getOriginal()->gods);
}

GREDIT(path, "путь", "задать Пути для группы: light, darkness, order, fury, reason, chaos, nature, society")
{
    return editor(argument, getOriginal()->path, ED_NO_NEWLINE);
}

