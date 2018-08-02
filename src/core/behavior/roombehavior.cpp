/* $Id$
 *
 * ruffina, 2004
 */
#include "roombehavior.h"

#include "room.h"

template class XMLStub<RoomBehavior>;

const DLString RoomBehavior::NODE_NAME = "behavior";

RoomBehavior::RoomBehavior( ) : room( NULL )
{
}

RoomBehavior::~RoomBehavior( )
{
}

void RoomBehavior::setRoom( Room *room )
{
    this->room = room;
}

void RoomBehavior::unsetRoom( )
{
}

Room * RoomBehavior::getRoom( )
{
    return room;
}

bool RoomBehavior::command( Character *, const DLString &, const DLString & ) 
{
    return false;
}

bool RoomBehavior::isCommon( )
{
    return true;
}

bool RoomBehavior::canEnter( Character * )
{
    return true;
}

