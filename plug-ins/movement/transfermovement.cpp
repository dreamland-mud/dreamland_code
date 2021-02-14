/* $Id$
 *
 * ruffina, 2004
 */
#include "transfermovement.h"
#include "move_utils.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "loadsave.h"
#include "descriptor.h"
#include "merc.h"
#include "def.h"

/*
 * TransferMovement
 */
TransferMovement::TransferMovement( Character *ch, Character *actor, Room *to_room,
                const char *mrl, const char *msl, const char *mre, const char *mse )
            : JumpMovement( ch, actor, to_room ),
              msgRoomLeave( mrl ), msgSelfLeave( msl ),
              msgRoomEnter( mre ), msgSelfEnter( mse )
            
{
}

bool TransferMovement::tryMove( Character *wch )
{
    check_camouflage( wch, to_room );
    undig( wch );
    return true;
}

void TransferMovement::msgEcho( Character *listener, Character *wch, const char *msg )
{
    if (msg && msg[0] && canHear( listener, wch ))
        listener->pecho( msg, wch, actor );
}

void TransferMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (wch) {
        if (fLeaving) {
            msgSelf( wch, msgSelfLeave );
            msgRoomNoActor( wch, msgRoomLeave );
        }
        else {
            msgSelf( wch, msgSelfEnter );
            msgRoomNoActor( wch, msgRoomEnter );
        }
    }
}

