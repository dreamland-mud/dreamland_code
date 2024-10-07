
#include <pcharacter.h>
#include <commandmanager.h>
#include "room.h"
#include "areahelp.h"
#include "helpcontainer.h"

#include "hedit.h"
#include "olc.h"
#include "security.h"
#include "string_utils.h"
#include "arg_utils.h"
#include "act.h"
#include "comm.h"
#include "websocketrpc.h"
#include "interp.h"

#include "def.h"

extern void help_save_ids();
bool text_match_with_highlight(const DLString &text, const DLString &args, ostringstream &matchBuf);

OLC_STATE(OLCStateHelp);

OLCStateHelp::OLCStateHelp() : id(-1), level(-1), isChanged(false)
{
}

OLCStateHelp::OLCStateHelp(HelpArticle *original) : id(-1), level(-1), isChanged(false)
{
    if (!original)
        return;

    this->id = original->getID();
    this->level = original->getLevel();
    this->text = original->text;
    this->keyword = original->keyword;
    this->extra = original->extra;
    this->labels = original->labels.persistent.toString();
    this->labelsAuto = original->labels.transient.toString();
    this->title = original->title;
    this->autotitle = original->getTitle(DLString::emptyString);
}

OLCStateHelp::~OLCStateHelp() 
{
}

list<HelpArticle::Pointer> help_find_by_keywords(const DLString &keywords)
{
    HelpArticles::const_iterator a;
    list<HelpArticle::Pointer> originals;

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
        if (is_name(keywords.c_str(), (*a)->getAllKeywordsString().c_str())
            || is_name(keywords.c_str(), (*a)->aka.toString().c_str()))
            originals.push_back(*a);
    }

    return originals;
}

void OLCStateHelp::commit() 
{
    HelpArticle::Pointer original = helpManager->getArticle(id);

    original->keyword = keyword;
    original->setLevel(level);
    original->text = text;
    original->title = title;
    original->labels.persistent.clear();
    original->labels.addPersistent(labels);
    original->extra = extra;
    original->refreshKeywords();
    original->save();

    help_save_ids();
}

void OLCStateHelp::statePrompt(Descriptor *d) 
{
    d->send( "Help> " );
}

void OLCStateHelp::changed( PCharacter *ch )
{
    isChanged = true;
}

/**
 * How all keywords for this article will look like after commit:
 * a combination of automatic keywords and currently edited keyword attribute.
 */
StringSet OLCStateHelp::allKeywords() const
{
    StringSet kw;

    HelpArticle::Pointer article = helpManager->getArticle(id);
    const StringSet &autoKeywords = article->getAutoKeywords();
    kw.insert(autoKeywords.begin(), autoKeywords.end());

    kw.fromString(keyword.get(EN));
    kw.fromString(keyword.get(RU));
    return kw;
}

void OLCStateHelp::show( PCharacter *ch ) const
{
    ptc(ch, "{WСтатья справки под номером {c%d{W, метки в файле {c%s{W, автоматические метки {c%s{x:\r\n", 
        id.getValue(), 
        labels.empty() ? "-" : labels.c_str(), 
        labelsAuto.empty() ? "-" : labelsAuto.c_str());     
    ptc(ch, "{DВсе ключевые слова:   [%s]\r\n", allKeywords().toString().c_str());
    ptc(ch, "{WКлючевые слова, EN{x:   [{C%s{x] %s\r\n", keyword.get(EN).c_str(), web_edit_button(ch, "keywords", "web").c_str());
    ptc(ch, "{WКлючевые слова, UA{x:   [{C%s{x] %s\r\n", keyword.get(UA).c_str(), web_edit_button(ch, "uakeywords", "web").c_str());
    ptc(ch, "{WКлючевые слова, RU{x:   [{C%s{x] %s\r\n", keyword.get(RU).c_str(), web_edit_button(ch, "rukeywords", "web").c_str());
    ptc(ch, "{WСкрытые алиасы, EN{x:   [{C%s{x] %s\r\n", extra.get(EN).c_str(), web_edit_button(ch, "aka", "web").c_str());
    ptc(ch, "{WСкрытые алиасы, UA{x:   [{C%s{x] %s\r\n", extra.get(UA).c_str(), web_edit_button(ch, "uaaka", "web").c_str());
    ptc(ch, "{WСкрытые алиасы, RU{x:   [{C%s{x] %s\r\n", extra.get(RU).c_str(), web_edit_button(ch, "ruaka", "web").c_str());
    ptc(ch, "{WУровень{x:              [{C%d{x]\r\n", level.getValue());
    ptc(ch, "{WЗаголовок, EN{x:        [{C%s{x] %s\r\n", title.get(EN).c_str(), web_edit_button(ch, "title", "web").c_str());
    ptc(ch, "{WЗаголовок, UA{x:        [{C%s{x] %s\r\n", title.get(UA).c_str(), web_edit_button(ch, "uatitle", "web").c_str());
    ptc(ch, "{WЗаголовок, RU{x:        [{C%s{x] %s\r\n", title.get(RU).c_str(), web_edit_button(ch, "rutitle", "web").c_str());
    ptc(ch, "{DАвтозаголовок:        [%s]\r\n", autotitle.c_str());

    DLString entext = text.get(EN);
    DLString uatext = text.get(UA);
    DLString rutext = text.get(RU);
    ptc(ch, "{WТекст, EN{x: %s{W...{x\r\n%s\r\n", web_edit_button(ch, "entext", "web").c_str(), String::truncate(entext, 200).c_str());
    ptc(ch, "{WТекст, UA{x: %s{W...{x\r\n%s\r\n", web_edit_button(ch, "uatext", "web").c_str(), String::truncate(uatext, 200).c_str());
    ptc(ch, "{WТекст, RU{x: %s{W...{x\r\n%s\r\n", web_edit_button(ch, "rutext", "web").c_str(), String::truncate(rutext, 200).c_str());

    ptc(ch, "{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ycancel{x, {hc{ydone{x\r\n");
}

HEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

HEDIT(keywords, "ключевые", "установить или очистить дополнительные ключевые слова")
{
    return editor(argument, keyword[EN], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(uakeywords, "укключевые", "установить или очистить дополнительные ключевые слова")
{
    return editor(argument, keyword[UA], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(rukeywords, "руключевые", "установить или очистить дополнительные ключевые слова")
{
    return editor(argument, keyword[RU], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(aka, "скрытые", "установить или очистить скрытые алиасы")
{
    return editor(argument, extra[EN], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(uaaka, "укскрытые", "установить или очистить скрытые алиасы")
{
    return editor(argument, extra[UA], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(ruaka, "рускрытые", "установить или очистить скрытые алиасы")
{
    return editor(argument, extra[RU], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(title, "заголовок", "установить или очистить заголовок вместо автоматического")
{
    return editor(argument, title[EN], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(uatitle, "укзаголовок", "установить или очистить заголовок вместо автоматического")
{
    return editor(argument, title[UA], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}

HEDIT(rutitle, "рузаголовок", "установить или очистить заголовок вместо автоматического")
{
    return editor(argument, title[RU], (editor_flags)(ED_HELP_HINTS|ED_NO_NEWLINE));
}


HEDIT(labels, "метки", "установить метки для экспорта на сайт")
{
    if (!*argument) {
        stc("Синтаксис:   labels <new labels>\n\r", ch);
        return false;
    }

    labels = argument;
    labels.toLower();
    ptc(ch, "Новые метки: %s\n\r", labels.c_str());
    return true;
}

HEDIT(level, "уровень", "установить уровень, с которого доступна справка")
{
    return numberEdit(-1, MAX_LEVEL, level);
}

HEDIT(text, "текст", "редактировать текст справки")
{
    return editor(argument, text[EN], ED_HELP_HINTS);
}

HEDIT(uatext, "уктекст", "редактировать текст справки")
{
    return editor(argument, text[UA], ED_HELP_HINTS);
}

HEDIT(rutext, "рутекст", "редактировать текст справки")
{
    return editor(argument, text[RU], ED_HELP_HINTS);
}

HEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

HEDIT(done, "готово", "выйти из редактора (не забывайте про asave changed)") 
{
    commit();
    detach(ch);
    return true;
}

HEDIT(cancel, "отменить", "отменить все изменения и выйти из редактора")
{
    detach(ch);
    return false;
}

CMD(hedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online help editor.")
{
    DLString args = argument, tmpArgs(argument);
    HelpArticle::Pointer help;
    Integer id;
    DLString arg1 = tmpArgs.getOneArgument();
    DLString arg2 = tmpArgs;

    if (args.empty()) {
        stc("Формат:  hedit ключевые слова\r\n", ch);
        stc("         hedit <id>\r\n", ch);
        stc("         hedit list           - список всех статей\r\n", ch);
        stc("         hedit create         - создать новую пустую статью справки\r\n", ch);
        stc("         hedit search <text>  - поиск в тексте статей, включая ссылки (hh123)\r\n", ch);
        stc("         hedit save           - сохранить все статьи справки на диск\r\n", ch);
        stc("         hedit label          - список всех меток\r\n", ch);
        stc("         hedit label <name>   - список всех статей у этой метки\r\n", ch);
        stc("         hedit label none     - список всех статей без единой метки\r\n", ch);
        return;
    }

    if (args.isNumber() && Integer::tryParse(id, args)) {
        help = helpManager->getArticle(id);
        if (!help) {
            stc("Справка с таким ID не найдена.\r\n", ch);
            return;
        }
    } else if (arg_oneof(arg1, "save")) {        
        for (auto &ai: helpManager->getArticles()) {
            HelpArticle *a = const_cast<HelpArticle *>(*ai);
            LogStream::sendNotice() << "Saving help " << a->getID() << " " << a->getAllKeywordsString() << endl;
            a->save();
        }

        return;
    } else if (arg_is_list(arg1)) {
        ostringstream buf;
        for (auto &a: helpManager->getArticles()) {
            buf << fmt(0, "[%6d] [%-30.30s{x] {D%-30.30s{x\r\n", 
                a->getID(), a->getTitle("").c_str(), a->getAllKeywordsString().c_str());
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    } else if (arg_oneof_strict(arg1, "search")) {
        ostringstream buf;
        const DLString lineFormat = web_cmd(ch, "hedit $1", "%4d") + " {C%s{x\r\n";

        if (arg2.empty()) {
            stc("Формат: hedit search <строка>\r\n", ch);
            return;
        }

        buf << "Статьи справки, содержащие '" << arg2 << "{x': " << endl;
        for (auto &a: helpManager->getArticles()) {
            ostringstream matchBuf;

            if (text_match_with_highlight(a->text.get(RU).c_str(), arg2, matchBuf)) {
                buf << fmt( 0, lineFormat.c_str(), a->getID(), a->getAllKeywordsString().c_str())
                    << matchBuf.str()
                    << endl;
            }
        }

        page_to_char(buf.str().c_str(), ch);
        return;

    } else if (arg_oneof_strict(arg1, "label"))  {
        // 'hedit label' - just show total counts
        if (arg2.empty()) {
            map<DLString, int> activeLabelCount;
            const DLString lineFormat = "[{c" + web_cmd(ch, "hedit label $1", "%12s") + "{x], статей {W%d{x";

            for (auto &a: helpManager->getArticles()) {
                for (auto &l: (*a)->labels.all) 
                    activeLabelCount[l]++;
                if ((*a)->labels.all.empty())
                    activeLabelCount["none"]++;
            }

            for (auto &kv: activeLabelCount) {
                ch->pecho(fmt(0, lineFormat.c_str(), kv.first.c_str(), kv.second));
            }
            return;
        }

        // 'hedit label <arg>' - show articles under this label
        list<int> labeledIds;
        for (auto &a: helpManager->getArticles())
            if (arg2 == "none") {
                if ((*a)->labels.all.empty())
                    labeledIds.push_back(a->getID());
            } else if ((*a)->labels.all.count(arg2) != 0)
                labeledIds.push_back(a->getID());

        if (labeledIds.empty()) {
            ch->pecho("Не найдено ни одной статьи с этой меткой, укажите название полностью.");
            return;
        }

        ch->pecho("Cписок всех статей с меткой {c%s{x:", arg2.c_str());
        const DLString lineFormat = web_cmd(ch, "hedit $1", "%4d") + "     %s";
        for (auto &id: labeledIds) {
            HelpArticle::Pointer a = helpManager->getArticle(id);
            ch->pecho(lineFormat.c_str(), a->getID(), a->getAllKeywordsString().c_str());
        }
        return;

    } else if (arg_oneof_strict(args, "create", "создать")) {
        for (auto &container: HelpLoader::getThis()->getElements()) {
            // Create a new empty node inside help.xml and properly register it.
            if (container->getName() == "help" && container.getDynamicPointer<HelpContainer>()) {
                HelpContainer *hc = container.getConstPointer<HelpContainer>();
                GenericHelp::Pointer help(NEW);
                help->setContainer(hc);
                hc->push_back(help);
                help->setID(help_next_free_id());
                helpManager->registrate(help);

                OLCStateHelp::Pointer hedit(NEW, help.getPointer());
                hedit->attach(ch);
                hedit->show(ch);
                return;
            }
        }

        ch->pecho("Не могу найти файл для сохранения справки.");
        return;

    } else {
        list<HelpArticle::Pointer> matches = help_find_by_keywords(args);

        if (matches.size() == 0) {
            stc("Не найдено ни одной статьи справки по этим ключевым словам.\r\n", ch);
            return;
        }

        if (matches.size() > 1) {
            stc("Найдено несколько статей справки, используйте ID, чтобы прицелиться получше:\r\n", ch);
            const DLString lineFormat = web_cmd(ch, "hedit $1", "%4d") + "     %s\r\n";
            for (list<HelpArticle::Pointer>::const_iterator m = matches.begin();  m != matches.end(); m++)
                ptc(ch, lineFormat.c_str(), (*m)->getID(), (*m)->getAllKeywordsString().c_str());
            return;
        }

        help = matches.front();
    }

    if (help->getID() <= 0) {
        stc("Эту статью справки невозможно сохранить на диск.\r\n", ch);
        return;
    }

    OLCStateHelp::Pointer he(NEW, help.getPointer());
    he->attach(ch);
    he->show(ch);
}

