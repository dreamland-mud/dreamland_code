
#include <algorithm>
#include <pcharacter.h>
#include <commandmanager.h>
#include "room.h"
#include "areahelp.h"
#include "helpcontainer.h"
#include "helprelated.h"

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

OLCStateHelp::OLCStateHelp() : id(-1), level(-1), seeAlsoEdited(false), refByEdited(false), isChanged(false)
{
}

OLCStateHelp::OLCStateHelp(HelpArticle *original) : id(-1), level(-1), seeAlsoEdited(false), refByEdited(false), isChanged(false)
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
    this->seeAlso = HelpArticle::seeAlsoToString(original->getSeeAlso());
    this->refBy = original->getRefBy().toString();
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
    if (seeAlsoEdited)
        original->setSeeAlso(HelpArticle::seeAlsoFromString(seeAlso));
    if (refByEdited) {
        StringSet names;
        names.fromString(refBy);
        original->setRefBy(names);
    }
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
    HelpArticle::Pointer article = helpManager->getArticle(id);
    DLString category = article->getCategory();
    ptc(ch, "{WКатегория{x:            [{C%s{x] %s{D(по сохранённым меткам){x\r\n",
        category.empty() ? "-" : category.c_str(),
        category.empty() ? "" : (HelpArticle::isPlayerCategory(category) ? "видна игрокам " : "скрыта от игроков "));
    ptc(ch, "{WСм. также, вручную{x:   [{C%s{x]\r\n", seeAlso.empty() ? "-" : seeAlso.c_str());
    for (auto i: HelpArticle::seeAlsoFromString(seeAlso)) {
        HelpArticle::Pointer a = helpManager->getArticle(i);
        ptc(ch, "    %6d %s\r\n", i, a ? a->getTitle(DLString::emptyString).c_str() : "{Rнет такой статьи{x");
    }
    DLString autoSeeAlso;
    for (auto &a: help_related(article.getPointer(), ch))
        autoSeeAlso += fmt(0, " %d", a->getID());
    ptc(ch, "{DСм. также, итог:      [%s ]{x\r\n", autoSeeAlso.c_str());
    if (!refBy.empty()) {
        HelpArticle::Pointer target = article->getAliasTarget();
        ptc(ch, "{WСсылка refby{x:         [{C%s{x]%s\r\n", refBy.c_str(),
            target ? fmt(0, " псевдоним статьи {c%d{x", target->getID()).c_str() : "");
    }
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
    ptc(ch, "{WТекст, EN{x: %s\r\n%s{x\r\n", web_edit_button(ch, "text", "web").c_str(), String::ellipsis(entext, 200).c_str());
    ptc(ch, "{WТекст, UA{x: %s\r\n%s{x\r\n", web_edit_button(ch, "uatext", "web").c_str(), String::ellipsis(uatext, 200).c_str());
    ptc(ch, "{WТекст, RU{x: %s\r\n%s{x\r\n", web_edit_button(ch, "rutext", "web").c_str(), String::ellipsis(rutext, 200).c_str());

    ptc(ch, "{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{yseealso{x, {hc{yalias{x, {hc{ycancel{x, {hc{ydone{x\r\n");
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

HEDIT(seealso, "смтакже", "установить или очистить список статей для строки 'См. также'")
{
    DLString arg = argument;

    if (arg.empty()) {
        stc("Синтаксис:   seealso <id> [<id> ...]  - статьи, которые идут первыми в строке 'См. также'\n\r", ch);
        stc("             seealso clear            - очистить список\n\r", ch);
        return false;
    }

    if (arg_is_clear(arg)) {
        seeAlso.clear();
        seeAlsoEdited = true;
        stc("Список 'См. также' очищен.\n\r", ch);
        return true;
    }

    std::vector<int> ids;
    for (DLString a = arg.getOneArgument(); !a.empty(); a = arg.getOneArgument()) {
        Integer v;
        if (!Integer::tryParse(v, a) || v <= 0) {
            ptc(ch, "'%s' не похоже на номер статьи.\n\r", a.c_str());
            return false;
        }

        HelpArticle::Pointer target = helpManager->getArticle(v);
        if (!target) {
            ptc(ch, "Статьи %d нет.\n\r", v.getValue());
            return false;
        }

        if (v == id.getValue()) {
            stc("Статья не может ссылаться сама на себя.\n\r", ch);
            return false;
        }

        if (target->getAliasTarget()) {
            ptc(ch, "Статья %d -- псевдоним статьи %d, укажи её.\n\r",
                v.getValue(), target->getAliasTarget()->getID());
            return false;
        }

        if (std::find(ids.begin(), ids.end(), (int)v) == ids.end())
            ids.push_back(v);
    }

    seeAlso = HelpArticle::seeAlsoToString(ids);
    seeAlsoEdited = true;
    ptc(ch, "Список 'См. также': %s\n\r", seeAlso.c_str());
    if (ids.size() > 4)
        stc("{YВнимание{x: в строку попадут только первые четыре.\n\r", ch);
    return true;
}

HEDIT(alias, "псевдоним", "сделать статью псевдонимом другой или снять это")
{
    DLString arg = argument;
    HelpArticle::Pointer article = helpManager->getArticle(id);

    if (arg.empty()) {
        stc("Синтаксис:   alias <id>   - справка по этой статье открывает статью <id>\n\r", ch);
        stc("             alias clear  - снять псевдоним\n\r", ch);
        return false;
    }

    for (auto &r: article->getRefBy()) {
        Integer v;
        if (!Integer::tryParse(v, r)) {
            stc("У этой статьи в refby записаны команды, псевдоним её сломает.\n\r", ch);
            return false;
        }
    }

    if (dynamic_cast<AreaHelp *>(article.getPointer())) {
        stc("Справка зоны не сохраняет refby, псевдоним пропадёт после asave.\n\r", ch);
        return false;
    }

    if (arg_is_clear(arg)) {
        refBy.clear();
        refByEdited = true;
        stc("Псевдоним снят.\n\r", ch);
        return true;
    }

    Integer v;
    if (!Integer::tryParse(v, arg) || v <= 0) {
        stc("Укажи номер статьи.\n\r", ch);
        return false;
    }

    HelpArticle::Pointer target = helpManager->getArticle(v);
    if (!target) {
        ptc(ch, "Статьи %d нет.\n\r", v.getValue());
        return false;
    }

    if (v == id.getValue()) {
        stc("Статья не может быть псевдонимом самой себя.\n\r", ch);
        return false;
    }

    if (target->getAliasTarget()) {
        ptc(ch, "Статья %d сама псевдоним статьи %d, укажи её.\n\r",
            v.getValue(), target->getAliasTarget()->getID());
        return false;
    }

    // a skill article carries the skill's stats header, an alias would hide it
    if ((article->labels.all.count("skill") || article->labels.all.count("spell"))
        && (target->labels.all.count("skill") || target->labels.all.count("spell")))
    {
        stc("Справки умений не делают псевдонимами друг друга.\n\r", ch);
        return false;
    }

    refBy = DLString(v.getValue());
    refByEdited = true;
    ptc(ch, "Справка по этой статье теперь открывает статью %d.\n\r", v.getValue());
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
        stc("         hedit label          - категории и остальные метки с числом статей\r\n", ch);
        stc("         hedit label <name>   - статьи этой категории, а если таких нет, с этой меткой\r\n", ch);
        stc("         hedit label none     - статьи без категории\r\n", ch);
        return;
    }

    if (args.isNumber() && Integer::tryParse(id, args)) {
        help = helpManager->getArticle(id);
        if (!help) {
            stc("Справка с таким ID не найдена.\r\n", ch);
            return;
        }
    } else if (arg_is(arg1, "save")) {        
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
    } else if (arg_is_strict(arg1, "search")) {
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

    } else if (arg_is_strict(arg1, "label"))  {
        // 'hedit label' - just show total counts
        if (arg2.empty()) {
            map<DLString, int> categoryCount, facetCount;
            const DLString lineFormat = "[{c" + web_cmd(ch, "hedit label $1", "%12s") + "{x], статей {W%d{x%s";

            for (auto &a: helpManager->getArticles()) {
                DLString category = (*a)->getCategory();
                categoryCount[category.empty() ? "none" : category]++;

                for (auto &l: (*a)->labels.all)
                    if (l != category)
                        facetCount[l]++;
            }

            ch->pecho("{WКатегории{x (одна на статью, по ним строится 'help index'):");
            for (auto &kv: categoryCount)
                ch->pecho(fmt(0, lineFormat.c_str(), kv.first.c_str(), kv.second,
                    kv.first == "none" || HelpArticle::isPlayerCategory(kv.first) ? "" : " {D(скрыта от игроков){x"));

            ch->pecho("\r\n{WОстальные метки{x (фасеты и метки, которые ставит движок):");
            for (auto &kv: facetCount)
                ch->pecho(fmt(0, lineFormat.c_str(), kv.first.c_str(), kv.second, ""));
            return;
        }

        // 'hedit label <arg>' - articles of this category, otherwise of this label
        list<int> labeledIds;
        for (auto &a: helpManager->getArticles()) {
            DLString category = (*a)->getCategory();
            if (arg2 == "none" ? category.empty() : category == arg2)
                labeledIds.push_back(a->getID());
        }

        bool byCategory = !labeledIds.empty();
        if (!byCategory)
            for (auto &a: helpManager->getArticles())
                if ((*a)->labels.all.count(arg2) != 0)
                    labeledIds.push_back(a->getID());

        if (labeledIds.empty()) {
            ch->pecho("Не найдено ни одной статьи с этой меткой, укажите название полностью.");
            return;
        }

        ch->pecho("Список всех статей %s {c%s{x:", byCategory ? "в категории" : "с меткой", arg2.c_str());
        const DLString lineFormat = web_cmd(ch, "hedit $1", "%4d") + "     %s";
        for (auto &id: labeledIds) {
            HelpArticle::Pointer a = helpManager->getArticle(id);
            ch->pecho(lineFormat.c_str(), a->getID(), a->getAllKeywordsString().c_str());
        }
        return;

    } else if (arg_is_strict(args, "create")) {
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

