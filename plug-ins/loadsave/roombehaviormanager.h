/* $Id: roombehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef ROOMBEHAVIORMANAGER_H
#define ROOMBEHAVIORMANAGER_H

#include <stdio.h>

class RoomIndexData;
class Room;

class RoomBehaviorManager {
public:        
        static void parse( RoomIndexData *, FILE * );
        static void save( const RoomIndexData *, FILE * );
        static void assign(Room *);
};

#endif
