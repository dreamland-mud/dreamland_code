#include <algorithm>
#include <cmath>
#include <set>

#include "olcvalidate.h"
#include "room.h"
#include "area.h"
#include "exits.h"
#include "extradescription.h"
#include "mobilefactory.h"
#include "objectfactory.h"
#include "resets.h"
#include "race.h"
#include "mobbody.h"
#include "skill.h"
#include "skillmanager.h"
#include "itemeconomy.h"
#include "armorgenerator.h"
#include "weapontier.h"
#include "magicitemgenerator.h"
#include "directions.h"
#include "dl_ctype.h"
#include "lang.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * Caps. No lint carried length caps before; these sit above the 99.9th
 * percentile of dreamland_areas (2026-11-18), so only real outliers report.
 * Visible characters, colour codes not counted.
 *-------------------------------------------------------------------------*/
static const int kOlcCapName = 60;      // room name (p99.9 = 49)
static const int kOlcCapShort = 100;    // mob/obj short (p99.9 = 92)
static const int kOlcCapLong = 200;     // mob long, obj ground line (p99.9 = 112 / 299 for objects)
static const int kOlcCapKeyword = 80;   // keyword list (p99.9 = 51)
static const int kOlcCapDesc = 2500;    // room and mob description (max 2272)

static const lang_t olcLangs[] = { EN, RU, UA };

/*--------------------------------------------------------------------------
 * Context
 *-------------------------------------------------------------------------*/
struct OlcCheck {
    OlcFindings &out;
    DLString kind;
    int vnum;

    OlcCheck(OlcFindings &o, const DLString &k, int v) : out(o), kind(k), vnum(v) { }

    void add(const char *severity, const char *check, const DLString &field,
             const DLString &lang, const DLString &message)
    {
        OlcFinding f;
        f.severity = severity;
        f.check = check;
        f.kind = kind;
        f.vnum = vnum;
        f.field = field;
        f.lang = lang;
        f.message = message;
        out.push_back(f);
    }

    void flag(const char *check, const DLString &field, const DLString &lang, const DLString &message)
    {
        add("warning", check, field, lang, message);
    }
};

/*--------------------------------------------------------------------------
 * KOI8 text helpers
 *-------------------------------------------------------------------------*/
static inline bool is_latin(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool is_cyr(unsigned char c)
{
    // 0xC0-0xFF is the KOI8 Cyrillic block; dl_is_cyrillic adds ё and the
    // Ukrainian letters below it.
    return c >= 0xC0 || dl_is_cyrillic((char)c);
}

static inline bool is_word_char(unsigned char c)
{
    return is_latin(c) || (c >= '0' && c <= '9') || c == '_' || is_cyr(c);
}

/** Up to 40 bytes of a text, on one line, for a message. */
static DLString snippet(const DLString &text)
{
    DLString s = text.substr(0, 40);
    for (auto &c: s)
        if (c == '\n' || c == '\r')
            c = ' ';
    if (text.size() > 40)
        s += "...";
    return s;
}

static DLString lowered(const DLString &text)
{
    DLString s = text;
    for (auto &c: s)
        c = dl_tolower(c);
    return s;
}

/** Visible characters: every '{' with the character after it is markup. */
static int visible_length(const DLString &text)
{
    int len = 0;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '{' && i + 1 < text.size()) {
            i++;
            continue;
        }
        if (text[i] != '\n' && text[i] != '\r')
            len++;
    }
    return len;
}

/** Whitespace-separated words of a keyword list, lowercased. */
static std::set<DLString> keyword_words(const DLString &text, size_t minLen = 1)
{
    std::set<DLString> words;
    DLString args = lowered(text);
    while (!args.empty()) {
        DLString w = args.getOneArgument();
        if (w.size() >= minLen)
            words.insert(w);
    }
    return words;
}

/*--------------------------------------------------------------------------
 * Text checks
 *-------------------------------------------------------------------------*/

/** text.charset: KOI8-safe after toKoi, asserted. */
static void check_charset(OlcCheck &c, const DLString &field, const DLString &lang, const DLString &text)
{
    for (size_t i = 0; i < text.size(); i++) {
        unsigned char b = (unsigned char)text[i];
        if (b == '\t' || b == '\n' || b == '\r' || (b >= 0x20 && b < 0x7F) || is_cyr(b))
            continue;

        char hex[8];
        snprintf(hex, sizeof(hex), "0x%02X", b);
        c.flag("text.charset", field, lang,
               DLString("byte ") + hex + " at " + DLString((int)i) + " is neither ASCII nor a KOI8 letter");
        return;
    }
}

// scripts/lint-mixed-script.py DECLINED_NAMES: a Latin handle, an apostrophe
// and a short Cyrillic case ending is how RU/UA decline a foreign nickname
// (Kit's ruling 2026-08-12). An allowlist by name, never a shape rule.
static const char *declinedNames[] = { "McLaud", "Jaceks", "Xok", "Merriman", "Denuck", "Hart", 0 };

static bool is_declined_name(const DLString &token)
{
    for (int i = 0; declinedNames[i]; i++) {
        DLString n = declinedNames[i];
        if (token.size() <= n.size() + 1 || token.compare(0, n.size(), n) != 0 || token[n.size()] != '\'')
            continue;
        DLString ending = token.substr(n.size() + 1);
        if (ending.size() > 4)
            continue;
        bool cyr = true;
        for (auto ch: ending)
            if (!is_cyr((unsigned char)ch))
                cyr = false;
        if (cyr)
            return true;
    }
    return false;
}

/** The markup lint-mixed-script.py strips before tokenizing, blanked out:
 *  {tags glued to their label, printf/act placeholders, $-codes, \n escapes. */
static DLString strip_markup(const DLString &text)
{
    DLString s = text;
    size_t n = s.size();

    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        size_t j = i;

        if (c == '{' && i + 1 < n && is_latin((unsigned char)s[i + 1])) {
            // TAG_RE: \{[A-Za-z][A-Za-z]?\d*
            j = i + 2;
            if (j < n && is_latin((unsigned char)s[j]))
                j++;
            while (j < n && isdigit((unsigned char)s[j]))
                j++;
        }
        else if (c == '%' && i + 1 < n) {
            // PLACEHOLDER_RE: %(?:%|[-0-9.#]*\$?[-0-9.#]*\^?[A-Za-z])
            if (s[i + 1] == '%')
                j = i + 2;
            else {
                size_t k = i + 1;
                while (k < n && strchr("-0123456789.#", s[k]))
                    k++;
                if (k < n && s[k] == '$')
                    k++;
                while (k < n && strchr("-0123456789.#", s[k]))
                    k++;
                if (k < n && s[k] == '^')
                    k++;
                if (k < n && is_latin((unsigned char)s[k]))
                    j = k + 1;
            }
        }
        else if (c == '$' && i + 1 < n && is_latin((unsigned char)s[i + 1])) {
            // DOLLAR_RE: \$[A-Za-z]\d*
            j = i + 2;
            while (j < n && isdigit((unsigned char)s[j]))
                j++;
        }
        else if (c == '\\' && i + 1 < n && strchr("nrt", s[i + 1])) {
            j = i + 2;
        }

        for (size_t k = i; k < j; k++)
            s[k] = ' ';
        if (j > i)
            i = j - 1;
    }

    return s;
}

/** text.mixed_script */
static void check_mixed_script(OlcCheck &c, const DLString &field, const DLString &lang, const DLString &text)
{
    DLString s = strip_markup(text);
    std::vector<DLString> bad;

    for (size_t i = 0; i < s.size(); ) {
        if (!is_word_char((unsigned char)s[i])) {
            i++;
            continue;
        }

        // WORD_RE: word characters, then word characters or apostrophes.
        size_t j = i;
        while (j < s.size() && is_word_char((unsigned char)s[j]))
            j++;
        while (j < s.size() && (is_word_char((unsigned char)s[j]) || s[j] == '\'' || s[j] == '`'))
            j++;

        DLString token = s.substr(i, j - i);
        bool lat = false, cyr = false;
        for (auto ch: token) {
            lat = lat || is_latin((unsigned char)ch);
            cyr = cyr || is_cyr((unsigned char)ch);
        }

        if (lat && cyr && !is_declined_name(token) && bad.size() < 3)
            bad.push_back(token);

        i = j;
    }

    if (bad.empty())
        return;

    DLString list;
    for (auto &t: bad)
        list += (list.empty() ? "" : ", ") + t;
    c.flag("text.mixed_script", field, lang, "Latin and Cyrillic letters in one word: " + list);
}

/** text.colour: the last colour opened is reset before the end ({x, {X or a
 *  {2 restore; {w is plain white, a colour like any other). Visibility
 *  tags ({l {n {s {A {S {T {I {F {h) take one parameter character and are no colour. */
static void check_colour(OlcCheck &c, const DLString &field, const DLString &lang, const DLString &text)
{
    bool open = false;
    char last = 0;

    for (size_t i = 0; i + 1 < text.size(); i++) {
        if (text[i] != '{')
            continue;

        char t = text[i + 1];
        i++;

        if (t == '{')
            continue;
        if (strchr("lnsASTIFh", t)) {
            i++;
            continue;
        }
        if (t == 'x' || t == 'X' || t == '2') {
            open = false;
            continue;
        }
        if (strchr("bcgmrwyBCGMRWYD", t)) {
            open = true;
            last = t;
        }
    }

    if (open)
        c.flag("text.colour", field, lang, DLString("colour {") + last + " is never reset with {x");
}

/** text.dash: house style is " -- " for a dash; info only. */
static void check_dash(OlcCheck &c, const DLString &field, const DLString &lang, const DLString &text)
{
    if (text.find(" - ") != DLString::npos)
        c.add("info", "text.dash", field, lang, "spaced single hyphen, use \" -- \"");
}

static void check_length(OlcCheck &c, const DLString &field, const DLString &lang,
                         const DLString &text, int cap)
{
    int len = visible_length(text);
    if (cap > 0 && len > cap)
        c.flag("text.length", field, lang,
               DLString(len) + " visible characters, the cap is " + DLString(cap));
}

/** Every per-text check of one language value. */
static void check_text(OlcCheck &c, const DLString &field, const DLString &lang,
                       const DLString &text, int cap)
{
    if (text.empty())
        return;

    check_charset(c, field, lang, text);
    check_mixed_script(c, field, lang, text);
    check_colour(c, field, lang, text);
    check_dash(c, field, lang, text);
    check_length(c, field, lang, text, cap);
}

/** The text checks over all languages of a field, plus text.lang when
 *  langRule: RU present, EN or UA empty. */
static void check_multi(OlcCheck &c, const DLString &field, const XMLMultiString &ms,
                        int cap, bool langRule)
{
    for (auto lang: olcLangs)
        check_text(c, field, lang2attr(lang), ms.get(lang), cap);

    if (!langRule || ms.get(RU).empty())
        return;

    if (ms.get(EN).empty())
        c.flag("text.lang", field, "en", "RU text has no EN version");
    if (ms.get(UA).empty())
        c.flag("text.lang", field, "ua", "RU text has no UA version");
}

/*--------------------------------------------------------------------------
 * Keyword in the ground line (scripts/find-longs-without-keyword.py)
 *-------------------------------------------------------------------------*/

// Inflection endings (RU/UA noun and adjective, EN plural): a long word is a
// form of a keyword when it is the keyword's stem plus one of these.
static const char *kwEndings[] = {
    "", "а", "я", "о", "е", "ы", "и", "і", "ї", "у", "ю", "є", "ь", "й", "ъ",
    "ой", "ей", "ом", "ем", "ам", "ям", "ах", "ях", "ов", "ев", "ів", "ью", "ье", "ою", "ею", "ої", "ій",
    "ого", "его", "ому", "ему", "ым", "им", "ім", "ое", "ее", "ые", "ие", "ых", "их", "ові", "еві",
    "ыми", "ими",
    "s", "es",
    0
};

static bool is_ending(const DLString &s)
{
    for (int i = 0; kwEndings[i]; i++)
        if (s == kwEndings[i])
            return true;
    return false;
}

static std::vector<DLString> long_words(const DLString &text)
{
    std::vector<DLString> out;
    DLString s;

    // Colour codes go, flexer pipes go, then words of letters, digits, ' and -.
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '{' && i + 1 < text.size()) {
            i++;
            continue;
        }
        if (text[i] != '|')
            s += dl_tolower(text[i]);
    }

    for (size_t i = 0; i < s.size(); ) {
        unsigned char ch = (unsigned char)s[i];
        if (!is_word_char(ch) && ch != '\'' && ch != '-') {
            i++;
            continue;
        }
        size_t j = i;
        while (j < s.size() && (is_word_char((unsigned char)s[j]) || s[j] == '\'' || s[j] == '-'))
            j++;

        DLString t = s.substr(i, j - i);
        i = j;

        size_t a = t.find_first_not_of("'-");
        size_t b = t.find_last_not_of("'-");
        if (a == DLString::npos)
            continue;
        t = t.substr(a, b - a + 1);
        out.push_back(t);

        // Hyphen parts, so "рыцарь" is found in "рыцарь-сидх".
        if (t.find('-') != DLString::npos) {
            DLString rest = t;
            size_t p;
            while ((p = rest.find('-')) != DLString::npos) {
                if (p >= 3)
                    out.push_back(rest.substr(0, p));
                rest = rest.substr(p + 1);
            }
            if (rest.size() >= 3)
                out.push_back(rest);
        }
    }

    return out;
}

static bool match_stem(const DLString &kw, const DLString &lw)
{
    if (kw.size() < 3)
        return kw == lw;

    for (size_t cut = 0; cut <= 2; cut++) {
        if (kw.size() - cut < 3)
            break;
        DLString stem = kw.substr(0, kw.size() - cut);
        if (lw.size() >= stem.size() && lw.compare(0, stem.size(), stem) == 0
            && is_ending(lw.substr(stem.size())))
            return true;
    }
    return false;
}

/** text.kw_in_long, per language that has both a keyword and a ground line. */
static void check_kw_in_long(OlcCheck &c, const DLString &field,
                             const XMLMultiString &keyword, const XMLMultiString &line)
{
    for (auto lang: olcLangs) {
        const DLString &kw = keyword.get(lang);
        const DLString &lg = line.get(lang);
        if (kw.empty() || lg.empty())
            continue;

        std::vector<DLString> kws = long_words(kw), lws = long_words(lg);
        bool found = false;
        for (auto &k: kws)
            for (auto &l: lws)
                if (!found && match_stem(k, l))
                    found = true;

        if (!found)
            c.flag("text.kw_in_long", field, lang2attr(lang),
                   "none of the keywords [" + snippet(kw) + "] is in the line");
    }
}

/*--------------------------------------------------------------------------
 * Extra descriptions and exits
 *-------------------------------------------------------------------------*/

/** structure.keyword and text checks of an ED list. */
static void check_eds(OlcCheck &c, const ExtraDescrList &eds)
{
    for (auto &ed: eds) {
        DLString field = "ed " + ed->keyword;

        for (auto lang: olcLangs)
            check_text(c, field, lang2attr(lang), ed->description.get(lang), 0);

        if (ed->keyword.empty()) {
            c.flag("structure.keyword", "ed", "", "an extra description with an empty keyword");
            continue;
        }

        // scripts/audit-kw-coverage.py no_en / no_cyr: the blob holds all
        // languages, so only a whole missing script is certain.
        bool lat = false, cyr = false;
        for (auto ch: ed->keyword) {
            lat = lat || is_latin((unsigned char)ch);
            cyr = cyr || is_cyr((unsigned char)ch);
        }
        if (!lat && !ed->description.get(EN).empty())
            c.flag("structure.keyword", field, "en", "EN description, no Latin keyword");
        if (!cyr && (!ed->description.get(RU).empty() || !ed->description.get(UA).empty()))
            c.flag("structure.keyword", field, "ru", "RU/UA description, no Cyrillic keyword");
    }
}

/** structure.keyword_lang: a door or extra exit keyword some languages lack. */
static void check_exit_keyword(OlcCheck &c, const DLString &field, const XMLMultiString &kw)
{
    if (kw.emptyValues())
        return;

    for (auto lang: olcLangs)
        if (kw.get(lang).empty())
            c.flag("structure.keyword_lang", field, lang2attr(lang),
                   "keyword set in other languages, empty here: players of this language can't name it");
}

static bool room_exists(int vnum)
{
    return vnum > 0 && get_room_index(vnum) != 0;
}

/** Display lookup of lint-extra-exits.py for_lang: the language, then RU, then EN. */
static const DLString & for_lang(const XMLMultiString &ms, lang_t lang)
{
    if (!ms.get(lang).empty())
        return ms.get(lang);
    if (!ms.get(RU).empty())
        return ms.get(RU);
    return ms.get(EN);
}

/** A 'stem|nom|gen|...' pad can produce this case for every padded word. */
static bool declinable(const DLString &pad, int gcase)
{
    if (pad.find('|') == DLString::npos)
        return true;

    DLString args = pad;
    while (!args.empty()) {
        DLString word = args.getOneArgument();
        int bars = std::count(word.begin(), word.end(), '|');
        if (bars > 0 && bars <= gcase - 1)
            return false;
    }
    return true;
}

/** structure.eexit_msg: scripts/lint-extra-exits.py over one extra exit. */
static void check_eexit_messages(OlcCheck &c, extra_exit_data *ex)
{
    static const struct { const char *name; XMLMultiString extra_exit_data::*msg; } msgs[] = {
        { "msgLeaveRoom", &extra_exit_data::msgLeaveRoom },
        { "msgEntryRoom", &extra_exit_data::msgEntryRoom },
        { "msgLeaveSelf", &extra_exit_data::msgLeaveSelf },
        { "msgEntrySelf", &extra_exit_data::msgEntrySelf },
    };

    DLString kw = ex->keyword.get(EN).empty() ? ex->keyword.get(RU) : ex->keyword.get(EN);

    for (auto &m: msgs) {
        DLString name = m.name;
        DLString field = "eexit " + kw + " " + name;
        // Arrival lines read the far side of the passage, departures the near one.
        bool entry = name.find("Entry") != DLString::npos;
        const XMLMultiString &side = entry ? ex->short_desc_to : ex->short_desc_from;
        const XMLMultiString &other = entry ? ex->short_desc_from : ex->short_desc_to;

        for (auto lang: olcLangs) {
            DLString text = (ex->*m.msg).get(lang);
            text.stripWhiteSpace();
            if (text.empty())
                continue;

            DLString la = lang2attr(lang);
            check_text(c, field, la, text, 0);

            for (size_t p = 0; p + 2 < text.size(); p++)
                if (text[p] == '%' && isdigit((unsigned char)text[p + 1]) && text[p + 2] != '$'
                    && (text[p + 2] == '^' || is_latin((unsigned char)text[p + 2])))
                {
                    c.flag("structure.eexit_msg", field, la, "unnumbered placeholder: use %N$ forms");
                    break;
                }

            if (text[text.size() - 1] == '.' && name.find("Room") != DLString::npos)
                c.flag("structure.eexit_msg", field, la, "ends in a full stop, the engine adds its own");

            if (lang == UA) {
                size_t p = 0;
                while ((p = text.find("%1$G", p)) != DLString::npos) {
                    size_t e = text.find(' ', p);
                    DLString word = text.substr(p, e == DLString::npos ? DLString::npos : e - p);
                    if (word.find("ось|ся|ась") != DLString::npos) {
                        c.flag("structure.eexit_msg", field, la, "Russian reflexive ending in a UA line");
                        break;
                    }
                    p += 4;
                }
            }

            const DLString &noun = for_lang(side, lang).empty() ? for_lang(other, lang) : for_lang(side, lang);
            size_t p = 0;
            while ((p = text.find("%4$N", p)) != DLString::npos) {
                p += 4;
                if (p >= text.size() || !isdigit((unsigned char)text[p]))
                    continue;
                int gcase = text[p] - '0';

                if (noun.empty()) {
                    c.flag("structure.eexit_msg", field, la,
                           DLString("wants %4 but short_desc_") + (entry ? "to" : "from") + " is empty");
                    break;
                }
                if ((lang == RU || lang == UA) && !declinable(noun, gcase))
                    c.flag("structure.eexit_msg", field, la,
                           "asks for case " + DLString(gcase) + ", [" + snippet(noun) + "] has no cell for it");
            }
        }
    }
}

/*--------------------------------------------------------------------------
 * Prototypes
 *-------------------------------------------------------------------------*/

static void check_range(OlcCheck &c, AreaIndexData *pArea)
{
    if (!pArea)
        return;
    if (c.vnum < pArea->min_vnum || c.vnum > pArea->max_vnum)
        c.flag("structure.vnum_range", "vnum", "",
               "outside the area's range " + DLString(pArea->min_vnum) + "-" + DLString(pArea->max_vnum));
}

static void check_resets(OlcCheck &c, RoomIndexData *pRoom)
{
    int n = 0;
    for (auto &r: pRoom->resets) {
        n++;
        DLString field = "reset " + DLString(n);
        std::vector<int> objs;
        int room = 0, mob = 0;

        switch (r->command) {
        case 'M':
            mob = r->arg1;
            break;
        case 'O':
        case 'G':
        case 'E':
            objs.push_back(r->arg1);
            break;
        case 'P':
            objs.push_back(r->arg1);
            objs.push_back(r->arg3);
            break;
        case 'D':
        case 'R':
            room = r->arg1;
            break;
        default:
            break;
        }

        // A put may pick one of several vnums.
        for (auto v: r->vnums)
            if (v != r->arg1)
                objs.push_back(v);

        if (r->command == 'M') {
            mob_index_data *pMob = mob > 0 ? get_mob_index(mob) : 0;
            if (!pMob || IS_SET(pMob->act, ACT_DELETED))
                c.flag("structure.reset", field, "", "loads mob " + DLString(mob) + ", which does not exist or is deleted");
        }

        for (auto v: objs) {
            obj_index_data *pObj = v > 0 ? get_obj_index(v) : 0;
            if (!pObj || IS_SET(pObj->extra_flags, ITEM_DELETED))
                c.flag("structure.reset", field, "", "names object " + DLString(v) + ", which does not exist or is deleted");
        }

        if ((r->command == 'D' || r->command == 'R') && !room_exists(room))
            c.flag("structure.reset", field, "", "names room " + DLString(room) + ", which does not exist");
    }
}

void olc_validate_room(RoomIndexData *pRoom, OlcFindings &out)
{
    if (!pRoom)
        return;

    OlcCheck c(out, "room", pRoom->vnum);

    check_range(c, pRoom->areaIndex);
    check_multi(c, "name", pRoom->name, kOlcCapName, true);
    check_multi(c, "desc", pRoom->description, kOlcCapDesc, true);
    check_multi(c, "smell", pRoom->smell, 0, false);
    check_multi(c, "sound", pRoom->sound, 0, false);
    check_eds(c, pRoom->extraDescriptions);
    check_resets(c, pRoom);

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        exit_data *pexit = pRoom->exit[door];
        if (!pexit)
            continue;

        DLString field = DLString("exit ") + dirs[door].name;
        if (!room_exists(pexit->u1.vnum))
            c.flag("structure.exit", field, "", "leads to room " + DLString(pexit->u1.vnum) + ", which does not exist");

        check_exit_keyword(c, field, pexit->keyword);
        check_multi(c, field + " desc", pexit->description, 0, false);
        check_multi(c, field + " short", pexit->short_descr, 0, false);
    }

    std::set<DLString> eexitWords;

    for (auto &ex: pRoom->extra_exits) {
        DLString kw = ex->keyword.get(EN).empty() ? ex->keyword.get(RU) : ex->keyword.get(EN);
        DLString field = "eexit " + kw;

        if (!room_exists(ex->u1.vnum))
            c.flag("structure.exit", field, "", "leads to room " + DLString(ex->u1.vnum) + ", which does not exist");

        check_exit_keyword(c, field, ex->keyword);
        check_multi(c, field + " desc", ex->description, 0, false);
        check_multi(c, field + " room_description", ex->room_description, 0, false);
        check_multi(c, field + " short_desc_from", ex->short_desc_from, 0, false);
        check_multi(c, field + " short_desc_to", ex->short_desc_to, 0, false);
        check_eexit_messages(c, ex);

        for (auto lang: olcLangs)
            for (auto &w: keyword_words(ex->keyword.get(lang), 3))
                eexitWords.insert(w);
    }

    // structure.kw_collision: 'look <word>' finds the ED or the extra exit, never both.
    for (auto &ed: pRoom->extraDescriptions) {
        DLString shared;
        for (auto &w: keyword_words(ed->keyword, 3))
            if (eexitWords.count(w))
                shared += (shared.empty() ? "" : " ") + w;
        if (!shared.empty())
            c.flag("structure.kw_collision", "ed " + ed->keyword, "",
                   "shares [" + shared + "] with an extra exit keyword");
    }
}

/** What deriveNumbers would write for these act names, against the prototype:
 *  an empty string when they agree. Compared as dice averages. */
static DLString mob_numbers_diff(mob_index_data *pMob, Race *mobrace, const Body::NameSet &acts)
{
    const MobTiers::Config &tc = MobBody::tiers();

    MobTiers::MobInfo m;
    m.vnum = pMob->vnum;
    m.level = pMob->level;
    m.tier = pMob->tier;
    m.formAc = pMob->formAcPct / 100.0;
    m.style = pMob->tierStyle;
    m.sentient = IS_SET(pMob->form, FORM_SENTIENT);
    m.acts = acts;

    MobTiers::Centres cn = tc.centres(m);
    int hp = std::max(1, (int)lround(cn.hp * mobrace->getHpMult()));
    int dam = std::max(1, (int)lround(cn.dmgAve * mobrace->getDmgMult()));

    int hitAve = pMob->hit[DICE_NUMBER] * (pMob->hit[DICE_TYPE] + 1) / 2 + pMob->hit[DICE_BONUS];
    int damAve = pMob->damage[DICE_NUMBER] * (pMob->damage[DICE_TYPE] + 1) / 2 + pMob->damage[DICE_BONUS];
    int manaAve = pMob->mana[DICE_NUMBER] * (pMob->mana[DICE_TYPE] + 1) / 2 + pMob->mana[DICE_BONUS];
    // deriveNumbers never lets the damroll go negative, so the damage dice may
    // sit a little over the centre.
    int damSlack = (std::max(pMob->level, 0) / 10 + 1) * 2;

    DLString diff;
    if (hitAve != hp)
        diff += " hit " + DLString(hitAve) + "/" + DLString(hp);
    if (damAve < dam || damAve > dam + damSlack)
        diff += " damage " + DLString(damAve) + "/" + DLString(dam);
    if (manaAve != cn.mana)
        diff += " mana " + DLString(manaAve) + "/" + DLString(cn.mana);
    if (pMob->hitroll != cn.hitrollBonus)
        diff += " hitroll " + DLString(pMob->hitroll) + "/" + DLString(cn.hitrollBonus);
    if (pMob->ac[0] != cn.ac * 10)
        diff += " ac " + DLString(pMob->ac[0]) + "/" + DLString(cn.ac * 10);
    return diff;
}

/** power.mob_numbers: the numbers mob_index_data::deriveNumbers writes, from
 *  the same MobTiers centres and race multipliers. deriveNumbers reads the act
 *  names before the tier adds its own (warrior from champion up), and the class
 *  factor depends on them: numbers that match either set are fine. */
static void check_mob_numbers(OlcCheck &c, mob_index_data *pMob)
{
    const MobTiers::Config &tc = MobBody::tiers();
    if (!tc.loaded)
        return;

    Race *mobrace = raceManager->find(pMob->race);
    if (!mobrace)
        return;

    Body::NameSet acts = MobBody::names(&act_flags, (unsigned int)pMob->act);
    DLString diff = mob_numbers_diff(pMob, mobrace, acts);
    if (diff.empty())
        return;

    const MobTiers::Tier &tierRow = tc.get(pMob->tier);
    if (!tierRow.actAdd.empty()) {
        Body::NameSet bodyActs = acts;
        for (auto &a: tierRow.actAdd)
            bodyActs.erase(a);
        if (mob_numbers_diff(pMob, mobrace, bodyActs).empty())
            return;
    }

    c.flag("power.mob_numbers", "numbers", "",
           "differ from the tier (now/tier):" + diff + "; the next rebuild or boot resets them");
}

void olc_validate_mob(mob_index_data *pMob, OlcFindings &out)
{
    // A deleted prototype is a tombstone until the next boot.
    if (!pMob || IS_SET(pMob->act, ACT_DELETED))
        return;

    OlcCheck c(out, "mob", pMob->vnum);

    check_range(c, pMob->area);

    if (pMob->keyword.emptyValues())
        c.flag("structure.keyword", "name", "", "no keyword in any language");

    check_multi(c, "name", pMob->keyword, kOlcCapKeyword, true);
    check_multi(c, "short", pMob->short_descr, kOlcCapShort, true);
    check_multi(c, "long", pMob->long_descr, kOlcCapLong, true);
    check_multi(c, "desc", pMob->description, kOlcCapDesc, true);
    check_multi(c, "smell", pMob->smell, 0, false);
    check_kw_in_long(c, "long", pMob->keyword, pMob->long_descr);
    check_mob_numbers(c, pMob);
}

/** power.spell_price: a spell or skill whose price the M measure needs. */
static void check_spell_price(OlcCheck &c, const DLString &field, const DLString &name, bool charge)
{
    double m = charge ? magic_spell_charge(name) : magic_spell_grant(name);
    if (m < 0)
        c.flag("power.spell_price", field, "",
               DLString(charge ? "charged spell " : "granted skill ") + name
               + " has no " + (charge ? "charge" : "grant") + " price in fight/spell_value.json");
}

static void check_obj_power(OlcCheck &c, obj_index_data *pObj)
{
    // Granted skills (behavior grantskills, props {skill, learned, msg}).
    const Json::Value &props = pObj->props;
    if (props.isObject() && props.isMember("grantskills") && props["grantskills"].isArray())
        for (auto &g: props["grantskills"])
            if (g.isObject() && g["skill"].isString())
                check_spell_price(c, "props grantskills", g["skill"].asString(), false);

    // Charged spells: v1-v4 of a potion, pill or scroll, v3 of a wand or staff.
    int first = 0, last = -1;
    if (pObj->item_type == ITEM_POTION || pObj->item_type == ITEM_PILL || pObj->item_type == ITEM_SCROLL) {
        first = 1;
        last = 4;
    } else if (pObj->item_type == ITEM_WAND || pObj->item_type == ITEM_STAFF) {
        first = last = 3;
    }
    for (int v = first; v <= last; v++) {
        int sn = pObj->value[v];
        if (sn <= 0 || sn >= skillManager->size())
            continue;
        Skill *skill = skillManager->find(sn);
        if (skill)
            check_spell_price(c, "value" + DLString(v), skill->getName(), true);
    }

    // Stats rolled per instance, or weapon dice that sit outside the M window.
    if (item_is_random(pObj) || pObj->item_type == ITEM_WEAPON || weapon_tier_table.size() == 0)
        return;

    int cm = item_measure_cm(pObj);
    if (cm <= 0)
        return;

    double curve = std::max(affix_window_curve(pObj->level, "armor", false),
                            affix_window_curve(pObj->level, "armor", true));
    int top = (int)(weapon_tier_table[BEST_TIER - 1].max_m * curve);

    if (cm > top) {
        char buf[128];
        snprintf(buf, sizeof(buf), "measures %.2f M, above the legendary window %.2f M at level %d",
                 cm / 100.0, top / 100.0, pObj->level);
        c.flag("power.measure", "stats", "", buf);
    }
}

void olc_validate_obj(obj_index_data *pObj, OlcFindings &out)
{
    if (!pObj || IS_SET(pObj->extra_flags, ITEM_DELETED))
        return;

    OlcCheck c(out, "obj", pObj->vnum);

    check_range(c, pObj->area);

    if (pObj->keyword.emptyValues())
        c.flag("structure.keyword", "name", "", "no keyword in any language");

    check_multi(c, "name", pObj->keyword, kOlcCapKeyword, true);
    check_multi(c, "short", pObj->short_descr, kOlcCapShort, true);
    check_multi(c, "long", pObj->description, kOlcCapLong, true);
    check_multi(c, "smell", pObj->smell, 0, false);
    check_multi(c, "sound", pObj->sound, 0, false);
    check_eds(c, pObj->extraDescriptions);
    check_kw_in_long(c, "long", pObj->keyword, pObj->description);
    check_obj_power(c, pObj);
}

void olc_validate_area(AreaIndexData *pArea, OlcFindings &out)
{
    if (!pArea)
        return;

    OlcCheck c(out, "area", pArea->vnum);

    if (pArea->min_vnum > pArea->max_vnum)
        c.flag("structure.vnum_range", "vnums", "", "the range ends before it starts");

    check_multi(c, "name", pArea->name, kOlcCapName, true);
}

void olc_validate_area_all(AreaIndexData *pArea, OlcFindings &out)
{
    if (!pArea)
        return;

    olc_validate_area(pArea, out);

    for (auto &r: pArea->roomIndexes)
        olc_validate_room(r.second, out);

    for (int h = 0; h < MAX_KEY_HASH; h++)
        for (mob_index_data *pMob = mob_index_hash[h]; pMob; pMob = pMob->next)
            if (pMob->area == pArea)
                olc_validate_mob(pMob, out);

    for (int h = 0; h < MAX_KEY_HASH; h++)
        for (obj_index_data *pObj = obj_index_hash[h]; pObj; pObj = pObj->next)
            if (pObj->area == pArea)
                olc_validate_obj(pObj, out);
}

/*--------------------------------------------------------------------------
 * Player profile
 *-------------------------------------------------------------------------*/

// Resolved editor subcommands (redit/medit/oedit OLC_CMD names) that set a
// number, a flag, a power source or code. Unreachable for now: OlcOps::apply
// refuses the player profile until phase 5, which replaces this blacklist
// with a whitelist (a blacklist leaks: delete, purge, exits, sex...).
static const char *powerFields[] = {
    "level", "hitdice", "manadice", "damdice", "ac", "hitroll", "wealth",
    "tier", "tierstyle", "affects", "noaffects", "oaff", "detection",
    "imm", "res", "vuln", "off", "act", "form", "part", "size", "race",
    "align", "damtype", "group", "professions", "practicer", "religion",
    "spec", "shop", "reviewed", "clan", "guilds",
    "addaffect", "delaffect", "addgrant", "random",
    "value0", "value1", "value2", "value3", "value4", "v0", "v1", "v2", "v3", "v4",
    "cost", "weight", "type", "extra", "wear", "limit", "condition", "material",
    "behaviors", "oldbehavior", "props", "fenia", "copy",
    "flags", "sector", "heal", "mana", "liquid",
    0
};

bool olc_power_field(const DLString &cmdName)
{
    for (int i = 0; powerFields[i]; i++)
        if (cmdName == powerFields[i])
            return true;
    return false;
}
