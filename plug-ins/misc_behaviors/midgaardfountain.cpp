/* $Id: midgaardfountain.cpp,v 1.1.2.5.22.2 2009/01/17 23:36:51 rufina Exp $
 *
 * ruffina, 2004
 */

#include "midgaardfountain.h"

#include "class.h"

#include "profiler.h"
#include "room.h"
#include "object.h"
#include "liquid.h"

#include "merc.h"
#include "act.h"
#include "vnum.h"
#include "def.h"
#include "l10n.h"

LIQ(blood);

bool MidgaardFountain::area( ) { 
    ProfilerBlock profiler("MidgaardFountain::area", 5);
    
    Character *wch;
    int count = 0;

    // Only monument instances matter here -- walk the prototype's instance list
    // rather than the whole object_list. This scan runs inside obj_update once per
    // tick, and object_list is ~60k while the monuments number a handful.
    obj_index_data *monumentIndex = get_obj_index(OBJ_VNUM_MONUMENT);
    if (monumentIndex)
        for (auto &o: monumentIndex->instances)
            if (o->in_room
                && o->in_room->area == obj->in_room->area)
            {
                count++;
            }

    wch = obj->in_room->people;

    if (count < 3) {
        if (obj->value2() != obj->pIndexData->value[2] && obj->value2() == liq_blood) {
            obj->value2(obj->pIndexData->value[2]);

            if (wch)
                oldact(_("Кровь в $o6 снова превращается в воду.."), wch, obj, 0, TO_ALL);
        }   
        return false;
    }

    if (obj->value2() == obj->pIndexData->value[2]) {
        obj->value2(liq_blood);
        
        if (wch) 
            oldact(_("Вода в $o6 медленно окрашивается {rкрасным{x."), wch, obj, 0, TO_ALL);
    }

    return false;
}

