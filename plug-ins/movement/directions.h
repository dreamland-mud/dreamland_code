/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DIRECTIONS_H__
#define __DIRECTIONS_H__

class Character;
struct extra_exit_data;

struct direction_t {
    int door;
    int rev;
    const char * name;
    const char * rname;
    const char * leave;
    const char * enter;
    const char * where;
};

extern const struct direction_t dirs [];

extern const char * extra_move_ru [];
extern const char * extra_move_rp [];
extern const char * extra_move_rt [];
extern char const extra_move_rtum [];
extern char const extra_move_rtpm [];


int direction_lookup( char c );
int direction_lookup( const char *arg );

#define FEX_NONE     (0)
#define FEX_NO_INVIS (A)
#define FEX_DOOR     (B)
#define FEX_NO_EMPTY (C)
#define FEX_VERBOSE  (D)
int find_exit( Character *ch, const char *arg, int flags );

extra_exit_data * get_extra_exit ( const char * name, extra_exit_data * list);

#endif
