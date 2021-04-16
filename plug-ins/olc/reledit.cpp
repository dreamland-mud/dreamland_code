
#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "room.h"
#include "areahelp.h"
#include "util/regexp.h"

#include "reledit.h"
#include "olc.h"
#include "security.h"

#include "religionflags.h"
#include "merc.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "interp.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateReligion);
RELIG(none);
RELIG(chronos);


OLCStateReligion::OLCStateReligion() : isChanged(false)
{
}

OLCStateReligion::OLCStateReligion(Religion *religion) 
    : isChanged(false)
{
    if (!original)
        return;

    original = religion->getIndex();
}

OLCStateReligion::~OLCStateReligion() 
{
}

void OLCStateReligion::commit() 
{
    if (!isChanged)
        return;

    DefaultReligion *original = getOriginal();
    if (!original)
        return;
    
    original->save();
    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

DefaultReligion * OLCStateReligion::getOriginal()
{
    Religion *religion = religionManager->find(original->getIndex());
    if (!religion)
        throw Exception("Attached religion doesn't exist");

    DefaultReligion *defaultReligion = dynamic_cast<DefaultReligion *>(religion);
    if (!defaultReligion)
        throw Exception("Attached religion was unloaded");
    
    return defaultReligion;
}

void OLCStateReligion::statePrompt(Descriptor *d) 
{
    d->send( "Religion> " );
}

void OLCStateReligion::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateReligion::show( PCharacter *ch )
{
    DefaultReligion *r = getOriginal();

    ptc(ch, "Короткое описание: {C%s{x  %s {D(short help){x\r\n", 
        r->getShortDescr().c_str(),
        web_edit_button(ch, "short", "web").c_str());

    ptc(ch, "Русское описание:  {C%s{x  %s {D(russian help){x\r\n",
        r->getRussianName().c_str(),
        web_edit_button(ch, "russian", "web").c_str());

    ptc(ch, "Пол божества:      {C%s{x {D(? sex){x\r\n",
        sex_table.name(r->getSex()).c_str());

    ptc(ch, "Описание:          {C%s{x  %s {D(desc help){x\r\n",
        r->getDescription().c_str(),
        web_edit_button(ch, "desc", "web").c_str());        

    ptc(ch, "Флаги:             {C%s{x {D(? religion_flags){x\r\n",
        r->flags.getValue() != 0 ? r->flags.names().c_str() : "-");

    OBJ_INDEX_DATA *tattoo = r->tattooVnum == 0 ? 0 : get_obj_index(r->tattooVnum);
    ptc(ch, "Знак:              {C%d{x [{W%s{x] {D(tattoo help){x\r\n",
        r->tattooVnum.getValue(), 
        tattoo ? russian_case(tattoo->short_descr, '1').c_str() : "none");

    if (r->help)
        ptc(ch, "Справка: %s {D(hedit %d){x\r\n",
            web_edit_button(ch, "hedit", r->help->getID()).c_str(),
            r->help->getID());

    ptc(ch, "Натура:            {Y%s{x {D(? align){x\r\n",
        r->align.getValue() != 0 ? r->align.names().c_str() : "-");

    ptc(ch, "Этос:              {Y%s{x {D(? ethos){x\r\n",
        r->ethos.getValue() != 0 ? r->ethos.names().c_str() : "-");

    ptc(ch, "Расы:              {Y%s{x {D(? races){x\r\n",
        r->races.empty() ? "-": r->races.toString().c_str());

    ptc(ch, "Классы:            {Y%s{x {D(? classes){x\r\n",
        r->classes.empty() ? "-": r->classes.toString().c_str());

    ptc(ch, "Кланы:             {Y%s{x {D(? clans){x\r\n",
        r->clans.empty() ? "-": r->clans.toString().c_str());

    ptc(ch, "Мин. параметры:    {Y%s{x {D(minstat help){x\r\n", 
            show_enum_array(r->minstat).c_str());

    ptc(ch, "Макс. параметры:   {Y%s{x {D(maxstat help){x\r\n", 
            show_enum_array(r->maxstat).c_str());

    ptc(ch, "Возраст:           {Y%s-%s{x {D(age help){x\r\n", 
        r->minage > 0 ? DLString(r->minage).c_str() : "0",
        r->maxage > 0 ? DLString(r->maxage).c_str() : "");

    ptc(ch, "{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");        
}

RELEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

RELEDIT(short, "короткое", "установить английское имя с большой буквы")
{
    return editor(argument, getOriginal()->shortDescr, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

RELEDIT(russian, "русское", "установить русское имя с падежами")
{
    return editor(argument, getOriginal()->nameRus, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

RELEDIT(sex, "пол", "установить пол божества")
{
    return flagValueEdit(getOriginal()->sex);
}

RELEDIT(desc, "описание", "установить описание божества")
{
    return editor(argument, getOriginal()->description, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

RELEDIT(flags, "флаги", "выставить флаги религии (? religion_flags)")
{
    return flagBitsEdit(getOriginal()->flags);
}

RELEDIT(mark, "знак", "установить vnum знака религии")
{
    Integer vnum;
    if (!Integer::tryParse(vnum, argument)) {
        stc("Укажи vnum предмета-знака или 0 для сброса.\r\n", ch);
        return false;
    }

    if (vnum == 0) {
        getOriginal()->tattooVnum = 0;
        stc("Знак очищен.\r\n", ch);
        return false;
    }
 
    OBJ_INDEX_DATA *tattoo = get_obj_index(vnum);
    if (!tattoo) {
        stc("Предмета с таким внумом не существует, сперва создайте его командой oedit create <vnum>.\r\n", ch);
        return false;
    }

    getOriginal()->tattooVnum = vnum.getValue();
    ptc(ch, "Знак этой религии теперь %d: %s.\r\n", 
            vnum, russian_case(tattoo->short_descr, '1').c_str());

    if (tattoo->item_type != ITEM_TATTOO) 
        stc("Осторожно, этот предмет не имеет тип 'mark'!\r\n", ch);
    
    if (!IS_SET(tattoo->wear_flags, ITEM_WEAR_TATTOO)) 
        stc("Осторожно, этот предмет не надевается на лоб (wear_tattoo)!\r\n", ch);

    return true;
}

RELEDIT(align, "натура", "ограничить по натуре")
{
    return flagBitsEdit(getOriginal()->align);
}

RELEDIT(ethos, "этос", "ограничить по этосу")
{
    return flagBitsEdit(getOriginal()->ethos);
}

RELEDIT(races, "расы", "ограничить по расам")
{
    return globalBitvectorEdit<Race>(getOriginal()->races);
}

RELEDIT(classes, "классы", "ограничить по классам")
{
    return globalBitvectorEdit<Profession>(getOriginal()->classes);
}

RELEDIT(clans, "кланы", "ограничить по клановой принадлежности")
{
    return globalBitvectorEdit<Clan>(getOriginal()->clans);
}

RELEDIT(minstat, "минпарам", "ограничить по параметрам снизу")
{
    return enumerationArrayEdit(stat_table, getOriginal()->minstat);
}

RELEDIT(maxstat, "макспарам", "ограничить по параметрам сверху")
{
    return enumerationArrayEdit(stat_table, getOriginal()->maxstat);
}

RELEDIT(age, "возраст", "ограничить по возрасту")
{
    DefaultReligion *r = getOriginal();
    return rangeEdit(0, 10000, r->minage, r->maxage);
}

RELEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

RELEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}

static bool religion_valid(DefaultReligion *rel)
{
    return rel 
        && rel->isValid()
        && !rel->flags.isSet(RELIG_SYSTEM);
}

CMD(reledit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online religion editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  reledit название\r\n", ch);
        stc("         reledit list\r\n", ch);
        stc("         reledit tattoo\r\n", ch);
        stc("         reledit create новое_ключевое_слово\r\n", ch);
        return;
    }

    if (arg_oneof(cmd, "create", "создать")) {
        static RegExp namePattern("^[a-z ]{2,}$", true);
        if (args.empty() || !namePattern.match(args)) {
            stc("Укажите английское название религии маленькими буквами.\r\n", ch);
            return;
        }

        if (religionManager->findUnstrict(args)) {
            stc("Религия с похожим ключевым словом уже существует.\r\n", ch);
            return;
        }

        DefaultReligion::Pointer rel(NEW);
        rel->setName(args);
        rel->shortDescr = args.capitalize();
        rel->help.construct();
        rel->help->setID(
            help_next_free_id()
        );
        
        ReligionLoader::getThis()->loadElement(rel);
        ReligionLoader::getThis()->saveElement(rel);

        ptc(ch, "Создана новая религия под именем %s.\r\n", rel->getName().c_str());

        OLCStateReligion::Pointer he(NEW, *rel);
        he->attach(ch);
        he->show(ch);
        return;
    }

    if (arg_oneof(cmd, "tattoo", "татуировка") || arg_oneof(cmd, "mark", "знак")) {
        ch->send_to(dlprintf(
            "{C%-15s %-17s %-6s %s{x\r\n", "Название", "Русское имя", "VNUM", "Описание"));        

        const DLString lineFormat = 
            web_cmd(ch, "reledit $1", "%-15s") 
                + " %-17s {W"
                + web_cmd(ch, "oedit $1", "%-6d")
                + "{x %s"
                + "{x\r\n";         
        const DLString lineFormatNoTattoo = 
            web_cmd(ch, "reledit $1", "%-15s") 
                + " %-17s\r\n";

        for (int r = 0; r < religionManager->size(); r++) {
            DefaultReligion *rel = dynamic_cast<DefaultReligion *>(religionManager->find(r));
            if (!religion_valid(rel))
                continue;

            OBJ_INDEX_DATA *pObj = NULL;

            if (rel->tattooVnum > 0 && rel->tattooVnum != 50)
                pObj = get_obj_index(rel->tattooVnum);

            if (pObj)
                ch->send_to(dlprintf(lineFormat.c_str(),
                        rel->getShortDescr().c_str(),
                        rel->getRussianName().ruscase('1').c_str(),
                        pObj->vnum,
                        russian_case(pObj->short_descr, '1').c_str()));
            else
                ch->send_to(dlprintf(lineFormatNoTattoo.c_str(),
                        rel->getShortDescr().c_str(),
                        rel->getRussianName().ruscase('1').c_str()));
        }
        return;
    }

    if (arg_is_list(cmd)) {
        ch->send_to(dlprintf("{C%-15s %-17s %-3s {Y%-3s   %-3s %1s %1s{x\r\n", "Название", "Русское имя", "SEX", "ALG", "ETH", "R", "C"));

        const DLString lineFormat = 
            web_cmd(ch, "reledit $1", "%-15s") + " %-17s %-3s %1s%1s%1s   %1s%1s%1s %1s %1s{x\r\n";

        for (int r = 0; r < religionManager->size(); r++) {
            DefaultReligion *rel = dynamic_cast<DefaultReligion *>(religionManager->find(r));
            if (!religion_valid(rel))
                continue;

            ch->send_to(dlprintf(lineFormat.c_str(),
                    rel->getShortDescr().c_str(),
                    rel->getRussianName().ruscase('1').c_str(),
                    rel->getSex() == SEX_MALE ? "M" : "F",
                    rel->getAlign().isSetBitNumber(N_ALIGN_GOOD) ? "{YG{x" : "{D-{x",
                    rel->getAlign().isSetBitNumber(N_ALIGN_NEUTRAL) ? "N" : "{D-{x",
                    rel->getAlign().isSetBitNumber(N_ALIGN_EVIL) ? "{RE{x" : "{D-{x",
                    rel->getEthos().isSetBitNumber(ETHOS_LAWFUL) ? "{WL{x" : "{D-{x",
                    rel->getEthos().isSetBitNumber(ETHOS_NEUTRAL) ? "N" : "{D-{x",
                    rel->getEthos().isSetBitNumber(ETHOS_CHAOTIC) ? "{MC{x" : "{D-{x",
                    (!rel->classes.empty() || !rel->races.empty()) ? "{Y*{x" : "",
                    !rel->clans.empty() ? "{Y*{x" : ""));
        }
        ch->pecho("R - ограничено по классу и/или расе; C - ограничено по клану.");
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    

    Religion *religion = religionManager->findExisting(arg);
    if (!religion)
        religion = religionManager->findUnstrict(arg);
        
    if (!religion || dynamic_cast<DefaultReligion *>(religion) == NULL) {
        stc("Религия с таким названием не найдена, используйте reledit list для списка.\r\n", ch);
        return;
    }

    OLCStateReligion::Pointer he(NEW, religion);
    he->attach(ch);
    he->show(ch);
}

