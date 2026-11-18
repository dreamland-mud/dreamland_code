#ifndef __OLCVALIDATE_H__
#define __OLCVALIDATE_H__

#include <vector>
#include "dlstring.h"

struct RoomIndexData;
struct mob_index_data;
struct obj_index_data;
struct AreaIndexData;

/**
 * OLC validators (docs/plans/OLC_REVAMP.md 3.1): a chain of pure reads over a
 * prototype, run after an /api/olc batch on everything it touched and by the
 * read-only 'validate' action. They never change a prototype and never look up
 * a vnum below 1.
 *
 * Every finding has a base severity: "warning" (a real defect) or "info" (style,
 * reported only on request). The batch's policy profile turns warnings into
 * errors (OlcOps::apply); the validators do not know who asked.
 *
 * Checks (check id: what it reports):
 *   structure.vnum_range    prototype vnum outside its area's range
 *   structure.exit          an exit or extra exit leads to a missing room
 *   structure.reset         a reset names a missing mob, object or room
 *   structure.keyword       mob/obj without a keyword in any language, an ED
 *                           with an empty keyword, or one with no word for a
 *                           language its description has (no Latin word with an
 *                           EN text, no Cyrillic word with a RU/UA one)
 *   structure.keyword_lang  a door or extra exit keyword filled in some
 *                           languages and empty in others
 *   structure.kw_collision  a room ED and an extra exit share a keyword word:
 *                           'look' can reach only one of them
 *   structure.eexit_msg     extra exit movement messages (scripts/lint-extra-exits.py):
 *                           %4 with no noun, a case the noun pad lacks, an
 *                           unnumbered placeholder, a trailing full stop, a RU
 *                           reflexive ending in UA
 *   text.charset            a byte that is neither ASCII nor a KOI8 letter
 *   text.mixed_script       Latin and Cyrillic letters in one word (rule of
 *                           scripts/lint-mixed-script.py, same tag stripping and
 *                           declined-name allowlist)
 *   text.lang               RU text present, EN or UA missing (name, short, long, desc)
 *   text.colour             a colour opened and never reset with {x
 *   text.length             visible length over the cap (kOlcCap* below)
 *   text.kw_in_long         a mob long / obj ground line holds none of its own
 *                           keywords in that language (scripts/find-longs-without-keyword.py)
 *   text.dash               info: a spaced single hyphen, house style is " -- "
 *   power.measure           object M (item_measure_cm) above the legendary tier
 *                           window for its level (weapon_tiers.json x the loot
 *                           affix curve); weapons and random items are skipped
 *   power.spell_price       a granted skill or a charged spell that
 *                           fight/spell_value.json does not price
 *   power.mob_numbers       mob hit/mana/damage/hitroll/ac differ from what
 *                           fight/mob_tiers.json derives: a hand edit that the
 *                           next rebuild or boot discards
 */
struct OlcFinding {
    DLString severity;   // error | warning | info
    DLString check;
    DLString kind;       // room | mob | obj | area
    int vnum;
    DLString field;
    DLString lang;
    DLString message;    // KOI8, like every engine string
    bool audit;          // set by the caller: a 'validate' op named this prototype

    OlcFinding() : vnum(0), audit(false) { }
};

typedef std::vector<OlcFinding> OlcFindings;

void olc_validate_room(RoomIndexData *pRoom, OlcFindings &out);
void olc_validate_mob(mob_index_data *pMob, OlcFindings &out);
void olc_validate_obj(obj_index_data *pObj, OlcFindings &out);
/** The area header only. */
void olc_validate_area(AreaIndexData *pArea, OlcFindings &out);
/** The header and every room, mob and object prototype of the area. */
void olc_validate_area_all(AreaIndexData *pArea, OlcFindings &out);

/** A resolved editor subcommand that sets a numeric or power field. Unreachable
 *  while OlcOps::apply refuses the player profile; phase 5 replaces this
 *  blacklist with a whitelist. */
bool olc_power_field(const DLString &cmdName);

#endif
