/* $Id$
 *
 * ruffina, 2004
 */

#include "xmlroom.h"
#include "xmlobjectfactory.h"
#include "xmlmobilefactory.h"
#include "merc.h"

#include "olc.h"
#include "loadsave.h"
#include "comm.h"
#include "onlinecreation.h"
#include "olcstate.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"

#include "def.h"

void
dump_obj(Character *ch, OBJ_INDEX_DATA *o)
{
    XMLStreamableBase<XMLObjectFactory> it("object");
    ostringstream os;
    
    it.init(o);
    it.toStream(os);

    page_to_char(os.str( ).c_str( ), ch);
}

void
dump_mob(Character *ch, MOB_INDEX_DATA *m)
{
    XMLStreamableBase<XMLMobileFactory> it("mobile");
    ostringstream os;
    
    it.init(m);
    it.toStream(os);

    page_to_char(os.str( ).c_str( ), ch);
}

void
dump_room(Character *ch, RoomIndexData *r)
{
    XMLStreamableBase<XMLRoom> it("room");
    ostringstream os;
    
    it.init(r);
    it.toStream(os);

    page_to_char(os.str( ).c_str( ), ch);
}

CMD(olcdump, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
    "dump area items")
{
    DLString constArguments = argument;

    if(constArguments.empty()) {
        dump_room(ch, ch->in_room->pIndexData);
        return;
    }
        
    const Object *o = get_obj_here(ch, constArguments.c_str( ));
    if(o) {
        dump_obj(ch, o->pIndexData);
        return;
    }

    const Character *c = get_char_room(ch, constArguments);
    if(c && c->is_npc( )) {
        dump_mob(ch, c->getNPC( )->pIndexData);
        return;
    }
    
    ch->pecho("nothng here like that");
}


