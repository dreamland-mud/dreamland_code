#include <string.h>
#include "pcharacter.h"
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
        min_distance = arg.length() > 5 ? 3 : arg.length() > 2 ? 2 : 1;

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

        buf << "Справка не найдена. Возможно, имелось в виду:" << endl;

        // Output matches starting with best distance ones, but no more than max_output.
        for (int i = 1; i <= min_distance && max_output > 0; i++) {
            auto & matches = candidates[i];

            for (auto &pair: matches) {
                int id = pair.second->getID();
                DLString hint = pair.first;
                DLString title = pair.second->getTitle(DLString::emptyString);
                
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
        // The keyword is cut to match the input size (unless input is too short already),
        // this allows for prefix matches.
        DLString kw = keyword;

        if (arg.length() > 3 && arg.length() < kw.length())
            String::truncate(kw, arg.length());

        return levenshtein(arg, kw, 1, 1, 1, 1);
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

CMDRUNP( help )
{
    std::basic_ostringstream<char> buf;
    DLString origArgument = arg_unquote(argument);

    if (!ch->getPC())
        return;

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
            ch->pecho("Нет подсказки по данному слову.");
            bugTracker->reportNohelp( ch, origArgument.c_str( ) );
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
        
        ch->pecho("Нет подсказки по данному слову.");
        bugTracker->reportNohelp( ch, origArgument.c_str( ) );
        return;
    }

    // Exact match - bingo.
    if (articles.size() == 1) {
        page_to_char( articles.front()->getText( ch ).c_str( ), ch );
        return;
    }

    // Several matches, display them all with numbers.
    buf << "По запросу '{C" << origArgument << "{x' найдено несколько разделов справки с такими номерами:" << endl << endl;
    DLString lineFormat = "[{C{hh%5d{x] %s\r\n";
    int firstId = -1;
    for (unsigned int a = 0; a < articles.size(); a++) {
        auto help = articles[a];
        DLString title = help->getTitle(DLString::emptyString);

        // Create a line with help ID and title 
        DLString line = title;

        if (firstId == -1)
            firstId = help->getID();

        buf << fmt(0, lineFormat.c_str(), help->getID(), line.c_str());
    }

    buf << endl
        << "Для уточнения поиска смотри справку по нужному номеру, например, "
        << "{y{hcсправка " << firstId << "{x." << endl;        

    page_to_char(buf.str().c_str(), ch);
}                  


