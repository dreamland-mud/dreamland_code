#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <string.h>
#include "jsoncpp/json/json.h"
#include "configurable.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "clanmanager.h"
#include "clan.h"
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
#include "def.h"
#include "bugtracker.h"
#include "screenreader.h"
#include "helpcategory.h"
#include "translit.h"
#include "l10n.h"
#include "helpworldindex.h"
#include "areahelp.h"
#include "area.h"
#include "areaquest.h"
#include "commandmanager.h"
#include "xmlattributevisited.h"
#include "fenia_utils.h"

/*---------------------------------------------------------------------------*
 * Help
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Fuzzy search -- docs/plans/help_revamp.md, Phase 4a.
 *
 * Runs only when HelpFinder found nothing. Stage order and every constant are
 * mirrored in dreamland_tools/help-search-replay.py, which replays the logged
 * misses (misc/nohelp.txt) against the help dump: tune there first, then
 * here, or the replay stops describing the game. The same goes for the
 * trailing level facet (help_level_rest), the public achievement fallback
 * (help_ach_answer) and the ё/е folding (help_fold_yo).
 *---------------------------------------------------------------------------*/
namespace {

const int WORD_EXACT = 100;     // query word equals a keyword word
const int WORD_PREFIX = 90;     // query word of PREFIX_MIN+ letters starts a keyword word
const int WORD_FUZZY = 80;      // minus FUZZY_STEP per edit
const int WORD_STEM = 70;       // long common prefix (inflection), minus the unshared tail
const int WORD_WEAK = 1;        // a 4-letter substitution: ranks below every other match
const int FUZZY_STEP = 15;
const int DF_BONUS = 50;        // divided by the number of articles the word matched: rare words win
const int MULTI_MIN_WORDS = 2;  // union results need this many matched words when the query has 3+
const int MAX_RESULTS = 5;
const size_t PREFIX_MIN = 3;
const size_t STEM_MIN_PREFIX = 5;
const size_t STEM_ABBR_PREFIX = 4;
const size_t FUZZY_MIN_LEN = 5;   // shorter query words get no free edit (шлем/шлеп, моба/мова)
const size_t SHORT_TYPO_LEN = 4;  // ... except a swap or a plural letter, and a weak substitution
const size_t FLIP_MIN_LEN = 3;    // every word this long before a layout flip is tried (уф -> ea)
const size_t MAX_WORDS = 8;       // a pasted sentence costs no more than a short question
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

/** Score of one lowercase query word against one lowercase keyword word, 0 for
 *  no match. A `glued` pair of query words (fire ball -> fireball) gets neither
 *  the stem rule, or it would match a keyword that merely starts it (темный
 *  шлем -> темный), nor the weak substitution: that would be a guess on a guess. */
int word_score(const DLString &q, const DLString &k, bool glued = false)
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
    else if (q.size() == SHORT_TYPO_LEN) {
        // Four letters are too few for a free edit, but two neighbours swapped
        // (bsah, kcik) or one letter added to a three-letter word (the plural:
        // шары, мечи) almost always lead to a single keyword word. Weights make
        // a swap the only distance-1 edit. One substituted letter (lokk) is as
        // likely a different word (шлем/шлеп), so it only scores WORD_WEAK.
        if (k.size() == SHORT_TYPO_LEN && levenshtein(q, k, 1, 2, 2, 2) == 1)
            best = WORD_FUZZY - FUZZY_STEP;
        else if (k.size() + 1 == q.size() && q.compare(0, k.size(), k) == 0)
            best = WORD_FUZZY - FUZZY_STEP;
        else if (!glued && k.size() == SHORT_TYPO_LEN && levenshtein(q, k, 1, 1, 1, 1) == 1)
            best = WORD_WEAK;
    }

    size_t p = glued ? 0 : stem_len(q, k);
    if (p > 0) {
        // Never as low as WORD_WEAK, whatever the unshared tail.
        int sc = max(WORD_STEM - (int)(max(q.size(), k.size()) - p), WORD_WEAK + 1);
        if (sc > best)
            best = sc;
    }

    return best;
}

/** The labels a leading facet word restricts the search to ("help skill
 *  bash" looks among skills and spells), or false if the word is no facet. */
bool help_facet_labels(const DLString &word, StringSet &labels)
{
    if (arg_is(word, "skill")) {
        labels.insert("skill");
        labels.insert("spell");
    }
    else if (arg_is(word, "spell"))
        labels.insert("spell");
    else if (arg_is(word, "class"))
        labels.insert("class");
    else if (arg_is(word, "race"))
        labels.insert("race");
    else if (arg_is(word, "command"))
        labels.insert("cmd");
    else if (arg_is(word, "area"))
        labels.insert("area");
    else if (arg_is(word, "religion"))
        labels.insert("religion");
    else if (arg_is(word, "clan"))
        labels.insert("clan");
    else
        return false;

    return true;
}

/** Append the lowercase words of a query or of one keyword, quotes dropped,
 *  ё folded into е. */
void split_words(const DLString &text, vector<DLString> &words)
{
    DLString rest = text;
    DLString word;

    rest.replaces("'", "");
    rest.replaces("\"", "");
    rest.toLower();
    help_fold_yo(rest);

    while (!(word = rest.getOneArgument()).empty())
        words.push_back(word);
}

/** A trailing word asking for ranks: "hunter level", "шалафи ранги". Whole
 *  words only, not arg_is(): that takes any prefix, and "shalafi r" asks
 *  nothing. The word must already be lowercase. */
bool help_level_facet(const DLString &word)
{
    static const char *words[] = {
        "level", "levels", "rank", "ranks",
        "уровень", "уровни", "рівень", "рівні", "ранг", "ранги", 0
    };

    for (int i = 0; words[i]; i++)
        if (word == words[i])
            return true;
    return false;
}

/** The words a trailing level facet asks about, a leading facet word dropped
 *  ("clan shalafi level" -> "shalafi"), or empty when the query is no
 *  "<clan> level" question. "clan level" stays empty: it asks about clan
 *  levels in general, which the common clan article answers. */
DLString help_level_rest(const DLString &query)
{
    vector<DLString> words;
    StringSet labels;

    split_words(query, words);
    if (words.size() < 2 || !help_level_facet(words.back()))
        return DLString::emptyString;

    words.pop_back();
    if (words.size() > 1 && help_facet_labels(words.front(), labels))
        words.erase(words.begin());
    if (words.size() == 1 && help_facet_labels(words.front(), labels))
        return DLString::emptyString;

    DLString rest;
    for (auto &w: words) {
        if (!rest.empty())
            rest += " ";
        rest += w;
    }
    return rest;
}

/** A clan's own article, the one that ends with the clan's ranks. Other
 *  articles labelled "clan" (rules, laws, the tournament) are not. */
bool help_is_clan_article(const HelpArticle::Pointer &a)
{
    return clanManager
        && a->labels.all.find("clan") != a->labels.all.end()
        && clanManager->findByHelpId(a->getID()) != 0;
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

        // "hunter level": the clan's own article, which ends with its ranks.
        // Nothing there means the plain search below, as if no facet was typed.
        if (!clanRest.empty()) {
            findMatchingArticles(ch, true);
            if (!articles.empty())
                return;
        }

        // Find help by keyword.
        findMatchingArticles(ch, false);

        // Our smartassery yielded nothing, just search for the whole argument.
        if (articles.empty() && !preferredLabels.empty()) {
            preferredLabels.clear();
            findMatchingArticles(ch, false);
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
    void findMatchingArticles(Character *ch, bool clansOnly)
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

            if (clansOnly && !help_is_clan_article(shown))
                continue;

            if (!articleMatches(*a, clansOnly))
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

    /** Every word of the lookup is the start of a keyword word, ё and е alike
     *  on both sides. With clansOnly the lookup is the words before a trailing
     *  level facet, and no facet filter applies. */
    bool articleMatches(const HelpArticle::Pointer &a, bool clansOnly) const
    {
        const char *lookup;

        if (clansOnly)
            lookup = clanRest.c_str();
        else {
            // If first keyword was something like "skill", look for remaining keywords within a certain label.
            if (!preferredLabels.empty() && !a->labels.all.containsAny(preferredLabels))
                return false;
            lookup = preferredLabels.empty() ? args.c_str() : argRest.c_str();
        }

        DLString fullKw = a->getAllKeywordsString() + " " + a->aka.toString();
        fullKw = fullKw.substitute('\'', "");
        help_fold_yo(fullKw);

        if (is_name(lookup, fullKw.c_str()))
            return true; 

        for (StringSet::const_iterator k = (*a)->getAllKeywords().begin(); k != (*a)->getAllKeywords().end(); k++)
            if (keywordMatches(lookup, *k))
                return true; 

        for (auto &aka: (*a)->aka) {
            if (keywordMatches(lookup, aka))
                return true;
        }
        return false;
    }

    /** The lookup is folded already, so a keyword with no ё/Ё (KOI8 0xA3/0xB3,
     *  nearly all of them) is compared as it is, without a copy. */
    static bool keywordMatches(const char *lookup, const DLString &kw)
    {
        if (kw.find_first_of("\xA3\xB3") == DLString::npos)
            return is_name(lookup, kw.c_str());

        DLString folded = kw;
        return is_name(lookup, help_fold_yo(folded).c_str());
    }

    void parseArgs(const char *argument) {
        args = arg_unquote(argument);
        help_fold_yo(args);
        argRest = args;
        arg1 = argRest.getOneArgument();

        // Reduce "help skill bash" to just "help bash".
        if (!argRest.empty())
            help_facet_labels(arg1, preferredLabels);

        clanRest = help_level_rest(args);
    }
    
    ArticleArray articles;
    DLString args, arg1, argRest;
    DLString clanRest;      // the words before a trailing level facet
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

    FuzzySearch(Character *ch, const char *argument) : numeric(false), clansOnly(false)
    {
        DLString query = arg_unquote(argument);
        query.stripWhiteSpace();

        Integer id;
        if (Integer::tryParse(id, query)) {
            numeric = true;
            searchNumber(ch, id);
            return;
        }

        // A trailing level facet, as in HelpFinder: the rest among the clans'
        // own articles first, so "batllerager level" still lands. No layout
        // flip here: the whole-query pass below flips the facet word too.
        DLString clanRest = help_level_rest(query);
        if (!clanRest.empty()) {
            StringSet clanLabel;
            clanLabel.insert("clan");
            split_words(clanRest, words);
            if (words.size() > MAX_WORDS)
                words.resize(MAX_WORDS);
            clansOnly = true;
            searchWords(ch, clanLabel);
            clansOnly = false;
            if (!hits.empty())
                return;
        }

        // A leading facet word filters, as in HelpFinder: scored as a word, the
        // frequent "spell" would outrank the one-off typo it comes with. And as
        // in HelpFinder, nothing found that way means the whole query is
        // searched again with no filter: arg_is() takes any prefix, so "re" in
        // "re scue" reads as "religion" and the glued "rescue" needs the retry.
        DLString facet, rest = query;
        StringSet labels;
        facet = rest.getOneArgument();
        if (!rest.empty() && help_facet_labels(facet, labels)
                && search(ch, facet, rest, labels))
            return;

        search(ch, DLString::emptyString, query, StringSet());
    }

    bool hasResults() const
    {
        return !hits.empty();
    }

    /** Suggestions for a mistyped id rather than for words. */
    bool isNumeric() const
    {
        return numeric;
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
    /** One pass: the layout flip, then the per-word scoring. True if anything was found. */
    bool search(Character *ch, const DLString &facet, const DLString &query, const StringSet &labels)
    {
        words.clear();
        split_words(query, words);
        if (words.size() > MAX_WORDS)
            words.resize(MAX_WORDS);
        if (words.empty())
            return false;

        if (searchFlipped(ch, facet, query))
            return true;

        searchWords(ch, labels);
        return !hits.empty();
    }

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
     *  prefix match again, the facet word (already in the right layout) kept in
     *  front. Two-letter words flip into a prefix of something every time
     *  (уф -> ea), so short queries skip this. The viewer's own Cyrillic layout
     *  goes first: the two differ only on the і/ы, ї/ъ, є/э, ґ/ё keys. */
    bool searchFlipped(Character *ch, const DLString &facet, const DLString &query)
    {
        for (auto &w: words)
            if (w.size() < FLIP_MIN_LEN)
                return false;

        lang_t langs[2] = { RU, UA };
        if (Player::displayLang(ch) == UA) {
            langs[0] = UA;
            langs[1] = RU;
        }
        DLString tried;

        for (int i = 0; i < 2; i++) {
            DLString flipped = translit(langs[i], query);

            if (flipped == query || flipped == tried)
                continue;
            tried = flipped;

            DLString lookup = facet.empty() ? flipped : facet + " " + flipped;
            HelpFinder finder(ch, lookup.c_str());
            const HelpFinder::ArticleArray &found = finder.getArticles();

            for (size_t j = 0; j < found.size() && (int)hits.size() < MAX_RESULTS; j++)
                hits.push_back(Hit{found[j], flipped, FLIP});

            if (!hits.empty())
                return true;
        }

        return false;
    }

    /** Per-word scoring against every keyword word of every visible article,
     *  only among `labels` when given. Articles matching every query word win;
     *  otherwise the ones matching the most words, a rare matched word
     *  outranking a common one. Two neighbouring words glued together also
     *  count, for both of them (fire ball -> fireball). */
    void searchWords(Character *ch, const StringSet &labels)
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

        vector<DLString> pairs;
        for (size_t i = 0; i + 1 < n; i++)
            pairs.push_back(words[i] + words[i + 1]);

        for (auto &a: helpManager->getArticles()) {
            // An alias answers to its own keywords but shows its target.
            HelpArticle::Pointer self(a);
            HelpArticle::Pointer shown = self->getAliasTarget();
            if (!shown)
                shown = self;
            if (!shown->visible(ch))
                continue;
            if (!labels.empty() && !self->labels.all.containsAny(labels))
                continue;
            if (clansOnly && !help_is_clan_article(shown))
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

            for (size_t i = 0; i < pairs.size(); i++) {
                int pairBest = 0;
                DLString pairHint;

                for (auto &k: kwords) {
                    int sc = word_score(pairs[i], k, true);
                    if (sc > pairBest) {
                        pairBest = sc;
                        pairHint = k;
                        if (sc == WORD_EXACT)
                            break;
                    }
                }

                if (pairBest <= 0)
                    continue;

                any = true;
                for (size_t j = i; j <= i + 1; j++)
                    if (pairBest > best[j]) {
                        best[j] = pairBest;
                        hint[j] = pairHint;
                    }
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

        // How many articles each query word really matched: a word matching
        // one article says more than a word matching twenty. Weak matches stay
        // out, so they cannot make a word look rare.
        vector<int> df(n, 0);
        for (auto &c: cands)
            for (size_t i = 0; i < n; i++)
                if (c.best[i] > WORD_WEAK)
                    df[i]++;

        struct Ranked {
            int strong;     // words matched by anything but a weak substitution
            int weak;       // words matched by a weak substitution only
            int score;      // of the strong words
            size_t cand;
            size_t hintWord;
        };
        vector<Ranked> ranked;
        int need = n >= 3 ? MULTI_MIN_WORDS : 1;
        bool full = false, anyStrong = false;

        for (size_t ci = 0; ci < cands.size(); ci++) {
            Ranked r = { 0, 0, 0, ci, 0 };
            int bestWord = -1;

            for (size_t i = 0; i < n; i++) {
                int b = cands[ci].best[i];
                int sc = 0;

                if (b <= 0)
                    continue;
                if (b > WORD_WEAK) {
                    sc = b + DF_BONUS / df[i];
                    r.strong++;
                    r.score += sc;
                }
                else
                    r.weak++;

                if (sc > bestWord) {
                    bestWord = sc;
                    r.hintWord = i;
                }
            }

            if (r.strong == (int)n)
                full = true;
            if (r.strong > 0 && r.strong + r.weak >= need)
                anyStrong = true;
            ranked.push_back(r);
        }

        // Strong matches decide: an article matching every word that way wins
        // outright. Otherwise a weak substitution only breaks ties between
        // strong matches, and an article matched by weak ones alone is shown
        // only when nothing matched for real.
        sort(ranked.begin(), ranked.end(), [&](const Ranked &x, const Ranked &y) {
            if (x.strong != y.strong)
                return x.strong > y.strong;
            if (x.score != y.score)
                return x.score > y.score;
            if (x.weak != y.weak)
                return x.weak > y.weak;
            return cands[x.cand].article->getID() < cands[y.cand].article->getID();
        });

        for (auto &r: ranked) {
            if ((int)hits.size() >= MAX_RESULTS)
                break;
            if (full ? r.strong < (int)n
                     : r.strong + r.weak < need || (anyStrong && r.strong == 0))
                continue;
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
    bool numeric;
    bool clansOnly;     // searchWords() looks only at the clans' own articles
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

/*-----------------------------------------------------------------------
 * World names -- help revamp Phase 4b
 *
 * A miss that is the exact name of a zone, room, creature or item gets the
 * zones it lives in instead of a dead end. One sentence per kind, zone names
 * linked to their help, no vnums: a screen reader reads it as plain prose.
 *-----------------------------------------------------------------------*/
static const unsigned int WORLD_MAX_ZONES = 5;

static DLString world_zone_list(Character *ch, const std::list<AreaIndexData *> &areas)
{
    lang_t lang = Player::displayLang(ch);
    DLString result;
    unsigned int shown = 0;

    for (auto area: areas) {
        if (shown >= WORLD_MAX_ZONES)
            break;

        if (shown > 0)
            result += ", ";

        DLString name = area->getName(lang);
        int hid = area_helpid(area);
        if (hid > 0)
            result += fmt(0, "{hh%d%s{x", hid, name.c_str());
        else
            result += name;

        shown++;
    }

    if (areas.size() > shown)
        result += fmt(ch, _(" и ещё %1$d"), (int)(areas.size() - shown));

    return result;
}

static const unsigned int WORLD_MAX_NAMES = 3;

/** Print what the world knows about the query; false when it knows nothing.
 *  A partial query names the names it completed to first. */
static bool help_world_answer(Character *ch, const DLString &query, bool keywords, bool partial = false)
{
    HelpWorldHits hits = partial ? help_world_prefix_lookup(query, keywords)
                                 : help_world_lookup(query, keywords);
    if (hits.empty())
        return false;

    ostringstream buf;
    if (hits.partial) {
        DLString names;
        unsigned int shown = 0;

        for (auto &n: hits.matchedNames) {
            if (shown >= WORLD_MAX_NAMES)
                break;
            if (shown > 0)
                names += ", ";
            names += n;
            shown++;
        }

        if (hits.matchedNames.size() > shown)
            names += fmt(ch, _(" и ещё %1$d"), (int)(hits.matchedNames.size() - shown));

        buf << fmt(ch, _("Справки с таким названием нет, но в мире есть похожее: %1$s."), names.c_str()) << endl;
    } else {
        buf << l(ch, "Справки с таким названием нет, но в мире оно встречается.") << endl;
    }

    if (!hits.areas[HWK_AREA].empty())
        buf << fmt(ch, _("Так называется зона: %1$s."),
                   world_zone_list(ch, hits.areas[HWK_AREA]).c_str()) << endl;

    if (!hits.areas[HWK_ROOM].empty())
        buf << fmt(ch, _("Так называется место в зонах: %1$s."),
                   world_zone_list(ch, hits.areas[HWK_ROOM]).c_str()) << endl;

    if (!hits.areas[HWK_MOB].empty()) {
        if (hits.keywordOnly)
            buf << fmt(ch, _("Похоже на существо из зон: %1$s."),
                       world_zone_list(ch, hits.areas[HWK_MOB]).c_str()) << endl;
        else
            buf << fmt(ch, _("Так зовут существо из зон: %1$s."),
                       world_zone_list(ch, hits.areas[HWK_MOB]).c_str()) << endl;
    }

    if (!hits.areas[HWK_OBJ].empty()) {
        if (hits.keywordOnly)
            buf << fmt(ch, _("Похоже на предмет из зон: %1$s."),
                       world_zone_list(ch, hits.areas[HWK_OBJ]).c_str()) << endl;
        else
            buf << fmt(ch, _("Так называется предмет из зон: %1$s."),
                       world_zone_list(ch, hits.areas[HWK_OBJ]).c_str()) << endl;
    }

    ch->send_to(buf);

    // One tag for the strongest kind that answered, so 4c can count them.
    static const char *tags[HWK_MAX] = { "wa", "wr", "wm", "wo" };
    if (hits.partial) {
        bugTracker->reportNohelpAnswered(ch->getPC(), query, "wp");
        return true;
    }

    for (int k = 0; k < HWK_MAX; k++)
        if (!hits.areas[k].empty()) {
            bugTracker->reportNohelpAnswered(ch->getPC(), query, tags[k]);
            break;
        }

    return true;
}

/** A bare number that is no help id but an area quest number, the one 'quest'
 *  takes: show the quest the way the zone help lists it. */
static bool help_quest_answer(Character *ch, const DLString &query)
{
    Integer vnum;
    if (!Integer::tryParse(vnum, query))
        return false;

    auto q = areaQuests.find(vnum.getValue());
    if (q == areaQuests.end())
        return false;

    AreaQuest *quest = q->second;
    if (quest->flags.isSet(AQUEST_HIDDEN) || !help_world_area_listed(quest->pAreaIndex))
        return false;

    ostringstream buf, line;
    area_quest_format(quest, line, ch);

    std::list<AreaIndexData *> zone;
    zone.push_back(quest->pAreaIndex);

    buf << fmt(ch, _("Справки с номером %1$d нет, но так пронумеровано задание из зоны %2$s:"),
               vnum.getValue(), world_zone_list(ch, zone).c_str())
        << endl
        << line.str() << endl;
    ch->send_to(buf);

    bugTracker->reportNohelpAnswered(ch->getPC(), query, "aq");
    return true;
}

/*-----------------------------------------------------------------------
 * Public achievements -- config/achievements_public.json
 *
 * Players type an achievement's name into help. The file is generated from
 * the achievement catalog by dreamland_fenia scripts/achievements/
 * gen-registry.py and holds the public ones only: a secret never reaches it,
 * so help cannot spoil one. "keys" are the names reduced the way
 * help_ach_norm() reduces a query.
 *-----------------------------------------------------------------------*/
struct PublicAchievement {
    DLString name[LANG_MAX];
    DLString desc[LANG_MAX];
    vector<DLString> keys;
};

static vector<PublicAchievement> publicAchievements;

CONFIGURABLE_LOADED(config, achievements_public)
{
    static const char *langs[LANG_MAX] = { "en", "ru", "ua" };

    publicAchievements.clear();
    if (!value.isArray())
        return;
    for (unsigned int i = 0; i < value.size(); i++) {
        const Json::Value &v = value[i];
        PublicAchievement a;

        for (int l = 0; l < LANG_MAX; l++) {
            a.name[l] = v["name"][langs[l]].asString();
            a.desc[l] = v["desc"][langs[l]].asString();
        }
        for (unsigned int k = 0; k < v["keys"].size(); k++)
            a.keys.push_back(DLString(v["keys"][k].asString()));

        publicAchievements.push_back(a);
    }
}

static const int ACH_HELP_ID = 5156;
static const size_t ACH_MIN_PREFIX = 4;
static const int ACH_MAX = 3;

/** Lowercase, ё folded, apostrophes and quotes dropped, any other ASCII
 *  punctuation a space, single spaces: gen-registry.py match_key(). */
static DLString help_ach_norm(const DLString &query)
{
    DLString s = query.colourStrip().toLower();
    DLString result;
    bool space = false;

    help_fold_yo(s);
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];

        if (c == '\'' || c == '"')
            continue;
        if (c < 128 && !isalnum(c) && c != '_') {
            space = true;
            continue;
        }
        if (space && !result.empty())
            result += ' ';
        space = false;
        result += (char)c;
    }
    return result;
}

/** A query naming a public achievement: its name and condition, then the
 *  achievements article. Exact names only, or with 'prefix' a start of a name
 *  of four letters or more. */
static bool help_ach_answer(Character *ch, const DLString &query, bool prefix)
{
    DLString q = help_ach_norm(query);
    if (q.empty() || (prefix && q.size() < ACH_MIN_PREFIX))
        return false;

    lang_t lang = Player::displayLang(ch);
    ostringstream buf;
    int shown = 0;

    for (auto &a: publicAchievements) {
        if (shown >= ACH_MAX)
            break;

        bool hit = false;
        for (auto &k: a.keys)
            if (prefix ? (k.size() > q.size() && k.compare(0, q.size(), q) == 0) : k == q) {
                hit = true;
                break;
            }
        if (!hit)
            continue;

        const DLString &name = a.name[lang].empty() ? a.name[LANG_RU] : a.name[lang];
        const DLString &desc = a.desc[lang].empty() ? a.desc[LANG_RU] : a.desc[lang];

        // The texts are arguments, never the format: a condition like
        // "100%" must reach the player as written.
        buf << fmt(ch, _("Достижение {W%1$s{x: %2$s"), name.c_str(), desc.c_str()) << endl;
        shown++;
    }

    if (shown == 0)
        return false;

    HelpArticle::Pointer help = helpManager->getArticle(ACH_HELP_ID);
    if (help && help->visible(ch))
        buf << fmt(ch, _("Подробнее: %1$s."),
                   help_link(ACH_HELP_ID, help->getTitle(DLString::emptyString, lang)).c_str()) << endl;

    ch->send_to(buf);
    bugTracker->reportNohelpAnswered(ch->getPC(), query, "ach");
    return true;
}

/** A query that is exactly a player's name, the login or the name in any
 *  language: point at the profile. Immortals stay out, whatever whois shows. */
static bool help_player_answer(Character *ch, const DLString &query)
{
    DLString q = help_world_normalize(query);
    if (q.size() < 2 || q.find(' ') != DLString::npos)
        return false;

    static const lang_t langs[] = { LANG_EN, LANG_RU, LANG_UA };
    PCMemoryInterface *found = 0;

    for (auto &p: PCharacterManager::getPCM()) {
        PCMemoryInterface *pci = p.second;

        if (pci->getLevel() >= LEVEL_IMMORTAL)
            continue;

        if (help_world_normalize(pci->getName()) == q)
            found = pci;
        for (int l = 0; !found && l < 3; l++)
            if (help_world_normalize(pci->getNameP('1', langs[l])) == q)
                found = pci;
        if (found)
            break;
    }

    if (!found)
        return false;

    lang_t lang = Player::displayLang(ch);
    DLString whois = "whois";
    Command::Pointer cmd = commandManager->findExact("whois");
    if (cmd)
        whois = cmd->getNameFor(lang);

    ostringstream buf;
    buf << fmt(ch, _("%1$s -- это игрок, смотри {y{hc%2$s %3$s{x."),
               found->getNameP('1', lang).colourStrip().c_str(),
               whois.c_str(), found->getName().c_str()) << endl;
    ch->send_to(buf);

    bugTracker->reportNohelpAnswered(ch->getPC(), query, "pc");
    return true;
}

/** A miss that starts with a command name ('зоны 10 15', 'кто Вася') was most
 *  likely meant for the command itself: say so before the guesses. */
static bool help_command_hint(Character *ch, const DLString &query)
{
    DLString rest = query;
    DLString first = rest.getOneArgument();
    first.toLower();

    if (first.empty())
        return false;

    Command::Pointer cmd = commandManager->findExact(first);
    if (!cmd || !cmd->visible(ch) || cmd->getName() == "help")
        return false;

    lang_t lang = Player::displayLang(ch);
    DLString hint = cmd->hint.getForLang(lang);
    ostringstream buf;

    if (hint.empty())
        buf << fmt(ch, _("Похоже на команду: {y{hc%1$s{x."), query.c_str()) << endl;
    else
        buf << fmt(ch, _("Похоже на команду: {y{hc%1$s{x -- %2$s."), query.c_str(), hint.c_str()) << endl;

    HelpArticle::Pointer help = cmd->getHelp();
    if (help) {
        HelpArticle::Pointer target = help->getAliasTarget();
        if (target)
            help = target;
    }
    if (help && help->getID() > 0 && help->visible(ch))
        buf << fmt(ch, _("Справка по ней: {hh%1$d%2$s{x."),
                   help->getID(), help->getTitle(DLString::emptyString, lang).c_str()) << endl;

    ch->send_to(buf);
    return true;
}

// Remember an article the player opened. Distinct ids, kept across remorts, so
// that asking for one article over and over earns nothing.
static void help_article_read(Character *ch, int id)
{
    if (id <= 0)
        return;

    XMLAttributeVisited::Pointer read =
            ch->getPC()->getAttributes().getAttr<XMLAttributeVisited>("helpsread");

    if (!read || !read->visit(id))
        return;

    int total = read->count();

    if (total >= 100)
        achievement(ch, "rtfm");
    if (total >= 1000)
        achievement(ch, "walking_manual");
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
    bool nthMiss = false;
    if (origArgument.size() > 1 && strchr( argument , '.')) {
        char argall[MAX_INPUT_LENGTH];
        int number = number_argument(argument, argall);

        if (number >= 1) {
            HelpArticle::Pointer help = HelpFinder(ch, argall).get(number);
            if (help) {
                page_to_char( help->getText( ch ).c_str( ), ch );
                help_article_read(ch, help->getID());
                return;
            }
            // No such nth match: search the word itself, so the player gets
            // the numbered list to pick from instead of a dead end.
            strcpy(argument, argall);
            nthMiss = true;
        }
        else {
            // Restore original argument without the dot, assume it was a typo.
            strcpy(argument, origArgument.substitute('.', ' ').c_str());
        }
    }
    
    // Поиск по строке без чисел.
    HelpFinder::ArticleArray articles = HelpFinder(ch, argument).getArticles();
    // No match. A quest number or a whole world name says more than a guess, a
    // command name goes above the guesses, a lone mob/object keyword below them.
    if (articles.empty()) {
        if (!nthMiss && help_quest_answer(ch, origArgument))
            return;

        if (!nthMiss && help_world_answer(ch, origArgument, false))
            return;

        // The start of a world name beats a guess: "заброшенный тов" is a
        // room, not a typo of some article.
        if (!nthMiss && help_world_answer(ch, origArgument, false, true))
            return;

        // An exact achievement or player name is no typo either: it beats a guess.
        if (!nthMiss && help_ach_answer(ch, origArgument, false))
            return;

        if (!nthMiss && help_player_answer(ch, origArgument))
            return;

        bool commandShown = !nthMiss && help_command_hint(ch, origArgument);

        if (origArgument.size() > 1) {
            FuzzySearch fs(ch, argument);
            if (fs.hasResults()) {
                fs.printResults(ch);
                // A text miss stays in nohelp.txt for analysis and the replay
                // even when a guess answered it, but no human needs to read
                // it. Numeric guesses mostly answer someone reading the help
                // by number, which is not a miss at all.
                if (!fs.isNumeric())
                    bugTracker->reportNohelpAnswered( ch->getPC(), origArgument.c_str( ) );
                return;
            }
        }
        
        if (commandShown) {
            bugTracker->reportNohelpAnswered(ch->getPC(), origArgument, "cmd");
            return;
        }

        if (!nthMiss && help_world_answer(ch, origArgument, true))
            return;

        if (!nthMiss && help_world_answer(ch, origArgument, true, true))
            return;

        // The start of an achievement name, when nothing else came close.
        if (!nthMiss && help_ach_answer(ch, origArgument, true))
            return;

        ch->pecho(_("Нет подсказки по данному слову."));
        bugTracker->reportNohelp( ch->getPC(), origArgument.c_str( ) );
        return;
    }

    if (nthMiss)
        bugTracker->reportNohelpAnswered( ch->getPC(), origArgument.c_str( ) );

    // Exact match - bingo.
    if (articles.size() == 1) {
        page_to_char( articles.front()->getText( ch ).c_str( ), ch );
        help_article_read(ch, articles.front()->getID());
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


