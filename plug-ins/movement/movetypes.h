/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MOVETYPES_H__
#define __MOVETYPES_H__

class Character;

struct movetype_t {
    int type;
    int danger;
    int wait;
    bool sneak;
    const char * name;
    const char * rname;
    const char * uaname;
    const char * enter;      // RU arrival verb (gender-inflected via %1$G)
    const char * leave;      // RU departure verb
    const char * enter_en;   // EN arrival verb (no gender)
    const char * leave_en;   // EN departure verb
    const char * enter_ua;   // UA arrival verb (gender-inflected via %1$G)
    const char * leave_ua;   // UA departure verb
};

extern const struct movetype_t movetypes [];

enum {
    MOVETYPE_MORESAFE = 0,
    MOVETYPE_SAFE,        
    MOVETYPE_NORMAL,                        
    MOVETYPE_DANGEROUS,                        
    MOVETYPE_DEATH,
};

extern const char * movedanger_names [];

enum {
    MOVETYPE_SWIMMING = 0,
    MOVETYPE_WATER_WALK,
    MOVETYPE_SLINK,
    MOVETYPE_CRAWL,
    MOVETYPE_WALK,
    MOVETYPE_QUICKLY,
    MOVETYPE_RUNNING,
    MOVETYPE_FLEE,
    MOVETYPE_RIDING,
    MOVETYPE_FLYING,
    // Text-only rows (mob reform, decisions 30, 47, 50): never chosen as a
    // way of moving, only substituted into the arrive/leave message. Each
    // copies its base row's danger, wait and sneak; the mechanics and every
    // Fenia hook keep seeing the base row.
    MOVETYPE_TEXT_FIRST,
    MOVETYPE_GALLOPING = MOVETYPE_TEXT_FIRST,
    MOVETYPE_CLATTERING,
    MOVETYPE_TROTTING,
    MOVETYPE_CRAWLING,
    MOVETYPE_SLITHERING,
    MOVETYPE_OOZING,
    MOVETYPE_HOPPING,
};

/** Text-only row by name ("galloping"), -1 if there is none. */
int movetype_text_lookup( const char * );

int movetype_lookup( const char * );
int movetype_resolve( Character *, const char * );

#endif
