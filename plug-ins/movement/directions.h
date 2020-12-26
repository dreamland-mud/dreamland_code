/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DIRECTIONS_H__
#define __DIRECTIONS_H__

class DLString;
class Character;
class Room;
struct exit_data;

struct direction_t {
    int door;
    int rev;
    const char * name;
    const char * rname;
    const char * leave;
    const char * enter;
    const char * where;
    const char * rname_extra_1;
    const char * rname_extra_2;
};

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
