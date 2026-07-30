/* $Id: showtable.cpp,v 1.1.2.9 2009/09/11 11:24:54 rufina Exp $
 *
 * ruffina, 2004
 */
/*
 *
 * sturm, 2003
 */
#include <iomanip>
#include <map>
#include "commandflags.h"
#include "commandtemplate.h"
#include "commandmanager.h"
#include "helpmanager.h"
#include "helpcategory.h"
#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "loadsave.h"
#include "player_utils.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"

/*
 * 'commands'
 */

// Output all command aliases and command names in all languages except for 'lang'
static DLString show_aliases(Command::Pointer &cmd, lang_t lang)
{
    StringSet aliases;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t l = (lang_t)i;

        if (l != lang)
            aliases.insert(cmd->name.get(l));

        for (auto &alias: cmd->aliases.get(l).split(" "))
            aliases.insert(alias);
    }

    return aliases.toString();
}

static void show_matched_commands( Character *ch, const DLString &arg )
{
    ostringstream buf;
    bool found = false;
    lang_t lang = Player::displayLang(ch);

    if (arg.empty( )) {
        ch->pecho(_("Использование: {yкоманды показ{D название{x."));
        return;
    }

    buf << fmt(ch, _("Найдены такие команды:")) << endl << endl;

    for (auto &c: commandManager->getCommands()) {
        ostringstream aliases;
        Command::Pointer cmd = *c;

        if (cmd->getLevel( ) >= LEVEL_HERO && !ch->is_immortal())
            continue;

        if (!cmd->matches( arg ))
            continue;

        found = true;

        // Header: name, hint in player's target lang
        buf << fmt(ch, _("Команда {c%1$s{x: %2$s"),
                       cmd->getNameFor(lang).c_str(), cmd->hint.getForLang(lang).c_str()) << endl;

        // Output names and aliases in all languages
        buf << fmt(ch, _("Синонимы: {D%1$s{x"), show_aliases(cmd, lang).c_str()) << endl;

        // Category, position -- flag words come from the (now externalized) tables
        DLString cat = cmd->getCommandCategory().messages(false, '1', lang).toLower();
        if (cat.empty())
            cat = _("(нет)").getMessage(ch);
        buf << fmt(ch, _("Категория {W%1$s{x"), cat.c_str());

        bitstring_t extra = cmd->getExtra();
        REMOVE_BIT(extra, CMD_HIDDEN|CMD_NO_INTERPRET);
        DLString posWord;
        switch (cmd->getPosition().getValue()) {
            default: posWord = _("всегда").getMessage(ch); break;
            case POS_STANDING: posWord = _("только стоя и вне боя").getMessage(ch); break;
            case POS_FIGHTING: posWord = _("сражаясь").getMessage(ch); break;
            case POS_SITTING: posWord = _("сидя").getMessage(ch); break;
            case POS_RESTING: posWord = _("на отдыхе").getMessage(ch); break;
            case POS_SLEEPING: posWord = _("во сне").getMessage(ch); break;
        }
        buf << fmt(ch, _(", можно выполнить {W%1$s.{x"), posWord.c_str()) << endl;

        // Command flags and order flags
        DLString flagMsg = extra > 0 ? command_flags.messages(extra, true, '1', lang)
                                     : _("без особенностей").getMessage(ch);
        buf << fmt(ch, _("Эта команда {W%1$s{x"), flagMsg.c_str());
        if (cmd->getOrder().getValue() != 0)
            buf << fmt(ch, _(", приказы примут {W%1$s{x"),
                           cmd->getOrder().messages(true, '1', lang).c_str());

        buf << "." << endl << endl;
    }

    if (found)
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->pecho(_("Не найдено ни одной команды, начинающейся с '%s'."), arg.c_str( ));
}

/*-----------------------------------------------------------------------
 * The bare 'commands' table -- HELP_IA.md
 *
 * Grouped by the 14 browsable help categories, the same ones 'help index' and
 * the website use, and resolved through HelpArticle::getCategory() so the three
 * views cannot drift apart. The old 27 'cat' flags survive as a search facet and
 * as what 'commands show' reports; they are no longer a player-facing taxonomy.
 *
 * Every entry is a link to the article that explains it, which is also the rule
 * for what gets listed: this table is a newbie's front door, and a word it
 * cannot explain does not belong on it. 'commands list' stays exhaustive.
 *-----------------------------------------------------------------------*/
// Raw pointers rather than HelpArticle::Pointer: the manager owns the articles
// and outlives every call here, and its list hands out const elements, which a
// reference-counted Pointer refuses to take.
typedef map<DLString, const HelpArticle *> ArticleByKeyword;

/** Index every article this character may read by its keywords. Articles that
 *  describe a command win a collision: 'search' is both a command and a racial
 *  skill, and on this table the command's own article is the honest answer. */
static void index_articles(Character *ch, ArticleByKeyword &index)
{
    static const DLString LABEL_COMMAND = "cmd";
    HelpArticles::const_iterator a;

    for (a = helpManager->getArticles( ).begin( );
         a != helpManager->getArticles( ).end( ); a++)
    {
        if ((*a)->getID() <= 0 || !(*a)->visible(ch))
            continue;

        bool isCommand = (*a)->labels.all.count(LABEL_COMMAND) > 0;

        for (auto &keyword: (*a)->getAllKeywords()) {
            DLString key = keyword;
            key.toLower();

            ArticleByKeyword::iterator i = index.find(key);
            if (i == index.end())
                index[key] = (*a).getPointer();
            else if (isCommand && i->second->labels.all.count(LABEL_COMMAND) == 0)
                i->second = (*a).getPointer();
        }
    }

    // A.K.A. words are a weaker signal than a real keyword, so they only fill
    // gaps. They are how the input operators '!', '\' and '|' are found at all:
    // no command owns them, but article 17 answers to them.
    for (a = helpManager->getArticles( ).begin( );
         a != helpManager->getArticles( ).end( ); a++)
    {
        if ((*a)->getID() <= 0 || !(*a)->visible(ch))
            continue;

        for (auto &keyword: (*a)->aka) {
            DLString key = keyword;
            key.toLower();

            if (index.count(key) == 0)
                index[key] = (*a).getPointer();
        }
    }
}

static const HelpArticle * article_by_name(const ArticleByKeyword &index, const DLString &name)
{
    if (name.empty())
        return 0;

    DLString key = name;
    key.toLower();

    ArticleByKeyword::const_iterator i = index.find(key);
    if (i == index.end())
        return 0;

    return i->second;
}

/** The article that actually explains this command to this viewer, NULL when
 *  there is none. */
static const HelpArticle * command_article(Character *ch, Command::Pointer &cmd,
                                           const ArticleByKeyword &index)
{
    CommandHelp::Pointer help = cmd->getHelp( );

    if (help && help->visible(ch))
        return help.getPointer();

    // An empty stub with 'refby' hands the reader over to the article covering
    // the whole family: 'east' to 'north', 'sit' to 'sleep', 'drop' to 'get'.
    if (help) {
        CommandHelp::Pointer umbrella = help->getReferencedBy( );

        if (umbrella && umbrella->visible(ch))
            return umbrella.getPointer();
    }

    // Skill commands keep their article on the skill and not on the command,
    // and a few stubs have neither a body nor a 'refby' yet are still a keyword
    // of the article that covers them. Both answer to '? <name>', so ask the
    // same way the player would. The viewer's own name first: auto-keywords
    // carry the English and Russian names, Ukrainian only where the article
    // spells it out, so the last two are the fallback and not the other way.
    const HelpArticle *article = article_by_name(index, cmd->getNameFor(Player::displayLang(ch)));

    if (!article)
        article = article_by_name(index, cmd->getName( ));

    if (!article)
        article = article_by_name(index, cmd->getRussianName( ));

    return article;
}

/** '?', '!', '\' and '|' are input operators rather than commands, so no
 *  Command object carries them, but they are the first thing a newbie needs. */
static const char * INPUT_OPERATORS[] = { "?", "!", "\\", "|", NULL };

static void show_commands_by_categories( Character *ch )
{
    ostringstream buf;
    lang_t lang = Player::displayLang(ch);
    ArticleByKeyword index;
    map<DLString, StringList> grouped;

    index_articles(ch, index);

    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;

        if (!cmd->visible( ch ))
            continue;

        if (cmd->getLevel( ) >= LEVEL_HERO)
            continue;

        const HelpArticle *article = command_article(ch, cmd, index);
        if (!article)
            continue;

        DLString category = article->getCategory();
        if (!HelpArticle::isPlayerCategory(category))
            continue;

        grouped[category].push_back(
            fmt(0, "{hh%d%s{hx", article->getID(), cmd->getNameFor(lang).c_str()));
    }

    for (int i = 0; INPUT_OPERATORS[i]; i++) {
        const HelpArticle *article = article_by_name(index, INPUT_OPERATORS[i]);
        if (!article)
            continue;

        DLString category = article->getCategory();
        if (!HelpArticle::isPlayerCategory(category))
            continue;

        grouped[category].push_back(
            fmt(0, "{hh%d%s{hx", article->getID(), INPUT_OPERATORS[i]));
    }

    buf << fmt(ch, _("{WКоманды{x по разделам справки:"))
        << endl << endl;

    // Streamed rather than formatted: a category's worth of commands with their
    // link markup runs well past what a format buffer is happy to hold.
    for (auto &key: HelpArticle::playerCategories()) {
        map<DLString, StringList>::const_iterator g = grouped.find(key);

        if (g == grouped.end() || g->second.empty())
            continue;

        buf << "{c" << help_category_name(key, lang) << ":{x "
            << g->second.join(" ") << endl;
    }

    buf << endl
        << fmt(ch, _("Полный список с описаниями: {y{hcкоманды список{x. "
                     "Все разделы справки: {y{hcпомощь разделы{x."))
        << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}

static void show_commands_list( Character *ch )
{
    ostringstream buf;
    lang_t lang = Player::displayLang(ch);

    // Column labels localized; the %-Ns padding realigns them to the data columns
    // regardless of the word's length.
    buf << fmt( ch, "%-12s | %-45s| %s",
                _("Название").getMessage(ch).c_str(),
                _("Справка").getMessage(ch).c_str(),
                _("Синонимы").getMessage(ch).c_str())
        << endl
        << "-------------+----------------------------------------------+---------------"
        << endl;

    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;
        
        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) >= LEVEL_HERO)
            continue;

        DLString name = cmd->getNameFor(lang);
        DLString hint = cmd->hint.getForLang(lang);
        DLString aliases = show_aliases(cmd, lang);
                
        buf << fmt( 0, "{c%-12s {x: %-45s: %s",
                        name.c_str(),
                        hint.c_str(),
                        aliases.c_str() )
            << endl;
    }

    buf << endl
        << fmt(ch, _("Таблица по разделам: {y{hcкоманды{x. "
                     "Подробно об одной команде: {yкоманды показать {Dслово{x."))
        << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}

CMDRUN( commands )
{
    DLString arg, args = constArguments; 
    
    arg = args.getOneArgument( );
    
    if (arg_is_show(arg)) {
        show_matched_commands( ch, args );
        return;
    }

    if (arg.empty( )) {
        show_commands_by_categories(ch);
        return;
    }
 
    if (arg_is_list(arg)) {
        show_commands_list(ch);
        return;
    }

    ch->pecho(_("Использование:\n"
    "{y{hcкоманды{x        - таблица всех команд\n"
    "{y{hcкоманды список{x - список команд с краткой справкой\n"
    "{yкоманды показ{x слово - показать синонимы и справку по команде.\n"));
}

/*
 * 'wizhelp'
 */
CMDRUN( wizhelp )
{
    ostringstream buf;

    // TODO rework when most wizhelp commands have UA, RU aliases
    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;

        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) <  LEVEL_HERO)
            continue;

        buf << fmt( 0, "{c%-12s {x: %-45s\n",
                        cmd->getName( ).c_str( ),
                        cmd->getHint( ).c_str( ));        
    }

    page_to_char( buf.str( ).c_str( ), ch );
}
