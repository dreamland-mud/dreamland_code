/* World name index for help misses -- help revamp Phase 4b.
 *
 * Players often type into 'help' the name of something they just saw: a room,
 * a creature, an item, a zone. No article answers that, but the world knows
 * where the thing lives, so a miss can still point at the right zone.
 *
 * The index is built on the first help miss and kept until the plugin is
 * unloaded: area files only load at boot, so it cannot go stale in between,
 * and a normal 'help' never pays for it.
 */
#ifndef HELPWORLDINDEX_H
#define HELPWORLDINDEX_H

#include <list>
#include "dlstring.h"

struct AreaIndexData;

enum HelpWorldKind {
    HWK_AREA = 0,
    HWK_ROOM,
    HWK_MOB,
    HWK_OBJ,
    HWK_MAX
};

struct HelpWorldHits {
    /** Distinct zones per kind, in area load order. */
    std::list<AreaIndexData *> areas[HWK_MAX];

    /** True when nothing matched a whole name and the hits come from single
     *  mob/object keywords, a weaker signal worth saying so. */
    bool keywordOnly;

    HelpWorldHits() : keywordOnly(false) { }

    bool empty() const;
};

/** Look the query up among zone, room, mob and object names in all languages;
 *  with 'keywords', fall back to single mob and object keywords. */
HelpWorldHits help_world_lookup(const DLString &query, bool keywords);

/** Lowercase, colour-free, single-spaced, quotes and a leading English article
 *  removed: the form both the index keys and the query are reduced to. */
DLString help_world_normalize(const DLString &str);

#endif
