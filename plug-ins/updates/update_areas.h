/* $Id$
 *
 * ruffina, 2004
 */
#ifndef UPDATE_AREAS_H
#define UPDATE_AREAS_H

struct Area;
class Room;

#define FRESET_ALWAYS (A)

void reset_area( Area *pArea, int flags = 0 );
void area_update( int flags = 0 );
void reset_room( Room *pRoom, int flags = 0 );

#endif
