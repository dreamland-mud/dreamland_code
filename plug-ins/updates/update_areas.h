/* $Id$
 *
 * ruffina, 2004
 */
#ifndef UPDATE_AREAS_H
#define UPDATE_AREAS_H

struct area_data;
class Room;

#define FRESET_ALWAYS (A)

void reset_area( area_data * );
void area_update( );
void reset_room( Room *pRoom, int flags = 0 );

#endif
