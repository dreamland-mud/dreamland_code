/* $Id: objects.cpp,v 1.1.2.3.6.2 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "objects.h"
#include "gangmob.h"
#include "gangsters.h"
#include "gangstersinfo.h"

#include "class.h"

#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "act.h"
#include "def.h"

/* GangKey */

void GangKey::get( Character *ch ) 
{
    Gangsters *gq = Gangsters::getThis( );

    if (ch->is_immortal( ) || gq->isLevelOK( ch ))
        return;

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    oldact("Почему-то тебе кажется, что Боги этого не одобрят.\r\n"
         "Пораженн$gое|ый|ая этой мыслью, ты роняешь $o4.", ch, obj, 0, TO_CHAR);
    act("%1$^C1 с озадаченным видом роняет %3$C4.", ch, 0, obj,TO_ROOM);
}

bool GangKey::extract( bool count )
{
    Gangsters *gq = Gangsters::getThis( );
    
    if (gq && needsReset) 
        gq->resetKeys( );
    
    ObjectBehavior::extract( count );
    return false;
}

bool GangKey::canSteal( Character *ch ) 
{ 
    Gangsters *gq = Gangsters::getThis( );

    if (ch->is_immortal( ) || gq->isLevelOK( ch ))
        return true;

    return false;
}

/*
 * GangPortal
 */

bool GangPortal::canDrop( Room *pRoomIndex ) 
{
    if (!Gangsters::checkRoom( pRoomIndex ))
        return false;
    
    if (pRoomIndex->exit[DIR_DOWN])
        return false;
            
    if (IS_SET(pRoomIndex->room_flags, ROOM_INDOORS)
        || pRoomIndex->getSectorType() == SECT_INSIDE)
        return false;

    if (!Gangsters::recursiveWalk( pRoomIndex, 0, 4 )) 
        return false;
        
    return true;
}


