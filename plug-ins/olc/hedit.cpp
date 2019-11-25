
#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "room.h"
#include "areahelp.h"

#include "hedit.h"
#include "olc.h"
#include "security.h"

#include "merc.h"
#include "update_areas.h"
#include "interp.h"
#include "mercdb.h"
#include "def.h"

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
}

OLCStateHelp::~OLCStateHelp() 
{
}

static HelpArticle::Pointer help_find_by_id(int id)
{
    HelpArticles::const_iterator a;
    HelpArticle::Pointer original;

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
        if ((*a)->getID() == id) {
            original = const_cast<HelpArticle *>(a->getPointer());
            break;
        }
    }
    return original;
}

list<HelpArticle::Pointer> help_find_by_keywords(const DLString &keywords)
{
    HelpArticles::const_iterator a;
    list<HelpArticle::Pointer> originals;

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
        if (is_name(keywords.c_str(), (*a)->getKeywordAttribute().c_str()))
            originals.push_back(*a);
    }

    return originals;
}

HelpArticle::Pointer OLCStateHelp::getOriginal() const
{
    HelpArticle::Pointer original;

    // First look up by unique ID, unless plugins have been reloaded while a builder was
    // insider 'hedit'.
    original = help_find_by_id(id);

    // If IDs have been changed since hedit had started, try looking up by keyword.
    if (!original) {
        list<HelpArticle::Pointer> matches = help_find_by_keywords(keywords);
        if (matches.size() > 1) 
            throw Exception("Cannot find unique match for help keyword, help not saved");
        original = matches.front();
    }

    if (!original)
        throw Exception("Cannot locate original help article, help not saved");

    return original;
}

void OLCStateHelp::commit() 
{
    HelpArticle::Pointer original = getOriginal();

    original->setKeywordAttribute(keywords);
    original->setLevel(level);
    original->setText(text);
    original->save();
}

void OLCStateHelp::statePrompt(Descriptor *d) 
{
    d->send( "Editing help> " );
}

void OLCStateHelp::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateHelp::show( PCharacter *ch ) const
{
    ptc(ch, "{WСтатья справки под номером {c%d{x:\r\n", id.getValue());
    ptc(ch, "{WКлючевые слова{x:    [{C%s{x]\r\n", keywords.c_str());
    ptc(ch, "{WУровень{x:           [{C%d{x]\r\n", level.getValue());
    ptc(ch, "{WТекст{x:\r\n%s\r\n", text.c_str());
    ptc(ch, "{WКоманды{x: keywords, level, text copy, text paste\r\n");
}

HEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

HEDIT(keywords, "ключевые", "установить слова, на которые откликается справка")
{
    if (!*argument) {
        stc("Синтаксис:   keywords 'long keyword' shortkeyword etc\n\r", ch);
        return false;
    }

    keywords = argument;
    keywords.toUpper();
    ptc(ch, "Новые ключевые слова: %s\n\r", keywords.c_str());
    return true;
}

HEDIT(level, "уровень", "установить уровень, с которого доступна справка")
{
    return numberEdit(-1, MAX_LEVEL, level);
}

HEDIT(text, "текст", "редактировать текст справки")
{
    return editor(argument, text);
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

HEDIT(dump, "вывод", "(отладка) вывести внутреннее состояние редактора")
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
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
        help = help_find_by_id(id);
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
            stc("Найдено несколько статей справки, пожалуйста, уточните поиск:\r\n", ch);
            for (list<HelpArticle::Pointer>::const_iterator m = matches.begin();  m != matches.end(); m++)
                ptc(ch, "     %s\r\n", (*m)->getKeywordAttribute().c_str());
            return;
        }

        help = matches.front();
    }

    OLCStateHelp::Pointer he(NEW, help.getPointer());
    he->attach(ch);
    he->show(ch);
}

