/* Automatic "see also" for help articles -- see helprelated.h. */
#include <ctime>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

#include "helprelated.h"
#include "character.h"
#include "player_utils.h"
#include "l10n.h"

using namespace std;

namespace {

const unsigned int KEEP = 12;       // ranked candidates kept per article, before visibility
const unsigned int RARE_DF = 5;     // a keyword word in more articles than this links nothing
const unsigned int MIN_WORD = 4;
const int W_CURATED = 1000;
const int W_INBOUND = 200;          // same category links here
const int W_KEYWORD = 100;          // same category, shared rare keyword
const int W_CROSSCAT = 0;           // another category links here: only fills
const int OUTDEG = 60;              // a link out of a short article counts more than one out of a hub
const int PER_SHARED = 10;
// Help text changes through plugin reloads and hedit without telling anyone,
// so the index is simply rebuilt when it is older than this.
const time_t TTL = 300;

struct Node {
    DLString cat;
    set<int> body;              // ids the text links to, all languages
    vector<int> curated;
    set<int> linked;            // body + curated
    set<DLString> words;        // keyword words, lowercase
    set<DLString> whole;        // whole keywords, lowercase
};

map<int, vector<int> > related;
time_t builtAt = 0;
size_t builtCount = 0;

/** The article an id opens: the alias target for an alias, null if missing. */
const HelpArticle * resolve(int id)
{
    HelpArticle::Pointer a = helpManager->getArticle(id);
    if (!a)
        return 0;

    HelpArticle::Pointer target = a->getAliasTarget();
    return target ? target.getPointer() : a.getPointer();
}

void scan_links(const DLString &text, set<int> &ids)
{
    for (DLString::size_type p = text.find("{hh"); p != DLString::npos; p = text.find("{hh", p + 3)) {
        DLString::size_type d = p + 3;
        int id = 0;

        // A real id is at most a few digits; more is garbage, not an overflow.
        for (int digits = 0; d < text.size() && isdigit((unsigned char)text.at(d)); d++, digits++)
            if (digits < 9)
                id = id * 10 + (text.at(d) - '0');

        if (id <= 0)
            continue;

        const HelpArticle *a = resolve(id);
        if (a)
            ids.insert(a->getID());
    }
}

/** Word characters as in help-related.py's [\w']: letters (any byte above
 *  ASCII is a KOI8 letter), digits, underscore and the apostrophe, which stays
 *  inside a word (зв'язок) and is trimmed off its ends. */
bool is_word_char(char c)
{
    unsigned char u = (unsigned char)c;
    return u >= 128 || isalnum(u) || c == '_' || c == '\'';
}

void add_word(Node &n, DLString word)
{
    while (!word.empty() && word.at(0) == '\'')
        word.erase(0, 1);
    while (!word.empty() && word.at(word.size() - 1) == '\'')
        word.erase(word.size() - 1);

    if (word.size() >= MIN_WORD && !word.isNumber())
        n.words.insert(word);
}

void add_keyword(Node &n, const DLString &keyword)
{
    DLString kw = keyword.toLower();
    kw.stripWhiteSpace();
    while (!kw.empty() && kw.at(0) == '\'')
        kw.erase(0, 1);
    while (!kw.empty() && kw.at(kw.size() - 1) == '\'')
        kw.erase(kw.size() - 1);
    kw.stripWhiteSpace();
    if (kw.empty())
        return;

    n.whole.insert(kw);

    DLString word;
    for (DLString::size_type i = 0; i <= kw.size(); i++) {
        if (i < kw.size() && is_word_char(kw.at(i))) {
            word += kw.at(i);
            continue;
        }
        add_word(n, word);
        word.clear();
    }
}

void build()
{
    map<int, Node> nodes;

    for (auto &p: helpManager->getArticles()) {
        HelpArticle::Pointer a(p);

        if (a->getID() <= 0 || a->getAliasTarget())
            continue;

        Node &n = nodes[a->getID()];
        n.cat = a->getCategory();

        for (int l = LANG_MIN; l < LANG_MAX; l++)
            scan_links(a->text.get((lang_t)l), n.body);
        n.body.erase(a->getID());

        for (auto id: a->getSeeAlso()) {
            const HelpArticle *c = resolve(id);
            if (c && c->getID() != a->getID()
                    && find(n.curated.begin(), n.curated.end(), c->getID()) == n.curated.end())
                n.curated.push_back(c->getID());
        }

        n.linked = n.body;
        n.linked.insert(n.curated.begin(), n.curated.end());

        for (auto &k: a->getAllKeywords())
            add_keyword(n, k);
        for (auto &k: a->aka)
            add_keyword(n, k);
    }

    map<int, set<int> > inbound;
    map<DLString, unsigned int> df;
    map<DLString, vector<int> > byCat;

    for (auto &p: nodes) {
        for (auto x: p.second.linked)
            inbound[x].insert(p.first);
        for (auto &w: p.second.words)
            df[w]++;
        if (!p.second.cat.empty())
            byCat[p.second.cat].push_back(p.first);
    }

    related.clear();

    for (auto &p: nodes) {
        int id = p.first;
        Node &n = p.second;
        map<int, int> score;

        auto add = [&](int j, int w) {
            if (j != id && nodes.count(j) && !n.body.count(j))
                score[j] += w;
        };

        for (unsigned int i = 0; i < n.curated.size(); i++)
            add(n.curated[i], W_CURATED - (int)i);

        for (auto j: inbound[id]) {
            auto from = nodes.find(j);
            if (from == nodes.end())
                continue;

            int w = (!n.cat.empty() && from->second.cat == n.cat) ? W_INBOUND : W_CROSSCAT;
            add(j, w + OUTDEG / max(1, (int)from->second.linked.size()));
        }

        set<DLString> mine;
        if (!n.cat.empty())
            for (auto &w: n.words)
                if (df[w] <= RARE_DF)
                    mine.insert(w);

        if (!mine.empty()) {
            for (auto j: byCat[n.cat]) {
                if (j == id)
                    continue;

                const Node &other = nodes[j];
                int shared = 0;
                bool wholeWord = false;

                for (auto &w: mine)
                    if (other.words.count(w)) {
                        shared++;
                        if (n.whole.count(w) || other.whole.count(w))
                            wholeWord = true;
                    }

                if (shared >= 2 || wholeWord)
                    add(j, W_KEYWORD + PER_SHARED * shared);
            }
        }

        vector<pair<int, int> > ranked;
        for (auto &s: score)
            ranked.push_back(make_pair(-s.second, s.first));
        sort(ranked.begin(), ranked.end());

        vector<int> &out = related[id];
        for (unsigned int i = 0; i < ranked.size() && i < KEEP; i++)
            out.push_back(ranked[i].second);
    }

    builtAt = time(0);
    builtCount = helpManager->getArticles().size();
}

void ensure_built()
{
    if (builtAt == 0 || time(0) - builtAt > TTL
            || builtCount != helpManager->getArticles().size())
        build();
}

DLString link(const HelpArticle *a, lang_t lang)
{
    DLString label = a->getTitle("toc", lang).colourStrip();
    if (label.empty())
        label = DLString(a->getID());

    // The tag parser swallows every digit after the id, so a title that
    // starts with one needs a separator.
    if (!label.empty() && isdigit((unsigned char)label.at(0)))
        label = " " + label;

    ostringstream buf;
    buf << "{hh" << a->getID() << label << "{x {D[" << a->getID() << "]{x";
    return buf.str();
}

} // namespace

vector<HelpArticle::Pointer> help_related(const HelpArticle *article, Character *ch, unsigned int cap)
{
    vector<HelpArticle::Pointer> result;

    if (!article || !ch || !helpManager || article->getID() <= 0)
        return result;

    ensure_built();

    auto r = related.find(article->getID());
    if (r == related.end())
        return result;

    for (auto id: r->second) {
        if (result.size() >= cap)
            break;

        HelpArticle::Pointer a = helpManager->getArticle(id);
        if (a && a->visible(ch))
            result.push_back(a);
    }

    return result;
}

DLString help_related_line(const HelpArticle *article, Character *ch)
{
    vector<HelpArticle::Pointer> list = help_related(article, ch);
    if (list.empty())
        return DLString::emptyString;

    // One plain line: a screen reader reads it as a sentence, and the id after
    // each title is what a telnet player types, since links only work in the
    // web client.
    lang_t lang = Player::displayLang(ch);
    ostringstream buf;
    buf << l(ch, "{WСм. также:{x ");
    for (unsigned int i = 0; i < list.size(); i++) {
        if (i > 0)
            buf << ", ";
        buf << link(list[i].getPointer(), lang);
    }

    return buf.str();
}
