/* $Id: roombehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "roombehaviorplugin.h"
#include "character.h"
#include "room.h"
#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

void RoomBehaviorPlugin::initialization( ) 
{
    Room *room;

    for (room = room_list; room; room = room->rnext) {
        if (!room->behavior)
            continue;
        
        if (room->behavior->getType( ) == getName( )) {
            room->behavior.recover( );
            room->behavior->setRoom( room );        
        }
    }
}

void RoomBehaviorPlugin::destruction( ) 
{
    Room *room;

    /* XXX */
    if (dreamland->isShutdown( ))
        return;

    for (room = room_list; room; room = room->rnext) {
        if (!room->behavior)
            continue;
        
        if (room->behavior->getType( ) == getName( )) {
            room->behavior->unsetRoom( );
            room->behavior.backup( );
        }
    }
}
