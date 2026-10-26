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
typedef std::map<DLString, std::vector<HelpWorldEntry> > HelpWorldMap;

/** Whole names: zones, rooms, mob and object short descriptions. */
static HelpWorldMap names;
/** Single keywords of mobs and objects: a weaker match, used only when no
 *  whole name answers. */
static HelpWorldMap keywords;
static bool built = false;

/** Words shorter than this are too common to say anything about a keyword. */
static const unsigned int MIN_KEYWORD_LEN = 4;

bool HelpWorldHits::empty() const
{
    for (int k = 0; k < HWK_MAX; k++)
        if (!areas[k].empty())
            return false;
    return true;
}

DLString help_world_normalize(const DLString &str)
{
    DLString s = str.colourStrip().toLower();
    s.substitute('\'', ' ');
    s.substitute('"', ' ');

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

static void add_entry(HelpWorldMap &map, const DLString &key, HelpWorldKind kind, AreaIndexData *area)
{
    DLString k = help_world_normalize(key);
    if (k.empty())
        return;

    std::vector<HelpWorldEntry> &entries = map[k];
    HelpWorldEntry entry(kind, area);
    for (auto &e: entries)
        if (e == entry)
            return;
    entries.push_back(entry);
}

/** A name may be a Flexer pad: index its nominative, the form players type. */
static void add_name(const DLString &name, HelpWorldKind kind, AreaIndexData *area)
{
    if (name.empty())
        return;

    if (name.find('|') == DLString::npos)
        add_entry(names, name, kind, area);
    else
        add_entry(names, russian_case(name, '1'), kind, area);
}

static void add_multi_name(const XMLMultiString &name, HelpWorldKind kind, AreaIndexData *area)
{
    for (auto &n: name)
        add_name(n.second, kind, area);
}

static void add_keywords(const XMLMultiString &kw, HelpWorldKind kind, AreaIndexData *area)
{
    for (auto &k: kw) {
        DLString words = k.second;
        DLString word;
        while (!(word = words.getOneArgument()).empty())
            if (word.size() >= MIN_KEYWORD_LEN)
                add_entry(keywords, word, kind, area);
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
                add_keywords(m->keyword, HWK_MOB, m->area);
            }

        for (OBJ_INDEX_DATA *o = obj_index_hash[i]; o; o = o->next)
            if (help_world_area_listed(o->area)) {
                add_multi_name(o->short_descr, HWK_OBJ, o->area);
                add_keywords(o->keyword, HWK_OBJ, o->area);
            }
    }

    built = true;
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

    for (auto &e: i->second) {
        std::list<AreaIndexData *> &list = hits.areas[e.first];
        bool seen = false;
        for (auto a: list)
            if (a == e.second)
                seen = true;
        if (!seen)
            list.push_back(e.second);
    }

    return hits;
}
