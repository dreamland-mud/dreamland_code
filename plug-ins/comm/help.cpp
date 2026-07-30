#include <map>
#include <string.h>
#include "pcharacter.h"
#include "player_utils.h"
#include "helpmanager.h"
#include "act.h"
#include "string_utils.h"
#include "levenshtein.h"
#include "arg_utils.h"
#include "dl_strings.h"
#include "commandtemplate.h"
#include "comm.h"
#include "merc.h"
#include "bugtracker.h"
#include "screenreader.h"
#include "helpcategory.h"
#include "l10n.h"

/*---------------------------------------------------------------------------*
 * Help
 *---------------------------------------------------------------------------*/

/**
 * An attempt to implement fuzzy search, matching input against keywords using
 * Levenshtein algorithm.
 */
struct FuzzySearch {
    FuzzySearch(Character *ch, const char *argument) 
    {
        arg = argument;
        arg.toLower();

        // For short user input, only look for very exact matches (distance 1).
        min_distance = arg.length() > 5 ? 4 : arg.length() > 2 ? 2 : 1;

        candidates.resize(min_distance + 1);

        empty = true;

        // Collect all matching articles.
        for (auto &a : helpManager->getArticles()) {
            if ((*a)->visible(ch))
                searchArticle(a);
        }
    }

    bool hasResults() 
    {
        return !empty;
    }

    void printResults(Character *ch) 
    {
        ostringstream buf;
        int max_output = 5; 
        int firstId = -1;   

        buf << l(ch, "Справка не найдена. Возможно, имелось в виду:") << endl;

        // Output matches starting with best distance ones, but no more than max_output.
        for (int i = 1; i <= min_distance && max_output > 0; i++) {
            auto & matches = candidates[i];

            for (auto &pair: matches) {
                int id = pair.second->getID();
                DLString hint = pair.first;
                DLString title = pair.second->getTitle(DLString::emptyString, Player::displayLang(ch));
                
                // For example:  {hh123beer{x: [{C{hh123{x] Spell 'beer armor'
                buf << fmt(0, "  {hh%d%-20s{x: [{C{hh%5d{x] %s\r\n",
                          id,
                          hint.c_str(),
                          id,
                          title.c_str());

                if (firstId <= 0)
                    firstId = id;

                if ((--max_output) <= 0)
                    break;
            }
        }

        if (firstId > 0)
            buf << endl
                << "Для уточнения поиска смотри справку по нужному номеру, например, "
                << "{y{hcсправка " << firstId << "{x." << endl;        

        ch->send_to(buf);
    }

private:

    void searchArticle(const HelpArticle::Pointer &a) 
    {
        int d;
        StringSet keywords; // contains main keywords and additional ones.
        keywords.insert(a->getAllKeywords().begin(), a->getAllKeywords().end());
        keywords.insert(a->aka.begin(), a->aka.end());

        // See if any of the article's keywords matches the input.
        for (auto &keyword: keywords) {
            DLString kw = keyword;
            kw.replaces("'", "");
            kw.toLower(); 
       
            // First try to match full keyword (with spaces but without quotes). 
            if ((d = getDistance(kw)) <= min_distance) { 
                candidates[d].push_back(make_pair(kw, a));
                empty = false;
                return;
            }

            // If keyword contains spaces, split it into words and try again.
            if (kw != keyword) {
                DLString word;
                while (!(word = kw.getOneArgument()).empty()) {
                    if ((d = getDistance(word)) <= min_distance) { 
                        candidates[d].push_back(make_pair(word, a));
                        empty = false;
                        return;
                    }
                }
            }
        }
    }

    int getDistance(const DLString &keyword)
    {
        // Return Levenshtein distance between user input and the keyword. 
        DLString kw = keyword;

        int lv = levenshtein(arg, kw, 1, 2, 1, 1);
        return lv;
    }

    DLString arg;

    // Keep a list of matches for each distance. A match (pair) contains the exact keyword and the article.
    vector<
        list<pair<DLString, HelpArticle::Pointer> > > candidates;

    // Cut-off distance.
    int min_distance;

    bool empty;
};

struct HelpFinder {
    typedef vector<HelpArticle::Pointer> ArticleArray;

    HelpFinder(Character *ch, const char *argument) {

        parseArgs(argument);

        // Find help by ID.
        Integer id;
        if (Integer::tryParse(id, args)) {
            HelpArticle::Pointer exact = helpManager->getArticle(id);
            
            if (exact)
                articles.push_back(exact);
            return;
        }

        // Find help by keyword.
        findMatchingArticles(ch);

        // Our smartassery yielded nothing, just search for the whole argument.
        if (articles.empty() && !preferredLabels.empty()) {
            preferredLabels.clear();
            findMatchingArticles(ch);
        }
    }
    
    HelpArticle::Pointer get(int number) const
    {    
        unsigned int n = (unsigned int)number;
        if (n > articles.size() || n < 0)
            return HelpArticle::Pointer();
        return articles.at(number-1);
    }
    
    const ArticleArray &getArticles() const {
        return articles;
    }
    
private:
    void findMatchingArticles(Character *ch) 
    {
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if (!(*a)->visible( ch ))
                continue;

            if (!articleMatches(*a))
                continue;
            
            articles.push_back(*a); 
        }
    }

    bool articleMatches(const HelpArticle::Pointer &a) const
    {
        // If first keyword was something like "skill", look for remaining keywords within a certain label.
        if (!preferredLabels.empty() && !a->labels.all.containsAny(preferredLabels))
            return false;

        DLString fullKw = a->getAllKeywordsString() + " " + a->aka.toString();
        fullKw = fullKw.substitute('\'', "");
        const char *lookup = preferredLabels.empty() ? args.c_str() : argRest.c_str();

        if (is_name(lookup, fullKw.c_str()))
            return true; 

        for (StringSet::const_iterator k = (*a)->getAllKeywords().begin(); k != (*a)->getAllKeywords().end(); k++)
            if (is_name(lookup, (*k).c_str()))
                return true; 

        for (auto &aka: (*a)->aka) {
            if (is_name(lookup, aka.c_str()))
                return true;
        }
        return false;
    }

    void parseArgs(const char *argument) {
        args = arg_unquote(argument);
        argRest = args;
        arg1 = argRest.getOneArgument();

        // Reduce "help skill bash" to just "help bash".
        if (!argRest.empty()) {
            if (arg_is(arg1, "skill")) {
                preferredLabels.insert("skill");
                preferredLabels.insert("spell");
            }
            else if (arg_is(arg1, "spell"))
                preferredLabels.insert("spell");
            else if (arg_is(arg1, "class"))
                preferredLabels.insert("class");
            else if (arg_is(arg1, "race"))
                preferredLabels.insert("race");
            else if (arg_is(arg1, "command"))
                preferredLabels.insert("cmd");
            else if (arg_is(arg1, "area"))
                preferredLabels.insert("area");
            else if (arg_is(arg1, "religion"))
                preferredLabels.insert("religion");
            else if (arg_is(arg1, "clan"))
                preferredLabels.insert("clan");
        }
    }
    
    ArticleArray articles;
    DLString args, arg1, argRest;
    StringSet preferredLabels;
};

/*-----------------------------------------------------------------------
 * Category index -- HELP_IA.md
 *
 * 'help index' lists the browsable categories; 'help index <category>' lists
 * that category's articles. The category an article belongs to is resolved by
 * HelpArticle::getCategory(), which the website and the help dump also use, so
 * the three views cannot drift apart.
 *
 * Output is plain lines with no frames or column art: roughly a third of the
 * players read the game through a screen reader, and a table drawn out of
 * punctuation is unreadable to them.
 *-----------------------------------------------------------------------*/
/** The category display names live in helpcategory.cpp, shared with the
 *  'commands' table so the two views group by the same 14 keys. */

/** Deliberately not arg_is(): that matches a one-letter prefix, so 'help i'
 *  would open the index instead of searching, and it logs an error for any
 *  keyword absent from the synonyms table -- which these three are, so every
 *  'help <anything>' would write three error lines. Accept the whole word or a
 *  prefix of at least three characters. */
static bool is_index_keyword(const DLString &arg)
{
    if (arg.size() < 3)
        return false;

    // strPrefix is "this is a prefix of the argument", and it lowercases both.
    static const lang_t langs[3] = { RU, EN, UA };
    for (int l = 0; l < 3; l++)
        if (arg.strPrefix(help_index_keyword(langs[l])))
            return true;

    return false;
}

/** All articles of one category this character may see. */
static void collect_category(Character *ch, const DLString &key,
                             HelpFinder::ArticleArray &result)
{
    HelpArticles::const_iterator a;

    for (a = helpManager->getArticles( ).begin( );
         a != helpManager->getArticles( ).end( ); a++)
    {
        if ((*a)->getID() <= 0 || !(*a)->visible(ch))
            continue;

        if ((*a)->getCategory() == key)
            result.push_back(*a);
    }
}

static void help_index_list(Character *ch, const DLString &cmdName,
                            const DLString &key)
{
    std::basic_ostringstream<char> buf;
    lang_t lang = Player::displayLang(ch);
    HelpFinder::ArticleArray articles;

    collect_category(ch, key, articles);

    buf << fmt(ch, _("{WРаздел '%1$s'{x, статей: %2$d"),
               help_category_name(key, lang).c_str(), (int)articles.size())
        << endl << endl;

    std::multimap<DLString, HelpArticle::Pointer> sorted;
    for (unsigned int i = 0; i < articles.size(); i++)
        sorted.insert(std::make_pair(
            articles[i]->getTitle("toc", lang).colourStrip().toLower(),
            articles[i]));

    for (auto &pair: sorted)
        buf << fmt(0, "  [{C{hh%d{x] %s\r\n",
                   pair.second->getID(),
                   pair.second->getTitle("toc", lang).c_str());

    buf << endl
        << fmt(ch, _("Все разделы: {y{hc%1$s %2$s{x."),
               cmdName.c_str(), help_index_keyword(lang))
        << endl;

    page_to_char(buf.str().c_str(), ch);
}

static void help_index_summary(Character *ch, const DLString &cmdName)
{
    std::basic_ostringstream<char> buf;
    lang_t lang = Player::displayLang(ch);
    const char *indexWord = help_index_keyword(lang);

    buf << fmt(ch, _("{WРазделы справки.{x Выбери раздел, чтобы увидеть его статьи:"))
        << endl << endl;

    for (auto &key: HelpArticle::playerCategories()) {
        HelpFinder::ArticleArray articles;
        collect_category(ch, key, articles);

        if (articles.empty())
            continue;

        DLString name = help_category_name(key, lang);

        // The whole label is the link, because a {hc} link sends exactly the
        // text it shows -- so the text has to be a command the player's own
        // language resolves.
        buf << fmt(0, "  {y{hc%s %s %s{x{D  --  %d{x\r\n",
                   cmdName.c_str(), indexWord, name.c_str(),
                   (int)articles.size());
    }

    page_to_char(buf.str().c_str(), ch);
}

CMDRUNP( help )
{
    std::basic_ostringstream<char> buf;
    DLString origArgument = arg_unquote(argument);

    if (!ch->getPC())
        return;

    // 'help index' / 'справка разделы' / 'довідка розділи', optionally with a
    // category name after it. Handled before the article search so that a
    // category named like an article keyword cannot shadow the index.
    {
        DLString rest = origArgument;
        DLString first = rest.getOneArgument();

        if (!first.empty() && is_index_keyword(first)) {
            DLString cmdName = getNameFor(Player::displayLang(ch));

            if (rest.empty()) {
                help_index_summary(ch, cmdName);
                return;
            }

            DLString key = help_category_from_argument(rest);
            if (!key.empty()) {
                help_index_list(ch, cmdName, key);
                return;
            }

            // Formatted first and sent as a whole: the message embeds a {hc}
            // link built from the command name, and handing that to pecho as a
            // format string would make it reinterpret any % inside it.
            ostringstream out;
            out << fmt(ch, _("Нет такого раздела справки. Все разделы: {y{hc%1$s %2$s{x."),
                       cmdName.c_str(),
                       help_index_keyword(Player::displayLang(ch)))
                << endl;
            ch->send_to(out);
            return;
        }
    }

    if (origArgument.empty()) {
        strcpy(argument, "summary");
    }

    // Вариант 2.create? - needs exact match.
    if (origArgument.size() > 1 && strchr( argument , '.')) {
        char argall[MAX_INPUT_LENGTH];
        int number = number_argument(argument, argall);

        if (number >= 1) {
            HelpArticle::Pointer help = HelpFinder(ch, argall).get(number);
            if (help) {
                page_to_char( help->getText( ch ).c_str( ), ch );
                return;
            }
            ch->pecho(_("Нет подсказки по данному слову."));
            bugTracker->reportNohelp( ch->getPC(), origArgument.c_str( ) );
            return;
        }

        // Restore original argument without the dot, assume it was a typo.
        strcpy(argument, origArgument.substitute('.', ' ').c_str());
    }
    
    // Поиск по строке без чисел.
    HelpFinder::ArticleArray articles = HelpFinder(ch, argument).getArticles();
    // No match, try fuzzy matching.
    if (articles.empty()) {
        if (origArgument.size() > 1) {
            FuzzySearch fs(ch, argument);
            if (fs.hasResults()) {
                fs.printResults(ch);
                return;
            }
        }
        
        ch->pecho(_("Нет подсказки по данному слову."));
        bugTracker->reportNohelp( ch->getPC(), origArgument.c_str( ) );
        return;
    }

    // Exact match - bingo.
    if (articles.size() == 1) {
        page_to_char( articles.front()->getText( ch ).c_str( ), ch );
        return;
    }

    // Several matches, display them all with numbers.
    buf << fmt(ch, _("По запросу '{C%1$s{x' найдено несколько разделов справки с такими номерами:"), origArgument.c_str()) << endl << endl;
    DLString lineFormat = "[{C{hh%5d{x] %s\r\n";
    int firstId = -1;
    for (unsigned int a = 0; a < articles.size(); a++) {
        auto help = articles[a];
        DLString title = help->getTitle(DLString::emptyString, Player::displayLang(ch));

        // Create a line with help ID and title 
        DLString line = title;

        if (firstId == -1)
            firstId = help->getID();

        buf << fmt(0, lineFormat.c_str(), help->getID(), line.c_str());
    }

    buf << endl
        << fmt(ch, _("Для уточнения поиска смотри справку по нужному номеру, например, {y{hcсправка %1$d{x."), firstId) << endl;

    page_to_char(buf.str().c_str(), ch);
}                  


