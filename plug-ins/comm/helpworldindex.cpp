#include <map>
#include <vector>
#include "helpworldindex.h"
#include "area.h"
#include "room.h"
#include "mobilefactory.h"
#include "objectfactory.h"
#include "dl_strings.h"
#include "merc.h"
#include "def.h"

typedef std::pair<HelpWorldKind, AreaIndexData *> HelpWorldEntry;

/** Everything one normalized key leads to, and how to show it to a player:
 *  the name itself, or the creature/item a keyword belongs to. */
struct HelpWorldKey {
    std::vector<HelpWorldEntry> entries;
    DLString display;
};
typedef std::map<DLString, HelpWorldKey> HelpWorldMap;

/** Whole names: zones, rooms, mob and object short descriptions. */
static HelpWorldMap names;
/** Single keywords of mobs and objects: a weaker match, used only when no
 *  whole name answers. */
static HelpWorldMap keywords;
static bool built = false;

/** Words shorter than this are too common to say anything about a keyword. */
static const unsigned int MIN_KEYWORD_LEN = 4;

/** A partial query this short, or one that starts more names than this, is
 *  too vague to name anything. */
static const unsigned int MIN_PREFIX_LEN = 4;
static const unsigned int MAX_PREFIX_NAMES = 10;

bool HelpWorldHits::empty() const
{
    for (int k = 0; k < HWK_MAX; k++)
        if (!areas[k].empty())
            return false;
    return true;
}

DLString &help_fold_yo(DLString &str)
{
    // KOI8-U bytes: Ё 0xB3 / ё 0xA3 -> Е 0xE5 / е 0xC5. Byte literals, not
    // 'ё': the source is UTF-8, where the letter takes two bytes.
    str.substitute('\xB3', '\xE5');
    str.substitute('\xA3', '\xC5');
    return str;
}

DLString help_world_normalize(const DLString &str)
{
    DLString s = str.colourStrip().toLower();
    s.substitute('\'', ' ');
    s.substitute('"', ' ');
    help_fold_yo(s);

    DLString result, word;
    while (!(word = s.getOneArgument()).empty()) {
        if (!result.empty())
            result += " ";
        result += word;
    }

    static const char *articles[] = { "a ", "an ", "the ", 0 };
    for (int i = 0; articles[i]; i++) {
        DLString article = articles[i];
        if (result.size() > article.size() && result.substr(0, article.size()) == article) {
            result = result.substr(article.size());
            break;
        }
    }

    return result;
}

bool help_world_area_listed(AreaIndexData *area)
{
    return area && !IS_SET(area->area_flag, AREA_HIDDEN|AREA_SYSTEM|AREA_WIZLOCK);
}

static void add_entry(HelpWorldMap &map, const DLString &key, HelpWorldKind kind, AreaIndexData *area,
                      const DLString &display)
{
    DLString k = help_world_normalize(key);
    if (k.empty())
        return;

    HelpWorldKey &slot = map[k];
    if (slot.display.empty())
        slot.display = display.colourStrip();

    std::vector<HelpWorldEntry> &entries = slot.entries;
    HelpWorldEntry entry(kind, area);
    for (auto &e: entries)
        if (e == entry)
            return;
    entries.push_back(entry);
}

/** A name may be a Flexer pad: the nominative is the form players type. */
static DLString nominative(const DLString &name)
{
    if (name.find('|') == DLString::npos)
        return name;
    return russian_case(name, '1');
}

static void add_multi_name(const XMLMultiString &name, HelpWorldKind kind, AreaIndexData *area)
{
    for (auto &n: name)
        if (!n.second.empty()) {
            DLString nom = nominative(n.second);
            add_entry(names, nom, kind, area, nom);
        }
}

/** Keywords lead to the creature or item they belong to, shown by its
 *  name in the same language. */
static void add_keywords(const XMLMultiString &kw, const XMLMultiString &shortDescr,
                         HelpWorldKind kind, AreaIndexData *area)
{
    for (auto &k: kw) {
        DLString display = nominative(shortDescr.get(k.first));
        DLString words = k.second;
        DLString word;

        if (display.empty())
            display = nominative(shortDescr.get(LANG_RU));

        while (!(word = words.getOneArgument()).empty())
            if (word.size() >= MIN_KEYWORD_LEN)
                add_entry(keywords, word, kind, area, display);
    }
}

static void build_index()
{
    for (auto area: areaIndexes)
        if (help_world_area_listed(area)) {
            add_multi_name(area->name, HWK_AREA, area);
            add_multi_name(area->altname, HWK_AREA, area);
        }

    for (auto &r: roomIndexMap)
        if (help_world_area_listed(r.second->areaIndex))
            add_multi_name(r.second->name, HWK_ROOM, r.second->areaIndex);

    for (int i = 0; i < MAX_KEY_HASH; i++) {
        for (MOB_INDEX_DATA *m = mob_index_hash[i]; m; m = m->next)
            if (help_world_area_listed(m->area)) {
                add_multi_name(m->short_descr, HWK_MOB, m->area);
                add_keywords(m->keyword, m->short_descr, HWK_MOB, m->area);
            }

        for (OBJ_INDEX_DATA *o = obj_index_hash[i]; o; o = o->next)
            if (help_world_area_listed(o->area)) {
                add_multi_name(o->short_descr, HWK_OBJ, o->area);
                add_keywords(o->keyword, o->short_descr, HWK_OBJ, o->area);
            }
    }

    built = true;
}

static void add_hits(HelpWorldHits &hits, const HelpWorldKey &key)
{
    for (auto &e: key.entries) {
        std::list<AreaIndexData *> &list = hits.areas[e.first];
        bool seen = false;
        for (auto a: list)
            if (a == e.second)
                seen = true;
        if (!seen)
            list.push_back(e.second);
    }
}

HelpWorldHits help_world_lookup(const DLString &query, bool useKeywords)
{
    HelpWorldHits hits;
    DLString q = help_world_normalize(query);

    if (q.size() < 3)
        return hits;

    if (!built)
        build_index();

    HelpWorldMap::const_iterator i = names.find(q);
    if (i == names.end()) {
        if (!useKeywords)
            return hits;
        i = keywords.find(q);
        if (i == keywords.end())
            return hits;
        hits.keywordOnly = true;
    }

    add_hits(hits, i->second);
    return hits;
}

HelpWorldHits help_world_prefix_lookup(const DLString &query, bool useKeywords)
{
    HelpWorldHits hits;
    DLString q = help_world_normalize(query);

    if (q.size() < MIN_PREFIX_LEN)
        return hits;

    if (!built)
        build_index();

    const HelpWorldMap &map = useKeywords ? keywords : names;
    unsigned int found = 0;

    for (HelpWorldMap::const_iterator i = map.lower_bound(q);
         i != map.end() && i->first.compare(0, q.size(), q) == 0;
         i++)
    {
        // A whole name is help_world_lookup's answer, not a partial one.
        if (i->first.size() == q.size())
            continue;

        if (++found > MAX_PREFIX_NAMES)
            return HelpWorldHits();

        add_hits(hits, i->second);

        bool seen = false;
        for (auto &n: hits.matchedNames)
            if (n == i->second.display)
                seen = true;
        if (!seen)
            hits.matchedNames.push_back(i->second.display);
    }

    hits.partial = !hits.empty();
    hits.keywordOnly = useKeywords && hits.partial;
    return hits;
}
