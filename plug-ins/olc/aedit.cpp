/* $Id$
 *
 * ruffina, 2004
 */
#include <pcharacter.h>
#include <object.h>
#include <string.h>

#include "room.h"
#include "aedit.h"
#include "hedit.h"
#include "security.h"
#include "olc.h"
#include "areahelp.h"
#include "qedit.h"
#include "string_utils.h"
#include "websocketrpc.h"
#include "merc.h"
#include "arg_utils.h"
#include "areautils.h"
#include "update_areas.h"
#include "interp.h"
#include "act.h"

#include "def.h"

OLC_STATE(OLCStateArea);

OLCStateArea::OLCStateArea() : vnum( -1 ), area_flag(0, &area_flags)
{
    /*fromXML will!*/
}

OLCStateArea::OLCStateArea(AreaIndexData *original) : area_flag(0, &area_flags)
{
    if (original) {
        if(original->area_file->file_name)
            file_name = original->area_file->file_name;

        name = original->name;
        altname = original->altname;
        resetMessage = original->resetMessage;
        authors = original->authors;
        translator = original->translator;
        speedwalk = original->speedwalk;

        low_range = original->low_range;
        high_range= original->high_range;
        min_vnum  = original->min_vnum;
        max_vnum  = original->max_vnum;
        area_flag.setValue( original->area_flag );
        security  = original->security;
        vnum      = original->vnum;

        if (original->behavior) 
            try {
                ostringstream ostr;
                original->behavior.toStream( ostr );
                behavior.assign( ostr.str( ) );
            } catch (const ExceptionXMLError &e) {
                LogStream::sendError( ) << e.what( ) << endl;
            }
    }
    else {
        top_area++;
        vnum = top_area - 1;

        name[RU] = "Новая зона";
        security = 9;
        min_vnum = 0;
        max_vnum = 0;
        low_range = 0;
        high_range = 0;
        file_name.setValue( DLString( "area" ) + DLString( vnum ) + ".are" );
    }
}

OLCStateArea::~OLCStateArea() 
{
}

void OLCStateArea::commit() 
{
    AreaIndexData *original = get_area_data(vnum);
    
    if(!original) {
        original = new AreaIndexData;
        original->area_file = new_area_file( file_name.getValue( ).c_str( ) );
        original->area_file->area = original;

        areaIndexes.push_back(original);

        original->create();
    }
    else {
        free_string(original->area_file->file_name);
        original->area_file->file_name = str_dup( file_name.getValue( ).c_str( ) );
    }

    original->name = name;
    original->altname = altname;
    original->authors = authors;
    original->translator = translator;
    original->speedwalk = speedwalk;
    original->resetMessage = resetMessage;
    original->low_range = low_range;
    original->high_range= high_range;
    original->min_vnum  = min_vnum;
    original->max_vnum  = max_vnum;
    original->area_flag = area_flag;
    original->security  = security;
    original->vnum = vnum;
    original->changed = true;

    if (!behavior.empty( )) 
        try {
            istringstream istr( behavior );
            original->behavior.fromStream( istr );
            original->behavior->setArea( original );
        } catch (const Exception &e) {
            LogStream::sendError( ) << e.what( ) << endl;
        }

    // FIXME update all instances
    original->area->area_flag = area_flag;

    if (original->helps.empty()) {
        AreaHelp::Pointer help = AreaUtils::createHelp(original);
        XMLPersistent<HelpArticle> phelp(help.getPointer());
        original->helps.push_back(phelp);
    }
}

void OLCStateArea::statePrompt(Descriptor *d) 
{
    d->send( "Area> " );
}

bool OLCStateArea::checkOverlap(int lower, int upper)
{
    for(auto &pArea: areaIndexes) {
        if(pArea->vnum == vnum)
            continue;
            
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper))
            return false;

        if ((lower <= pArea->max_vnum && pArea->max_vnum <= upper))
            return false;
    }
    return true;
}

void OLCStateArea::changed( PCharacter *ch )
{
    AreaIndexData *original = get_area_data(vnum);    
    if (original)
        original->changed = true;
}

// Area Editor Functions.
AEDIT(show, "показать", "показать все поля")
{
    AreaIndexData *original = get_area_data(vnum);
    Area *instance = original ? original->area : 0;

    ptc(ch, "Area:       [{C%5d{x]\n\r", vnum.getValue( ));
    ptc(ch, "Name EN:    [{W%s{x] %s\n\r", name.get(EN).c_str(), web_edit_button(ch, "name", "web").c_str());   
    ptc(ch, "Name UA:    [{W%s{x] %s\n\r", name.get(UA).c_str(), web_edit_button(ch, "uaname", "web").c_str());   
    ptc(ch, "Name RU:    [{W%s{x] %s\n\r", name.get(RU).c_str(), web_edit_button(ch, "runame", "web").c_str());   
    ptc(ch, "Authors:    [{W%s{x]\n\r", authors.getValue( ).c_str( ));
    ptc(ch, "Translator: [{W%s{x]\n\r", translator.getValue( ).c_str( ));
    ptc(ch, "File:       [{W%s{x]\n\r", file_name.getValue( ).c_str( ));
    ptc(ch, "Vnums:      [{W%u-%u{x]\n\r", min_vnum.getValue( ), max_vnum.getValue( ));
    ptc(ch, "Levels:     [{W%u-%u{x]\n\r", low_range.getValue( ), high_range.getValue( ));
    ptc(ch, "Flags:      [{W%s{x] {D(? area_flags{w)\n\r", area_flags.names(area_flag).c_str());
    ptc(ch, "Security:   [{W%d{x]\n\r", security.getValue( ));
    ptc(ch, "Speedwalk:  EN [{W%s{x] %s  UA [{W%s{x] %s  RU [{W%s{x] %s\n\r", 
          String::stripEOL(speedwalk.get(EN)).c_str(), web_edit_button(ch, "speedwalk", "web").c_str(),   
          String::stripEOL(speedwalk.get(UA)).c_str(), web_edit_button(ch, "uaspeedwalk", "web").c_str(),   
          String::stripEOL(speedwalk.get(RU)).c_str(), web_edit_button(ch, "ruspeedwalk", "web").c_str());   
    ptc(ch, "Message:    EN [{W%s{x] %s  UA [{W%s{x] %s  RU [{W%s{x] %s\n\r", 
          String::stripEOL(resetMessage.get(EN)).c_str(), web_edit_button(ch, "message", "web").c_str(),   
          String::stripEOL(resetMessage.get(UA)).c_str(), web_edit_button(ch, "uamessage", "web").c_str(),   
          String::stripEOL(resetMessage.get(RU)).c_str(), web_edit_button(ch, "rumessage", "web").c_str());   

    if (instance)
        ptc(ch, "{DMain instance: empty [%s], age [%d], players [%d], flags [%s], rooms count [%d]{x\n\r",
            (instance->empty ? "yes" : "no"), instance->age, instance->nplayer, 
            area_flags.names(instance->area_flag).c_str(), instance->rooms.size());
                     
    if (!behavior.empty( ))
        ptc(ch, "Behavior:\r\n%s", behavior.c_str( ));

    if (original) {
        DLString buf;

        for (auto &article: original->helps) 
            if (article->getID() > 0)
                buf += "    [{C" + article->getAllKeywordsString() + "{x] "
                    +  web_edit_button(ch, "hedit", DLString(article->getID()))
                    + " {D(hedit " + DLString(article->getID()) + "){x\r\n";

        ch->pecho("Helps:\r\n" + buf);

    } else {
        ptc(ch, "Helps:      {Dno helps for the new area, save first\r\n");
    }

    if (original) {
        ostringstream buf;

        const DLString lineFormatQuestEdit = 
            "{W[" + web_cmd(ch, "qedit $1", "%6d") + "{W] {c%s{x, {C%d{x шаг%I|а|ов, уровни {C%d{x-{C%d{x\r\n";

        for (auto &q: original->quests) {
            buf << fmt(0, lineFormatQuestEdit.c_str(), 
                    q->vnum.getValue(), 
                    q->title.get(LANG_DEFAULT).c_str(),
                    q->steps.size(), q->steps.size(),
                    q->minLevel.getValue(), q->maxLevel.getValue());
        }

        if (!buf.str().empty())
            ch->pecho("Quests:\r\n" + buf.str());
        else
            ch->pecho("Quests:     (none) {D({y{hcquest create{x{D){x\r\n");

    } else {
        ptc(ch, "Quests:     {Dno quests for the new area, save and use {yquest create{x\r\n");
    }

    return false;
}

static AreaQuest * arg_areaquest(const DLString &arg, AreaIndexData *pArea)
{
    Integer questId;

    if (arg.isNumber() && Integer::tryParse(questId, arg)) {
        auto q = pArea->questMap.find(questId);
        if (q != pArea->questMap.end())        
            return q->second;
    }

    return 0;
}

AEDIT(quest, "квест", "редактировать квесты в зоне")
{
    AreaIndexData *original = get_area_data(vnum);
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (!original) {
        ptc(ch, "No quests for the new area, save and use {yquest create{x\r\n");
        return false;
    }

    // 'quest create' 'quest create 2300'
    if (arg_is(arg, "create")) {
        DLString argVnum = args.getOneArgument();
        Integer vnum;

        // 'quest create' - assign next free vnum
        if (argVnum.empty()) {
            for (vnum = original->min_vnum; vnum <= original->max_vnum; vnum++) {
                if (original->questMap.find(vnum) == original->questMap.end())
                    break;
            }

            if (vnum > original->max_vnum) {
                ptc(ch, "В зоне не осталось свободного внума для квеста.\r\n");
                return false;
            }

        } // 'quest create <vnum>' - check if vnum is valid
        else if (!argVnum.isNumber() || !Integer::tryParse(vnum, argVnum)) {
            ptc(ch, "Формат: quest create [номер]\r\n");
            return false;
        }

        if (vnum < original->min_vnum || vnum > original->max_vnum) {
            ptc(ch, "Номер квеста должен лежать в диапазоне от %d до %d.\r\n",
                original->min_vnum, original->max_vnum);
            return false;
        }

        AreaQuest::XMLPointer newQuest(NEW);
        newQuest->vnum.setValue(vnum);
        newQuest->pAreaIndex = original;

        original->questMap[vnum] = *newQuest;
        original->quests.push_back(newQuest);
        areaQuests[vnum] = *newQuest;

        OLCStateAreaQuest::Pointer qedit(NEW, *newQuest);
        qedit->attach(ch);
        qedit->show(ch);
        return true;
    }   

    // 'quest delete 2300'
    if (arg_is(arg, "del")) {
        arg = args.getOneArgument();
        AreaQuest *q = arg_areaquest(arg, original);

        if (!q) {
            ptc(ch, "Квест по номеру %s не определен.\r\n", arg.c_str());
            return false;
        }

        original->questMap.erase(q->vnum.getValue());
        areaQuests.erase(q->vnum.getValue());
        original->quests.remove(q);

        ptc(ch, "Квест удален!\r\n");
        return true;
    }

    // 'quest 2300'
    if (arg.isNumber()) {
        AreaQuest *q = arg_areaquest(arg, original);

        if (!q) {
            ptc(ch, "Квест по номеру %s не определен.\r\n", arg.c_str());
            return false;
        }

        OLCStateAreaQuest::Pointer qedit(NEW, q);
        qedit->attach(ch);
        qedit->show(ch);
        return true;
    }

    ptc(ch, "Формат: quest <vnum>, quest create [<vnum>], quest delete <vnum>\r\n");
    return false;
}

AEDIT(reset, "сбросить", "сбросить арию, обновив всех мобов, предметы и двери")
{
    AreaIndexData *original = get_area_data(vnum);

    if (original) {
        // FIXME reset either all instances or the current one.
        reset_area(original->area, FRESET_ALWAYS);
        stc("Ария сброшена.\n\r", ch);
        return false;
    }

    stc("Создание арии не завершено - нечего сбрасывать.\n\r", ch);
    return false;
}

AEDIT(create, "создать", "создать новую арию")
{
    OLCStateArea::Pointer ae(NEW, (AreaIndexData *)NULL);
    ae->attach(ch);
    ae->findCommand(ch, "show")->entryPoint(ch, "");

    stc("Aрия создана.\n\r", ch);
    return false;
}

AEDIT(name, "имя", "установить имя зоны")
{
    return editor(argument, name[EN], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(uaname, "укимя", "установить имя зоны")
{
    return editor(argument, name[UA], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(runame, "ruимя", "установить имя зоны")
{
    return editor(argument, name[RU], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(message, "сообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[EN], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(uamessage, "уксообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[UA], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(rumessage, "русообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[RU], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(speedwalk, "маршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[EN], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(uaspeedwalk, "укмаршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[UA], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(ruspeedwalk, "румаршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[RU], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(flags, "флаги", "установить или сбросить флаги арии (? area_flags)")
{
    return flagBitsEdit(area_flag);
}

AEDIT(file, "файл", "установить имя файла, в который сохраняется ария")
{
    char file[MAX_STRING_LENGTH];
    int i, length;

    if (ch->getSecurity( ) < 10) {
        stc("Недостаточно прав для изменения имени файла арии.\r\n", ch);
        return false;
    }

    one_argument(argument, file);        /* Forces Lowercase */

    if (!*argument) {
        stc("Синтаксис:  filename [$имя_file]\n\r", ch);
        return false;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen(argument);
    if (length > 64) {
        stc("No more than eight characters allowed.\n\r", ch);
        return false;
    }

    /*
     * Allow only letters and numbers.
     */
    for (i = 0; i < length; i++) {
        if (!isalnum(file[i])) {
            stc("Only letters and numbers are valid.\n\r", ch);
            return false;
        }
    }

    strcat(file, ".are");
    file_name = file;

    stc("Filename set.\n\r", ch);
    return true;
}

AEDIT(security, "права", "установить уровень доступа к арии, 0..9")
{
    char sec[MAX_STRING_LENGTH];
    int value;

    one_argument(argument, sec);

    if (!is_number(sec) || !*sec) {
        stc("Syntax:  security [#xlevel]\n\r", ch);
        return false;
    }

    value = atoi(sec);

    if (value > ch->getSecurity( ) || value < 0) {
        if (ch->getSecurity() != 0) {
            ch->pecho("Security is 0-%d.", ch->getSecurity());
        }
        else
            stc("Security is 0 only.\n\r", ch);
        return false;
    }

    security = value;

    stc("Security set.\n\r", ch);
    return true;
}

AEDIT(authors, "авторы", "установить имена создателей арии или none")
{
    if (!*argument) {
        stc("Syntax:  authors names\n\r", ch);
        stc("Syntax:  authors none\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "none"))
        authors = "none";
    else
        authors = argument;

    return true;
}

AEDIT(translator, "переводчик", "установить имена переводчиков или none")
{
    if (!*argument) {
        stc("Syntax:  translator names\n\r", ch);
        stc("Syntax:  translator none\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "none"))
        translator = "none";
    else
        translator = argument;

    return true;
}


AEDIT(vnums, "внумы", "устаноить диапазон внумов (нижний верхний)")
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }
    
    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  vnum [#xlower] [#xupper]\n\r", ch);
        return false;
    }

    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    max_vnum = iupper;
    stc("Lower & Upper vnums set.\n\r", ch);    
    return true;
}

AEDIT(lvnum, "нвнум", "установить нижнюю границу диапазона внумов")
{
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, lower);

    if (!is_number(lower) || !*lower) {
        stc("Syntax:  min_vnum [#xlower]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = max_vnum;

    if (ilower > iupper) {
        stc("OLCStateArea:  Value must be less than the max_vnum.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    stc("Lower vnum set.\n\r", ch);
    return true;
}

AEDIT(uvnum, "ввнум", "установить верхнюю границу диапазона внумов")
{
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, upper);

    if (!is_number(upper) || !*upper) {
        stc("Syntax:  max_vnum [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = min_vnum;
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    max_vnum = iupper;
    stc("Upper vnum set.\n\r", ch);
    return true;
}

AEDIT(levels, "уровни", "установить рекомендуемые уровни арии (нижний верхний)")
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  levels [#xlower] [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    low_range = ilower;
    high_range = iupper;
    stc("Level range set.\n\r", ch);
    return true;
}

AEDIT(behavior, "поведение", "запустить строковый редактор для установки поведения (XML)")
{
    if (argument[0] == '\0') {
        if(!sedit(behavior))
            return false;

        stc("Behavior set.\n\r", ch);
        return true;
    }

    if (!str_cmp( argument, "clear" )) {
        behavior.clear( );
        stc("Поведение очищено.\r\n", ch);
        return true;
    }

    stc("Syntax:  behavior\n\r", ch);
    stc("Syntax:  behavior clear\n\r", ch);
    return false;
}

AEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

AEDIT(done, "готово", "выйти из редактора (не забывайте про asave changed)") 
{
    commit();
    detach(ch);
    return true;
}

AEDIT(cancel, "отменить", "отменить все изменения и выйти из редактора")
{
    detach(ch);
    return false;
}

AEDIT(dump, "вывод", "(отладка) вывести внутреннее состояние редактора")
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}

CMD(aedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online area editor.")
{
    AreaIndexData *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    pArea = ch->in_room->areaIndex();

    argument = one_argument(argument, arg);
    if (is_number(arg)) {
        value = atoi(arg);
        pArea = get_area_data(value);
        if (!pArea) {
            stc("That area vnum does not exist.\n\r", ch);
            return;
        }
    }
    else {
        if (!str_cmp(arg, "create")) {
            if (ch->getSecurity() < 10) {
                stc("Insuficiente seguridad para crear areas.\n\r", ch);
                return;
            }
            OLCStateArea::Pointer ae(NEW, (AreaIndexData *)NULL);
            ae->attach(ch);
            ae->findCommand(ch, "show")->entryPoint(ch, "");
            stc("Ария создана.\r\n", ch);
            return;
        }
    }

    if (!OLCState::can_edit(ch, pArea)) {
        stc("У тебя недостаточно прав для редактирования арий.\n\r", ch);
        return;
    }

    OLCStateArea::Pointer ae(NEW, pArea);
    ae->attach(ch);
    ae->findCommand(ch, "show")->entryPoint(ch, "");
}

