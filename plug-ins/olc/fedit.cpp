
#include "fedit.h"
#include "olc.h"
#include "security.h"
#include <jsoncpp/json/json.h>

#include "configurable.h"
#include "merc.h"
#include "arg_utils.h"
#include "act.h"
#include "comm.h"
#include "websocketrpc.h"
#include "mercdb.h"
#include "def.h"

extern void help_save_ids();
bool text_match_with_highlight(const DLString &text, const DLString &args, ostringstream &matchBuf);

OLC_STATE(OLCStateFile);

OLCStateFile::OLCStateFile() : isChanged(false)
{
}

OLCStateFile::OLCStateFile(Configurable::Pointer original) : isChanged(false)
{
    if (!original)
        return;

    this->path = original->getPath();
    this->text = original->getText();
}

OLCStateFile::~OLCStateFile() 
{
}

void OLCStateFile::commit() 
{
    Configurable::Pointer original = configReg->get(path);
    original->setText(text);

    if (owner)
        owner->character->println("Изменения сохранены на диск.");
}

void OLCStateFile::statePrompt(Descriptor *d) 
{
    d->send( "File> " );
}

void OLCStateFile::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateFile::show( PCharacter *ch ) const
{
    ptc(ch, "{WКофигурационный файл {c%s{W:\r\n", path.c_str());
    ptc(ch, "{WТекст{x: %s\r\n%s\r\n{R...{x\r\n", 
        web_edit_button(ch, "text", "web").c_str(),
        text.substr(0, 100).c_str());
    ptc(ch, "{WКоманды{x: {y{hccommands{x, {y{hcshow{x, {y{hccancel{x, {y{hcdone{x\r\n");
}

bool OLCStateFile::validate(PCharacter *ch) const
{
    ostringstream errbuf;
    Configurable::Pointer original = configReg->get(path);

    if (!original->validate(text, errbuf)) {
        ch->println("Ошибка парсинга JSON:");
        ch->println(errbuf.str());
        ch->println("Отредактируйте текст еще раз ({y{hctext web{x) или отмените изменения ({y{hccancel{x).");
        return false;
    }

    ch->println("JSON выглядит хорошо.");
    return true;
}


FEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

FEDIT(text, "текст", "редактировать текст справки")
{
    return editor(argument, text, ED_JSON);
}

FEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

FEDIT(done, "готово", "выйти из редактора") 
{
    if (!validate(ch))
        return false;

    commit();
    detach(ch);
    return true;
}

FEDIT(cancel, "отменить", "отменить все изменения и выйти из редактора")
{
    detach(ch);
    return false;
}

CMD(fedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online configuration file editor.")
{
    DLString args = argument;
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args;

    if (arg1.empty()) {
        stc("Формат:  fedit <json path>    - редактировать файл\r\n", ch);
        stc("         fedit list           - список всех конфигурационных файлов\r\n", ch);
        stc("         fedit list <строка>  - список всех файлов со строкой в имени\r\n", ch);
        return;
    }

    if (arg_is_list(arg1)) {
        ostringstream buf;
        const DLString lineFormat = "    " + web_cmd(ch, "fedit $1", "%s");

        if (arg2.empty()) {
            buf << "Все конфигурационные файлы для редактирования:" << endl;
            for (auto &path: configReg->getPaths())
                buf << fmt(0, lineFormat.c_str(), path.c_str()) << endl;

        } else {
            buf << "Файлы, содержащие {W" << arg2 << "{x в имени:" << endl;
            for (auto &path: configReg->getPaths())
                if (path.find(arg2) != DLString::npos)
                    buf << fmt(0, lineFormat.c_str(), path.c_str()) << endl;
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    } 

    list<Configurable::Pointer> matches = configReg->getAll(arg1);
    if (matches.empty()) {
        ch->printf("Не найдено ни одного файла с именем {W%s{x.\r\n", arg1.c_str());
        return;
    }

    Configurable::Pointer cfg = matches.front();
    if (matches.size() > 1) {
        ch->pecho("Найден%1$I|о|ы {W%1$d{x файл%1$I|а|ов, редактирую 1й в списке:\r\n", matches.size());
    }

    OLCStateFile::Pointer fe(NEW, cfg);
    fe->attach(ch);
    fe->show(ch);
}

