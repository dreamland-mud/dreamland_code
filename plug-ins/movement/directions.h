/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DIRECTIONS_H__
#define __DIRECTIONS_H__

#include "lang.h"
#include "dlstring.h"

class Character;
class Room;
struct exit_data;

struct direction_t {
    int door;
    int rev;
    const char * name;      // EN (caseless; prepositions live in the frame)
    const char * rname;     // RU nominative     ("север")
    const char * leave;     // RU accusative/to  ("на север")
    const char * enter;     // RU genitive/from  ("с севера")
    const char * where;     // RU locative/at    ("на севере")
    const char * ua_rname;  // UA nominative     ("північ")
    const char * ua_leave;  // UA accusative/to  ("на північ")
    const char * ua_enter;  // UA genitive/from  ("з півночі")
    const char * ua_where;  // UA locative/at    ("на півночі")
    const char * rname_extra_1;
    const char * rname_extra_2;
};

// Grammatical case selector for direction_word().
enum dir_case_t {
    DIR_CASE_NOM = 0,  // nominative:      "север"   / "north" / "північ"
    DIR_CASE_TO,       // accusative (to): "на север"/ "north" / "на північ"
    DIR_CASE_FROM,     // genitive (from): "с севера"/ "north" / "з півночі"
    DIR_CASE_AT        // locative (at):   "на севере"/"north" / "на півночі"
};

/** Direction word in the viewer's language and the requested grammatical case.
 *  EN is caseless -- returns the bare name for every case, so EN callers must
 *  carry the preposition in the frame ("leaves %s" / "from the %s"). */
const char * direction_word(lang_t lang, int door, dir_case_t dcase);

/** Build a %w / $w act argument for `door` in case `dcase`, carrying the EN/RU/UA
 *  words so a TO_ROOM message resolves the direction per viewer. The returned
 *  struct's pointers reference static dirs[] storage, so it is safe to pass its
 *  address straight to a synchronous act/oldact call. */
LangText direction_langtext(int door, dir_case_t dcase);

extern const struct direction_t dirs [];

extern const char * extra_move_ru [];
extern const char * extra_move_rp [];
extern const char * extra_move_rt [];
extern char const extra_move_rtum [];
extern char const extra_move_rtpm [];


int direction_lookup( char c );
int direction_lookup( const char *arg );

/** Return short description of the exit, or generic "door" if not set. */
const char * direction_doorname(exit_data *);

/** Per-viewer door name for the %w / $w act code and single-viewer string
 *  building. Owns its three language forms; the RU form is pre-declined to a
 *  grammatical case (russian_case is viewer-independent), EN is caseless, UA
 *  degrades to nominative (no ukrainian_case yet). Generic fallback for a
 *  nameless door: door / дверь / двері. Keep the DoorName alive while any
 *  LangText from lt() is in use -- the LangText points into these strings. */
struct DoorName {
    DLString en, ru, ua;
    const DLString & forLang(lang_t lang) const {
        if (lang == LANG_EN && !en.empty()) return en;
        if (lang == LANG_UA && !ua.empty()) return ua;
        return ru;
    }
    LangText lt() const { return LangText { en.c_str(), ru.c_str(), ua.c_str() }; }
};

/** Build a DoorName for `pexit`, RU pre-declined to case `rcase` ('1' nom, '4'
 *  acc, ...). RU output matches the legacy russian_case(direction_doorname())
 *  path byte-for-byte; EN/UA are the exit's own short_descr forms (RU fallback
 *  when empty). */
DoorName direction_doorname_langtext(exit_data *pexit, char rcase);
/** Return corresponding exit from the opposite side, but only if it leads back here. */
exit_data *direction_reverse(Room *room, int door);
/** Return room this door leads to. */
Room * direction_target(Room *room, int door);

/** Split direction.victim arguments. */
bool direction_range_argument(const DLString &cargs, DLString &argDoor, DLString &argVict, int &door);

/** Return door connecting two rooms. */
int door_between_rooms(Room *src, Room *target);

#define FEX_NONE     (0)
#define FEX_NO_INVIS (A)
#define FEX_DOOR     (B)
#define FEX_NO_EMPTY (C)
#define FEX_VERBOSE  (D)
int find_exit( Character *ch, const char *arg, int flags );

#endif
