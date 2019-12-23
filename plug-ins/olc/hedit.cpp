
#include <pcharacter.h>
#include <commandmanager.h>
#include "room.h"
#include "areahelp.h"

#include "hedit.h"
#include "olc.h"
#include "security.h"

#include "merc.h"
#include "arg_utils.h"
#include "websocketrpc.h"
#include "interp.h"
#include "mercdb.h"
#include "def.h"

extern void help_save_ids();

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
    this->text = original->c_str();
    this->keywords = original->getKeywordAttribute();
    this->labels = original->labels.persistent.toString();
    this->labelsAuto = original->labels.transient.toString();
}

OLCStateHelp::~OLCStateHelp() 
{
}

list<HelpArticle::Pointer> help_find_by_keywords(const DLString &keywords)
{
    HelpArticles::const_iterator a;
    list<HelpArticle::Pointer> originals;

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
        if (is_name(keywords.c_str(), (*a)->getAllKeywordsString().c_str()))
            originals.push_back(*a);
    }

    return originals;
}

void OLCStateHelp::commit() 
{
    HelpArticle::Pointer original = helpManager->getArticle(id);

    original->setKeywordAttribute(keywords);
    original->setLevel(level);
    original->setText(text);
    original->labels.persistent.clear();
    original->labels.addPersistent(labels);
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

    kw.fromString(keywords);
    return kw;
}

void OLCStateHelp::show( PCharacter *ch ) const
{
    ptc(ch, "{WСтатья справки под номером {c%d{W, метки в файле {c%s{W, автоматические метки {c%s{x:\r\n", 
        id.getValue(), 
        labels.empty() ? "-" : labels.c_str(), 
        labelsAuto.empty() ? "-" : labelsAuto.c_str());     
    ptc(ch, "{DВсе ключевые слова: [%s]\r\n", allKeywords().toString().c_str());
    ptc(ch, "{WКлючевые слова{x:     [{C%s{x]\r\n", keywords.c_str());
    ptc(ch, "{WУровень{x:            [{C%d{x]\r\n", level.getValue());
    ptc(ch, "{WТекст{x: %s\r\n%s\r\n", 
        web_edit_button(ch, "text", "web").c_str(),
        text.c_str());
    ptc(ch, "{WКоманды{x: commands, show, cancel, done\r\n");
}

HEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

HEDIT(keywords, "ключевые", "установить или очистить дополнительные ключевые слова")
{
    if (!*argument) {
        stc("Синтаксис:   keywords 'long keyword' shortkeyword\n\r", ch);
        stc("             keywords clear\n\r", ch);
        return false;
    }

    if (arg_oneof_strict(argument, "clear", "очистить")) {
        keywords.clear();
        stc("Ключевые слова очищены.\r\n", ch);
        return true;
    }

    keywords = argument;
    keywords.toUpper();
    ptc(ch, "Новые ключевые слова: %s\n\r", keywords.c_str());
    return true;
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
    return editor(argument, text, ED_HELP_HINTS);
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
    DLString args = argument;
    HelpArticle::Pointer help;
    Integer id;

    if (args.empty()) {
        stc("Формат:  hedit ключевые слова\r\n", ch);
        stc("         hedit <id>\r\n", ch);
        return;
    }

    if (args.isNumber() && Integer::tryParse(id, args)) {
        help = helpManager->getArticle(id);
        if (!help) {
            stc("Справка с таким ID не найдена.\r\n", ch);
            return;
        }
    }
    else {
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

