#include <map>
#include <set>
#include <vector>
#include <algorithm>
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
#include "translit.h"
#include "l10n.h"

/*---------------------------------------------------------------------------*
 * Help
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Fuzzy search -- docs/plans/help_revamp.md, Phase 4a.
 *
 * Runs only when HelpFinder found nothing. Stage order and every constant are
 * mirrored in dreamland_tools/help-search-replay.py, which replays the logged
 * misses (misc/nohelp.txt) against the help dump: tune there first, then
 * here, or the replay stops describing the game.
 *---------------------------------------------------------------------------*/
namespace {

const int WORD_EXACT = 100;     // query word equals a keyword word
const int WORD_PREFIX = 90;     // query word of PREFIX_MIN+ letters starts a keyword word
const int WORD_FUZZY = 80;      // minus FUZZY_STEP per edit
const int WORD_STEM = 70;       // long common prefix (inflection), minus the unshared tail
const int FUZZY_STEP = 15;
const int DF_BONUS = 50;        // divided by the number of articles the word matched: rare words win
const int MULTI_MIN_WORDS = 2;  // union results need this many matched words when the query has 3+
const int MAX_RESULTS = 5;
const size_t PREFIX_MIN = 3;
const size_t STEM_MIN_PREFIX = 5;
const size_t STEM_ABBR_PREFIX = 4;
const size_t FUZZY_MIN_LEN = 5;   // shorter query words get no edits at all (шлем/шлеп, моба/мова)
const size_t FLIP_MIN_LEN = 3;    // every word this long before a layout flip is tried (уф -> ea)
const size_t NUMBER_MAX_DIGITS = 6;

// Number row and numpad neighbours of each digit, for one-key typos in an id.
const char *DIGIT_NEIGHBOURS[10] = {
    "91", "0247", "1356", "2456", "3571", "46289", "57390", "6841", "7952", "0863"
};

int fuzzy_cutoff(size_t n)
{
    if (n < FUZZY_MIN_LEN)
        return 0;
    if (n <= 5)
        return 1;
    if (n <= 8)
        return 2;
    return 3;
}

size_t common_prefix(const DLString &a, const DLString &b)
{
    size_t n = 0, limit = min(a.size(), b.size());

    while (n < limit && a[n] == b[n])
        n++;

    return n;
}

/** Common prefix of query word q and keyword word k when it counts as the same
 *  stem, else 0. A shorter prefix passes only when q is an abbreviation of k with
 *  at most one trailing letter wrong (invey -> inventory): a long query word
 *  sharing four letters with a short keyword (переулок/перед) is noise. */
size_t stem_len(const DLString &q, const DLString &k)
{
    size_t p = common_prefix(q, k);

    if (p >= STEM_MIN_PREFIX)
        return p;
    if (p >= STEM_ABBR_PREFIX && q.size() <= p + 1 && k.size() >= q.size())
        return p;
    return 0;
}

/** Score of one lowercase query word against one lowercase keyword word, 0 for no match. */
int word_score(const DLString &q, const DLString &k)
{
    if (q == k)
        return WORD_EXACT;

    if (q.size() >= PREFIX_MIN && k.size() > q.size() && k.compare(0, q.size(), q) == 0)
        return WORD_PREFIX;

    int best = 0;
    int cut = fuzzy_cutoff(q.size());

    if (cut > 0) {
        int diff = (int)q.size() - (int)k.size();
        if (diff < 0)
            diff = -diff;
        if (diff <= cut) {
            int d = levenshtein(q, k, 1, 1, 1, 1);
            if (d <= cut)
                best = WORD_FUZZY - FUZZY_STEP * d;
        }
    }

    size_t p = stem_len(q, k);
    if (p > 0) {
        int sc = WORD_STEM - (int)(max(q.size(), k.size()) - p);
        if (sc > best)
            best = sc;
    }

    return best;
}

/** Append the lowercase words of a query or of one keyword, quotes dropped. */
void split_words(const DLString &text, vector<DLString> &words)
{
    DLString rest = text;
    DLString word;

    rest.replaces("'", "");
    rest.replaces("\"", "");
    rest.toLower();

    while (!(word = rest.getOneArgument()).empty())
        words.push_back(word);
}

/** A {hh link whose visible text is the title: {hh followed by the digits alone
 *  renders an empty label, and a screen reader needs the words, not a number. */
DLString help_link(int id, const DLString &title)
{
    DLString label = title.colourStrip();

    // The tag parser swallows every digit after the id, so a title that
    // starts with one needs a separator.
    if (!label.empty() && isdigit((unsigned char)label.at(0)))
        label = " " + label;

    return fmt(0, "{hh%d%s{x", id, label.c_str());
}

} // namespace

struct HelpFinder {
    typedef vector<HelpArticle::Pointer> ArticleArray;

    HelpFinder(Character *ch, const char *argument) {

        parseArgs(argument);

        // Find help by ID.
        Integer id;
        if (Integer::tryParse(id, args)) {
            HelpArticle::Pointer exact = helpManager->getArticle(id);
            
            if (exact) {
                HelpArticle::Pointer target = exact->getAliasTarget();
                if (target)
                    exact = target;
                articles.push_back(exact);
            }
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
            // An alias answers to its own keywords but shows its target.
            HelpArticle::Pointer self(*a);
            HelpArticle::Pointer shown = self->getAliasTarget();
            if (!shown)
                shown = self;

            if (!shown->visible( ch ))
                continue;

            if (!articleMatches(*a))
                continue;

            bool seen = false;
            for (auto &x: articles)
                if (x.getPointer() == shown.getPointer())
                    seen = true;
            if (seen)
                continue;
            
            articles.push_back(shown); 
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

/**
 * Suggestions for a query HelpFinder could not match: the query typed in the
 * wrong keyboard layout, per-word fuzzy matching against keyword words, or for
 * a numeric id the registered ids one typo away. One pass over the articles,
 * only on a miss.
 */
struct FuzzySearch {
    enum Kind { TEXT, FLIP, NUMBER };

    struct Hit {
        HelpArticle::Pointer article;
        DLString hint;      // what matched: a keyword word, or the flipped query
        Kind kind;
    };

    FuzzySearch(Character *ch, const char *argument)
    {
        DLString query = arg_unquote(argument);
        query.stripWhiteSpace();

        Integer id;
        if (Integer::tryParse(id, query)) {
            searchNumber(ch, id);
            return;
        }

        split_words(query, words);
        if (words.empty())
            return;

        if (!searchFlipped(ch, query))
            searchWords(ch);
    }

    bool hasResults() const
    {
        return !hits.empty();
    }

    void printResults(Character *ch) const
    {
        ostringstream buf;
        lang_t lang = Player::displayLang(ch);
        int firstId = -1;

        buf << l(ch, "Справка не найдена. Возможно, имелось в виду:") << endl;

        // One line per hit, the title as the link text, no columns: a screen
        // reader reads it top to bottom and every line stands on its own.
        for (auto &hit: hits) {
            int id = hit.article->getID();
            DLString title = hit.article->getTitle(DLString::emptyString, lang);
            DLString hint = hintFor(hit, lang, title);

            buf << fmt(0, "  [{C%d{x] %s", id, help_link(id, title).c_str());
            if (!hint.empty())
                buf << " (" << hint << ")";
            buf << "\r\n";

            if (firstId <= 0)
                firstId = id;
        }

        if (firstId > 0)
            buf << endl
                << fmt(ch, _("Для уточнения поиска смотри справку по нужному номеру, например, {y{hcсправка %1$d{x."), firstId) << endl;

        ch->send_to(buf);
    }

private:
    /** The matched word, when it tells the viewer something the title does not:
     *  dropped for a fuzzy hit in the other script (a Cyrillic keyword for an
     *  English reader and vice versa), kept for a layout flip, where the flipped
     *  text is the whole point. */
    static DLString hintFor(const Hit &hit, lang_t lang, const DLString &title)
    {
        if (hit.hint.empty() || hit.kind == NUMBER)
            return DLString::emptyString;

        if (hit.kind == TEXT) {
            bool cyr = String::hasCyrillic(hit.hint);

            if ((lang == EN) == cyr)
                return DLString::emptyString;
            if (title.toLower().find(hit.hint) != DLString::npos)
                return DLString::emptyString;
        }

        return hit.hint;
    }

    /** The query typed in the wrong keyboard layout: flip it and run the plain
     *  prefix match again. Two-letter words flip into a prefix of something
     *  every time (уф -> ea), so short queries skip this. */
    bool searchFlipped(Character *ch, const DLString &query)
    {
        for (auto &w: words)
            if (w.size() < FLIP_MIN_LEN)
                return false;

        static const lang_t langs[2] = { RU, UA };
        DLString tried;

        for (int i = 0; i < 2; i++) {
            DLString flipped = translit(langs[i], query);

            if (flipped == query || flipped == tried)
                continue;
            tried = flipped;

            HelpFinder finder(ch, flipped.c_str());
            const HelpFinder::ArticleArray &found = finder.getArticles();

            for (size_t j = 0; j < found.size() && (int)hits.size() < MAX_RESULTS; j++)
                hits.push_back(Hit{found[j], flipped, FLIP});

            if (!hits.empty())
                return true;
        }

        return false;
    }

    /** Per-word scoring against every keyword word of every visible article.
     *  Articles matching every query word win; otherwise the ones matching the
     *  most words, a rare matched word outranking a common one. */
    void searchWords(Character *ch)
    {
        struct Candidate {
            HelpArticle::Pointer article;
            vector<int> best;
            vector<DLString> hint;
        };
        vector<Candidate> cands;
        map<HelpArticle *, size_t> index;
        vector<DLString> kwords;
        size_t n = words.size();

        for (auto &a: helpManager->getArticles()) {
            // An alias answers to its own keywords but shows its target.
            HelpArticle::Pointer self(a);
            HelpArticle::Pointer shown = self->getAliasTarget();
            if (!shown)
                shown = self;
            if (!shown->visible(ch))
                continue;

            kwords.clear();
            for (auto &kw: self->getAllKeywords())
                split_words(kw, kwords);
            for (auto &kw: self->aka)
                split_words(kw, kwords);

            vector<int> best(n, 0);
            vector<DLString> hint(n);
            bool any = false;

            for (size_t i = 0; i < n; i++) {
                for (auto &k: kwords) {
                    int sc = word_score(words[i], k);
                    if (sc > best[i]) {
                        best[i] = sc;
                        hint[i] = k;
                        if (sc == WORD_EXACT)
                            break;
                    }
                }
                if (best[i] > 0)
                    any = true;
            }

            if (!any)
                continue;

            auto found = index.find(shown.getPointer());
            if (found == index.end()) {
                index[shown.getPointer()] = cands.size();
                cands.push_back(Candidate{shown, best, hint});
            } else {
                Candidate &c = cands[found->second];
                for (size_t i = 0; i < n; i++)
                    if (best[i] > c.best[i]) {
                        c.best[i] = best[i];
                        c.hint[i] = hint[i];
                    }
            }
        }

        if (cands.empty())
            return;

        // How many articles each query word matched: a word matching one
        // article says more than a word matching twenty.
        vector<int> df(n, 0);
        for (auto &c: cands)
            for (size_t i = 0; i < n; i++)
                if (c.best[i] > 0)
                    df[i]++;

        struct Ranked {
            int matched;
            int score;
            size_t cand;
            size_t hintWord;
        };
        vector<Ranked> ranked;
        bool full = false;

        for (size_t ci = 0; ci < cands.size(); ci++) {
            Ranked r = { 0, 0, ci, 0 };
            int bestWord = -1;

            for (size_t i = 0; i < n; i++) {
                if (cands[ci].best[i] <= 0)
                    continue;

                int sc = cands[ci].best[i] + DF_BONUS / df[i];
                r.matched++;
                r.score += sc;
                if (sc > bestWord) {
                    bestWord = sc;
                    r.hintWord = i;
                }
            }

            if (r.matched == (int)n)
                full = true;
            ranked.push_back(r);
        }

        int need = full ? (int)n : (n >= 3 ? MULTI_MIN_WORDS : 1);

        sort(ranked.begin(), ranked.end(), [&](const Ranked &x, const Ranked &y) {
            if (x.matched != y.matched)
                return x.matched > y.matched;
            if (x.score != y.score)
                return x.score > y.score;
            return cands[x.cand].article->getID() < cands[y.cand].article->getID();
        });

        for (auto &r: ranked) {
            if (r.matched < need || (int)hits.size() >= MAX_RESULTS)
                break;
            hits.push_back(Hit{cands[r.cand].article, cands[r.cand].hint[r.hintWord], TEXT});
        }
    }

    /** No article with this id: the registered, visible ids a typo away.
     *  Neighbours first (the player is walking the index by number), then one
     *  digit hit on an adjacent key, any other digit, a digit dropped or added,
     *  two adjacent digits swapped; ties by distance from the typed number. */
    void searchNumber(Character *ch, int number)
    {
        DLString digits(number);

        if (number <= 0 || digits.size() > NUMBER_MAX_DIGITS)
            return;

        struct Numbered {
            int cls;
            int dist;
            int id;
            HelpArticle::Pointer article;
        };
        vector<Numbered> found;
        set<HelpArticle *> seen;

        auto add = [&](int cls, const DLString &text) {
            int id = atoi(text.c_str());
            if (id <= 0 || id == number)
                return;

            HelpArticle::Pointer a = helpManager->getArticle(id);
            if (!a)
                return;

            HelpArticle::Pointer target = a->getAliasTarget();
            if (target)
                a = target;
            if (!a->visible(ch) || seen.count(a.getPointer()))
                return;

            seen.insert(a.getPointer());
            found.push_back(Numbered{cls, abs(number - id), id, a});
        };

        for (int d = 1; d <= 5; d++) {
            add(0, DLString(number - d));
            add(0, DLString(number + d));
        }

        for (size_t i = 0; i < digits.size(); i++)
            for (const char *r = DIGIT_NEIGHBOURS[digits[i] - '0']; *r; r++)
                add(1, DLString(digits.substr(0, i) + *r + digits.substr(i + 1)));

        for (size_t i = 0; i < digits.size(); i++)
            for (char r = '0'; r <= '9'; r++)
                if (r != digits[i])
                    add(2, DLString(digits.substr(0, i) + r + digits.substr(i + 1)));

        if (digits.size() > 1)
            for (size_t i = 0; i < digits.size(); i++)
                add(3, DLString(digits.substr(0, i) + digits.substr(i + 1)));

        for (size_t i = 0; i <= digits.size(); i++)
            for (char r = '0'; r <= '9'; r++) {
                if (i == 0 && r == '0')
                    continue;
                add(3, DLString(digits.substr(0, i) + r + digits.substr(i)));
            }

        for (size_t i = 0; i + 1 < digits.size(); i++) {
            DLString swapped = digits;
            std::swap(swapped[i], swapped[i + 1]);
            add(4, swapped);
        }

        sort(found.begin(), found.end(), [](const Numbered &x, const Numbered &y) {
            if (x.cls != y.cls)
                return x.cls < y.cls;
            if (x.dist != y.dist)
                return x.dist < y.dist;
            return x.id < y.id;
        });

        for (size_t i = 0; i < found.size() && (int)i < MAX_RESULTS; i++)
            hits.push_back(Hit{found[i].article, DLString::emptyString, NUMBER});
    }

    vector<DLString> words;
    vector<Hit> hits;
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
        buf << fmt(0, "  [{C%d{x] %s\r\n",
                   pair.second->getID(),
                   help_link(pair.second->getID(),
                             pair.second->getTitle("toc", lang)).c_str());

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
            // No such nth match: search the word itself, so the player gets
            // the numbered list to pick from instead of a dead end.
            strcpy(argument, argall);
        }
        else {
            // Restore original argument without the dot, assume it was a typo.
            strcpy(argument, origArgument.substitute('.', ' ').c_str());
        }
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


