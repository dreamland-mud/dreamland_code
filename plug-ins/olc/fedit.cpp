
#include "fedit.h"
#include "olc.h"
#include "security.h"
#include "json_utils_ext.h"

#include "configurable.h"
#include "merc.h"
#include "arg_utils.h"
#include "act.h"
#include "comm.h"
#include "websocketrpc.h"
#include <fstream>
#include <sstream>

#include "def.h"

extern void help_save_ids();
bool text_match_with_highlight(const DLString &text, const DLString &args, ostringstream &matchBuf);

// Pushing a huge file into the in-client web editor crashes the client (the
// translations catalog hit 450 KB and froze the mudjs tab). Above this size the
// web editor is refused with a hint to edit the file on disk instead.
static const size_t FEDIT_WEB_MAX = 200 * 1024;

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
    original->save();

    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
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
    ptc(ch, "{WКонфигурационный файл {c%s{W:\r\n", path.c_str());
    ptc(ch, "{WТекст{x: %s\r\n%s\r\n{R...{x\r\n", 
        web_edit_button(ch, "text", "web").c_str(),
        text.substr(0, 100).c_str());
    ptc(ch, "{WКоманды{x: {y{hccommands{x, {y{hcshow{x, {y{hccancel{x, {y{hcdone{x\r\n");
}

bool OLCStateFile::validate(PCharacter *ch) const
{
    ostringstream errbuf;

    if (!JsonUtils::validate(text, errbuf)) {
        ch->pecho("Ошибка парсинга JSON:");
        ch->pecho(errbuf.str());
        ch->pecho("Отредактируйте текст еще раз ({y{hctext web{x) или отмените изменения ({y{hccancel{x).");
        return false;
    }

    ch->pecho("JSON выглядит хорошо.");
    return true;
}


FEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

FEDIT(text, "текст", "редактировать текст справки")
{
    DLString cmdArgs = argument;
    if (arg_is_web(cmdArgs.getOneArgument()) && text.size() > FEDIT_WEB_MAX) {
        ch->pecho("Файл слишком большой (%d КБ) для редактора в клиенте -- он завесит окно.",
                  (int)(text.size() / 1024));
        ch->pecho("Редактируй его на диске и перезагрузи через {y{hcplug reload most{x,");
        ch->pecho("либо разбей на файлы помельче (для translations -- по подсистемам).");
        return false;
    }

    return editor(argument, text, (editor_flags)(ED_JSON|ED_CALL_DONE));
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

// fedit reload <path>: re-read one configurable from disk and run its normal
// load path, so its CONFIGURABLE_LOADED handler rebuilds whatever it caches,
// without a plugin reload (the same loaded() call fedit's own 'done' makes).
// The file is parsed first: load() hands even a half-parsed value to loaded(),
// and the handlers clear their tables before refilling them.
static void fedit_reload(Character *ch, DLString name)
{
    if (name.empty()) {
        ch->pecho("Формат: fedit reload <json path>, например fedit reload config/aging");
        return;
    }

    if (name.size() > 5 && name.substr(name.size() - 5) == ".json")
        name = name.substr(0, name.size() - 5);

    Configurable::Pointer cfg = configReg->get(name);
    if (!cfg) {
        list<Configurable::Pointer> matches = configReg->getAll(name);
        if (matches.empty()) {
            ch->pecho("Не найдено ни одного файла с именем {W%s{x.", name.c_str());
            return;
        }
        if (matches.size() > 1) {
            ch->pecho("Под {W%s{x подходит несколько файлов, укажи точный путь:", name.c_str());
            for (auto &m: matches)
                ch->pecho("    %s", m->getPath().c_str());
            return;
        }
        cfg = matches.front();
    }

    try {
        std::ifstream ifs(cfg->getAbsolutePath().c_str());
        if (!ifs) {
            ch->pecho("Не могу прочитать {W%s{x.", cfg->getAbsolutePath().c_str());
            return;
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();

        ostringstream errbuf;
        if (!JsonUtils::validate(buffer.str(), errbuf)) {
            ch->pecho("Ошибка парсинга JSON в {W%s{x, файл не перезагружен:", cfg->getPath().c_str());
            ch->pecho("%s", errbuf.str().c_str());
            return;
        }

        cfg->load();
        ch->pecho("Файл {W%s{x перезагружен с диска: %d записей.",
                  cfg->getPath().c_str(), (int)cfg->getValue().size());

    } catch (const std::exception &ex) {
        ch->pecho("Ошибка при перезагрузке {W%s{x: %s", cfg->getPath().c_str(), ex.what());
    }
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
        stc("         fedit reload <json path> - перечитать файл с диска без перезагрузки плагина\r\n", ch);
        return;
    }

    if (arg_oneof_strict(arg1, "reload")) {
        fedit_reload(ch, arg2);
        return;
    }

    if (arg_is_list(arg1)) {
        ostringstream buf;
        const DLString lineFormat = "    " + web_cmd(ch, "fedit $1|text web", "%s");
        DLString lastFolder = "\x01"; // sentinel: no real folder compares equal

        // Print a folder header whenever the folder prefix changes. getPaths() returns
        // "folder/file" strings from a sorted map, so same-folder entries are contiguous;
        // files with no folder are listed without a header.
        auto groupHeader = [&](const DLString &path) {
            DLString::size_type slash = path.rfind('/');
            DLString folder = (slash == DLString::npos) ? DLString("") : path.substr(0, slash);
            if (folder != lastFolder) {
                lastFolder = folder;
                if (!folder.empty())
                    buf << "{C" << folder << "/{x" << endl;
            }
        };

        if (arg2.empty()) {
            buf << "Все конфигурационные файлы для редактирования:" << endl;
            for (auto &path: configReg->getPaths()) {
                groupHeader(path);
                buf << fmt(0, lineFormat.c_str(), path.c_str()) << endl;
            }

        } else {
            buf << "Файлы, содержащие {W" << arg2 << "{x в имени:" << endl;
            for (auto &path: configReg->getPaths()) {
                if (path.find(arg2) == DLString::npos)
                    continue;
                groupHeader(path);
                buf << fmt(0, lineFormat.c_str(), path.c_str()) << endl;
            }
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    list<Configurable::Pointer> matches = configReg->getAll(arg1);
    if (matches.empty()) {
        ch->pecho("Не найдено ни одного файла с именем {W%s{x.", arg1.c_str());
        return;
    }

    Configurable::Pointer cfg = matches.front();
    if (matches.size() > 1) {
        ch->pecho("Found {W%1$d{x file(s), editing the 1st in the list:\r\n", matches.size());
    }

    OLCStateFile::Pointer fe(NEW, cfg);
    fe->attach(ch);
    fe->show(ch);
    fe->handle(ch->desc, const_cast<char *>("text web"));
}

