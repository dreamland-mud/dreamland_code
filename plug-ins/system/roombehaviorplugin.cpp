/* $Id: roombehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "roombehaviorplugin.h"
#include "character.h"
#include "room.h"
#include "dreamland.h"
#include "merc.h"

#include "def.h"

void RoomBehaviorPlugin::initialization( ) 
{
    for (auto &room: roomInstances) {
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
    /* XXX */
    if (dreamland->isShutdown( ))
        return;

    for (auto &room: roomInstances) {
        if (!room->behavior)
            continue;
        
        if (room->behavior->getType( ) == getName( )) {
            room->behavior->unsetRoom( );
            room->behavior.backup( );
        }
    }
}
