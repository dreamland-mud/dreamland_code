#include "room.h"
#include "save.h"
#include "mercdb.h"
#include "def.h"

void debug_save_world( ) __attribute__ ((constructor));

void debug_save_world( )
{
    Room *r;

    for (r = room_list; r; r = r->rnext) {
        save_room_objects( r );
        save_room_mobiles( r );
    }
}


