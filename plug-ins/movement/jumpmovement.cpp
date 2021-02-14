/* $Id$
 *
 * ruffina, 2004
 */
#include "transfermovement.h"

#include "character.h"
#include "room.h"

JumpMovement::JumpMovement( Character *ch )
                     : Movement( ch )
{
    this->actor = ch;
}

JumpMovement::JumpMovement( Character *ch, Character *actor, Room *to_room )
                     : Movement( ch )
{
    this->actor = actor;
    this->to_room = to_room;
}

bool JumpMovement::moveAtomic( )
{
    if (!canMove( ch ))
        return false;

    if (ch->mount && !canMove( ch->mount ))
        return false;

    if (!tryMove( ch ))
        return false;

    if (ch->mount && !tryMove( ch->mount ))
        return false;

    place( ch );

    if (ch->mount)
        place( ch->mount );

    return true;
}

bool JumpMovement::findTargetRoom( )
{
    return to_room != NULL;
}

bool JumpMovement::canMove( Character *wch )
{
    return true;
}

bool JumpMovement::tryMove( Character *wch )
{
    return true;
}

void JumpMovement::moveFollowers( Character *wch )
{
}

void JumpMovement::msgRoomNoActor(Character *wch, const char *msg) 
{
    for (Character *rch = wch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != wch && rch != wch->mount && rch != actor)
            msgEcho( rch, wch, msg );    
}


