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
 * SilentTransferMovement
 */
SilentTransferMovement::SilentTransferMovement(Character *ch, Room *to_room)
              : TransferMovement(ch, 0, to_room, 0, 0, 0, 0)
{
    this->doLook = false;
}

/*
 * TransferMovement
 */
TransferMovement::TransferMovement( Character *ch, Character *actor, Room *to_room,
                const char *mrl, const char *msl, const char *mre, const char *mse )
            : JumpMovement( ch, actor, to_room ),
              msgRoomLeave( mrl ), msgSelfLeave( msl ),
              msgRoomEnter( mre ), msgSelfEnter( mse ),
              multiLang( false )
{
}

TransferMovement::TransferMovement( Character *ch, Character *actor, Room *to_room,
                const MultiMessage &mrl, const MultiMessage &msl,
                const MultiMessage &mre, const MultiMessage &mse )
            : JumpMovement( ch, actor, to_room ),
              msgRoomLeave( 0 ), msgSelfLeave( 0 ),
              msgRoomEnter( 0 ), msgSelfEnter( 0 ),
              multiLang( true ),
              mmRoomLeave( mrl ), mmSelfLeave( msl ),
              mmRoomEnter( mre ), mmSelfEnter( mse )
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
    if (!wch)
        return;

    // Trilingual path: render each line in every viewer's own display language.
    // The mover reads the self line in theirs; every other watcher in the room
    // (minus actor/mount) reads the room line in theirs -- mirrors msgSelfRoom.
    if (multiLang) {
        const MultiMessage &selfMsg = fLeaving ? mmSelfLeave : mmSelfEnter;
        const MultiMessage &roomMsg = fLeaving ? mmRoomLeave : mmRoomEnter;

        if (!selfMsg.empty() && canHear( wch, wch ))
            wch->pecho( selfMsg, wch, actor );

        if (!roomMsg.empty())
            for (Character *rch = wch->in_room->people; rch; rch = rch->next_in_room)
                if (rch != wch && rch != wch->mount && rch != actor && canHear( rch, wch ))
                    rch->pecho( roomMsg, wch, actor );

        return;
    }

    if (fLeaving) {
        msgSelf( wch, msgSelfLeave );
        msgRoomNoActor( wch, msgRoomLeave );
    }
    else {
        msgSelf( wch, msgSelfEnter );
        msgRoomNoActor( wch, msgRoomEnter );
    }
}

