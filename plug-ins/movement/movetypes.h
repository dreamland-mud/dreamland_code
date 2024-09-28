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
    const char * enter;
    const char * leave;
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
};

int movetype_lookup( const char * );
int movetype_resolve( Character *, const char * );

#endif
