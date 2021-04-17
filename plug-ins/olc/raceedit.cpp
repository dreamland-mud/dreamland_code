#include <algorithm>

#include "raceedit.h"
#include "olc.h"
#include "security.h"
#include "hedit.h"

#include "util/regexp.h"
#include "wearlocation.h"
#include "defaultpcrace.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateRace);

OLCStateRace::OLCStateRace() : isChanged(false)
{
}

OLCStateRace::OLCStateRace(DefaultRace *race) 
    : isChanged(false)
{
    if (!original)
        return;

    original = race->getIndex();
}

OLCStateRace::~OLCStateRace() 
{
}

void OLCStateRace::commit() 
{
    if (!isChanged)
        return;

    DefaultRace *original = getOriginal();
    if (!original)
        return;
    
    original->save();
    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

DefaultRace * OLCStateRace::getOriginal()
{
    DefaultRace *race = dynamic_cast<DefaultRace *>(original.getElement());
    if (!race)
        throw Exception("Attached race was unloaded");
    
    return race;
}

DefaultPCRace* OLCStateRace::getPC() 
{
    DefaultRace *race = getOriginal();
    PCRace *pc = race->getPC();
    if (!pc)
        return 0;

    return dynamic_cast<DefaultPCRace *>(pc);
}

bool OLCStateRace::checkPC(DefaultPCRace *pc)
{
    if (!pc) {
        if (owner)
            stc("Это поле определено только для игровых рас.\r\n", owner->character);
        return false;
    }

    return true;
}

void OLCStateRace::statePrompt(Descriptor *d) 
{
    d->send( "Race> " );
}

void OLCStateRace::changed( PCharacter *ch )
{
    isChanged = true;
}

static DLString show_flag(const Flags &f)
{
    return f.getValue() == 0 ? "-" : f.names();
}

void OLCStateRace::show( PCharacter *ch )
{
    DefaultRace *r = getOriginal();
    DefaultPCRace *pc = getPC();

    ptc(ch, "Раса:      {C%s\r\n", r->getName().c_str());
    ptc(ch, "Названия:  мужское [{C%s{x]  %s {D(male help){x\r\n",
        r->nameMale.c_str(), web_edit_button(ch, "male", "web").c_str());
    ptc(ch, "           женское [{C%s{x]  %s {D(female help){x\r\n", 
        r->nameFemale.c_str(), web_edit_button(ch, "female", "web").c_str());
    ptc(ch, "           среднее [{C%s{x]  %s {D(neut help){x\r\n", 
        r->nameNeuter.c_str(), web_edit_button(ch, "neuter", "web").c_str());
    ptc(ch, "           множественное [{C%s{x]  %s {D(mult help){x\r\n", 
        r->nameMlt.c_str(), web_edit_button(ch, "mult", "web").c_str());

    if (pc) {
        ptc(ch, "           в счете [{C%s{x]  %s {D(score help){x\r\n", 
            pc->nameScore.c_str(), web_edit_button(ch, "score", "web").c_str());        
        ptc(ch, "           'кто' по-английски [{C%s{x]  %s {D(whoeng help){x\r\n", 
            pc->nameWho.c_str(), web_edit_button(ch, "whoeng", "web").c_str());        
        ptc(ch, "           'кто' по-русски [{C%s{x]  %s {D(whorus help){x\r\n", 
            pc->nameWhoRus.c_str(), web_edit_button(ch, "whorus", "web").c_str());        
        ptc(ch, "           'кто' по-женски [{C%s{x]  %s {D(whofemale help){x\r\n", 
            pc->nameWhoFemale.c_str(), web_edit_button(ch, "whofemale", "web").c_str());        
    }

    ptc(ch, "Поведение:     {Y%s{x {D(act){x\r\n", show_flag(r->act).c_str());
    ptc(ch, "Аффекты:       {Y%s{x {D(aff){x\r\n", show_flag(r->aff).c_str());
    ptc(ch, "Обнаружение:   {Y%s{x {D(det){x\r\n", show_flag(r->det).c_str());
    ptc(ch, "Атаки:         {Y%s{x {D(off){x\r\n", show_flag(r->off).c_str());
    ptc(ch, "Иммунитет:     {Y%s{x {D(imm){x\r\n", show_flag(r->imm).c_str());
    ptc(ch, "Сопротивл:     {Y%s{x {D(res){x\r\n", show_flag(r->res).c_str());
    ptc(ch, "Уязвимость:    {Y%s{x {D(vuln){x\r\n", show_flag(r->vuln).c_str());
    ptc(ch, "Форма тела:    {Y%s{x {D(form){x\r\n", show_flag(r->form).c_str());
    ptc(ch, "Части тела:    {Y%s{x {D(parts){x\r\n", show_flag(r->parts).c_str());
    ptc(ch, "Слоты:         {Y%s{x {D(wearloc){x\r\n", r->wearloc.toString().c_str());

    ptc(ch, "Политика:      ненавидит [{r%s{x] {D(hates){x любит [{g%s{x] {D(loves){x\r\n",
           r->hunts.toString().c_str(), r->donates.toString().c_str());

    if (pc) {
        ptc(ch, "Параметры:     {G%s {D(stats help){x\r\n", show_enum_array(pc->stats).c_str());
        ptc(ch, "Натура:        {G%s {D(align){x  мин {G%d{x макс {G%d{x {D(minalign, maxalign){x\r\n",
             show_flag(pc->align).c_str(), pc->minAlign.getValue(), pc->maxAlign.getValue());
        ptc(ch, "Бонусы:        здоровье {G%d {D(hp){x мана {G%d {D(mana){x практики {G%d {D(prac){x\r\n",
                pc->hpBonus.getValue(), pc->manaBonus.getValue(), pc->pracBonus.getValue());
    }

    if (r->help)
        ptc(ch, "Справка:       %s {D(help или hedit %d){x\r\n",
            web_edit_button(ch, "hedit", r->help->getID()).c_str(),
            r->help->getID());
    else
        ptc(ch, "Справка:       нет {D(help create){x\r\n");

    ptc(ch, "\r\n{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

RACEEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

RACEEDIT(help, "справка", "создать или посмотреть справку")
{
    DLString arg = argument;
    DefaultRace *r = getOriginal();

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
        r->help->setRace(r);

        OLCStateHelp::Pointer hedit(NEW, r->help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }   

    ptc(ch, "Использование: help, help create\r\n");
    return false;
}


RACEEDIT(male, "мужское", "название расы в мужском роде")
{
    return editor(argument, getOriginal()->nameMale, ED_NO_NEWLINE);
}
RACEEDIT(female, "женское", "название расы в женском роде")
{
    return editor(argument, getOriginal()->nameFemale, ED_NO_NEWLINE);
}
RACEEDIT(neuter, "среднее", "название расы в среднем роде")
{
    return editor(argument, getOriginal()->nameNeuter, ED_NO_NEWLINE);
}
RACEEDIT(mult, "множественное", "название расы во множественном числе")
{
    return editor(argument, getOriginal()->nameMlt, ED_NO_NEWLINE);
}
RACEEDIT(score, "счет", "как видно расу по команде 'счет'")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && editor(argument, pc->nameScore, ED_NO_NEWLINE);
}
RACEEDIT(whoeng, "ктоангл", "как видно расу по команде 'кто' в англ варианте")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && editor(argument, pc->nameWho, ED_NO_NEWLINE);
}
RACEEDIT(whorus, "кторус", "как видно расу по команде 'кто' в русском варианте")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && editor(argument, pc->nameWhoRus, ED_NO_NEWLINE);
}
RACEEDIT(whofemale, "ктожен", "как видно женское название расы по команде 'кто' в русском варианте")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && editor(argument, pc->nameWhoFemale, ED_NO_NEWLINE);
}

RACEEDIT(act, "поведение", "флаги поведения (? act_flags)")
{
    return flagBitsEdit(getOriginal()->act);
}
RACEEDIT(aff, "аффекты", "флаги аффектов (? affect_flags)")
{
    return flagBitsEdit(getOriginal()->aff);
}
RACEEDIT(det, "обнаружение", "флаги обнаружений (? detect_flags)")
{
    return flagBitsEdit(getOriginal()->det);
}
RACEEDIT(off, "атаки", "флаги атаки (? off_flags)")
{
    return flagBitsEdit(getOriginal()->off);
}
RACEEDIT(imm, "иммунитет", "флаги иммунитета (? imm_flags)")
{
    return flagBitsEdit(getOriginal()->imm);
}
RACEEDIT(res, "сопротивляемость", "флаги сопротивляемости (? res_flags)")
{
    return flagBitsEdit(getOriginal()->res);
}
RACEEDIT(vuln, "уязвимость", "флаги уязвимости (? vuln_flags)")
{
    return flagBitsEdit(getOriginal()->vuln);
}
RACEEDIT(form, "форма", "формы тела (? form_flags)")
{
    return flagBitsEdit(getOriginal()->form);
}
RACEEDIT(parts, "части", "части тела (? part_flags)")
{
    return flagBitsEdit(getOriginal()->parts);
}
RACEEDIT(wearloc, "слоты", "слоты экипировки (? wearloc)")
{
    return globalBitvectorEdit<Wearlocation>(getOriginal()->wearloc);
}
RACEEDIT(hates, "ненавидит", "какие расы не любит")
{
    return globalBitvectorEdit<Race>(getOriginal()->hunts);
}
RACEEDIT(loves, "любит", "какие расы любит")
{
    return globalBitvectorEdit<Race>(getOriginal()->donates);
}

RACEEDIT(stats, "парамеры", "бонусы к параметрам персонажа")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && enumerationArrayEdit(stat_table, pc->stats);
}
RACEEDIT(align, "натура", "ограничить натуру по названию")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && flagBitsEdit(pc->align);
}
RACEEDIT(minalign, "мин", "ограничить натуру по числовому значению снизу")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && numberEdit(ALIGN_EVIL, ALIGN_GOOD, pc->minAlign);
}
RACEEDIT(maxalign, "макс", "ограничить натуру по числовому значению сверху")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && numberEdit(ALIGN_EVIL, ALIGN_GOOD, pc->maxAlign);
}
RACEEDIT(hp, "здоровье", "бонус здоровья при создании персонажа")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && numberEdit(0, 100, pc->hpBonus);
}
RACEEDIT(mana, "мана", "бонус маны при создании персонажа")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && numberEdit(0, 100, pc->manaBonus);
}
RACEEDIT(prac, "практики", "бонус на кол-во практик при создании персонажа")
{
    DefaultPCRace *pc = getPC();
    return checkPC(pc) && numberEdit(0, 100, pc->pracBonus);
}

RACEEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

RACEEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

static int count_mob_race(Race *race)
{
    int cnt = 0;

    for(int hashi = 0;hashi < MAX_KEY_HASH;hashi++)
    for(MOB_INDEX_DATA *pMob = mob_index_hash[hashi]; pMob; pMob = pMob->next) {
        if (race->getName() == pMob->race)
            cnt++;
    }

    return cnt;
}

CMD(raceedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online race editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  raceedit название\r\n", ch);
        stc("         raceedit list [name|rname|mobs] \r\n", ch);
        stc("         raceedit create pc|npc <name>\r\n", ch);
        return;
    }

    if (arg_oneof(cmd, "create", "создать")) {
        DLString subcmd = args.getOneArgument();

        bool pc;
        if (arg_oneof(subcmd, "pc", "игрок"))
            pc = true;
        else if (arg_oneof(subcmd, "npc", "mob", "моб"))
            pc = false;
        else {
            stc("Формат: raceedit create pc|npc <name>\r\n", ch);
            return;
        }

        static RegExp namePattern("^[a-z ]{2,}$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("Укажите английское название расы маленькими буквами.\r\n", ch);
            return;
        }

        if (raceManager->findExisting(args)) {
            stc("Раса с таким названием уже существует.\r\n", ch);
            return;
        }

        if (pc && raceManager->findUnstrictPC(args)) {
            stc("Игровая раса с таким или похожим названием уже существует.\r\n", ch);
            return;
        }

        DefaultRace::Pointer race;
        if (pc) {
            DefaultPCRace::Pointer pcrace(NEW);
            race.setPointer(*pcrace);
        } else {
            race.construct();
        }

        race->setName(args);
        
        RaceLoader::getThis()->loadElement(race);
        RaceLoader::getThis()->saveElement(race);

        ptc(ch, "Создана новая %s раса под именем %s.\r\n", (pc ? "PC" : "NPC"), race->getName().c_str());

        OLCStateRace::Pointer editor(NEW, *race);
        editor->attach(ch);
        editor->show(ch);
        return;
    }

    if (arg_is_list(cmd)) {
        struct RaceInfo { 
            static bool compareMobs(const RaceInfo &a, const RaceInfo &b)
            {
                return a.mobs < b.mobs;
            }
            static bool compareName(const RaceInfo &a, const RaceInfo &b)
            {
                return a.race->getName().compare(b.race->getName()) < 0;
            }
            static bool compareRusName(const RaceInfo &a, const RaceInfo &b)
            {
                return a.race->getMaleName().compareRussian(b.race->getMaleName()) < 0;
            }

            Race *race; 
            int mobs; 
        };

        vector<RaceInfo> races;

        for (int i = 0; i < raceManager->size(); i++) {
            DefaultRace *race = dynamic_cast<DefaultRace *>(raceManager->find(i));
            if (race && race->isValid()) {
                RaceInfo ri;
                ri.race = race;
                ri.mobs = count_mob_race(race);
                races.push_back(ri);
            }
        }

        if (arg_has_oneof(args, "count", "mobs", "мобы") || args.empty()) 
            sort(races.begin(), races.end(), RaceInfo::compareMobs);
        else if (arg_has_oneof(args, "name", "имя"))
            sort(races.begin(), races.end(), RaceInfo::compareName);
        else if (arg_has_oneof(args, "rname", "russina", "русское"))
            sort(races.begin(), races.end(), RaceInfo::compareRusName);

        ch->send_to(dlprintf("{C%-15s %-19s {Y%s{x\r\n", "Название", "По-русски", "Мобов"));

        for (auto &ri: races) {
            Race *race = ri.race;
            const DLString searchFormat = "searcher mq race='" + race->getName() + "'";
            const DLString lineFormat = 
                web_cmd(ch, "raceedit $1", "{C%-15s") 
                    + " {%s%-19s " 
                    + web_cmd(ch, searchFormat, "{Y%4d") 
                    + "{x\r\n";

            ch->send_to(dlprintf(lineFormat.c_str(),
                    race->getName().c_str(),
                    (race->isPC() ? "g" : "w"),
                    race->getMaleName().ruscase('1').c_str(),
                    ri.mobs));
        }
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    Race *race = raceManager->findExisting(arg);
    DefaultRace *defRace;
    if (!race)
        race = raceManager->findUnstrict(arg);
        
    if (!race || !(defRace = dynamic_cast<DefaultRace *>(race))) {
        stc("Раса с таким названием не найдена, используйте {y{hcraceedit list{x для списка.\r\n", ch);
        return;
    }

    OLCStateRace::Pointer editor(NEW, defRace);
    editor->attach(ch);
    editor->show(ch);
}

