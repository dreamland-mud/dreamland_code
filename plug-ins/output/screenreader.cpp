#include "screenreader.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "merc.h"
#include "dl_ctype.h"
#include "dlstring.h"
#include "def.h"

bool uses_screenreader(Descriptor *d)
{
    return d && d->character && uses_screenreader(d->character);
}

bool uses_screenreader(Character *ch)
{
    if (!ch)
        return false;
        
    if (ch->desc && ch->desc->telnet.ttype == TTYPE_LYNTIN)
        return true;

    // getPC() is the switched-from player for a possessed mob, so the
    // player's own screenreader setting follows them into the body.
    const PCharacter *pc = ch->getPC();
    if (pc && IS_SET(pc->config, CONFIG_SCREENREADER))
        return true;

    return false;
}

// Punctuation that may stay at the edge of a word: "(glowing)", "sword,".
static bool is_edge_punct(char c)
{
    return c == '(' || c == ')' || c == '"' || c == '\'' || c == '.'
        || c == ',' || c == '!' || c == '?' || c == ':';
}

// Punctuation that may stay inside a word: "fire-proof", "it's", "Blizzard(TM)".
static bool is_inner_punct(char c)
{
    return c == '-' || c == '\'' || c == '.' || c == '(' || c == ')';
}

// One space-separated word. Colour codes ({X, two bytes) always survive,
// symbols are trimmed from the edges and dropped from the middle. A word
// without letters or digits leaves only its colour codes behind.
// Three-byte tags ({IS, {lr, {Sf) keep their third char as plain text: it is
// alnum, so it is never dropped and the tag stays intact.
static DLString clean_word(const DLString &word, bool &hasText)
{
    int first = -1, last = -1;
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '{') {
            i++;
            continue;
        }
        if (dl_isalnum(word[i])) {
            if (first < 0)
                first = i;
            last = i;
        }
    }

    hasText = first >= 0;

    DLString out;
    for (size_t i = 0; i < word.size(); i++) {
        char c = word[i];
        if (c == '{') {
            out += c;
            if (i + 1 < word.size())
                out += word[++i];
            continue;
        }
        if (!hasText)
            continue;
        if (dl_isalnum(c))
            out += c;
        else if ((int)i < first || (int)i > last) {
            if (is_edge_punct(c))
                out += c;
        }
        else if (is_inner_punct(c))
            out += c;
    }
    return out;
}

DLString screenreader_clean_name(const DLString &name)
{
    DLString result, pending, word;
    bool anyText = false;

    for (size_t i = 0; i <= name.size(); i++) {
        if (i < name.size() && !dl_isspace(name[i])) {
            word += name[i];
            if (name[i] == '{' && i + 1 < name.size())
                word += name[++i];
            continue;
        }

        if (word.empty())
            continue;

        bool hasText;
        DLString cleaned = clean_word(word, hasText);
        word.clear();

        if (!hasText) {
            pending += cleaned;
            continue;
        }

        if (anyText)
            result += ' ';
        result += pending;
        result += cleaned;
        pending.clear();
        anyText = true;
    }

    if (!anyText)
        return name;

    result += pending;
    return result;
}
