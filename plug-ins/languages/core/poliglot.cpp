/* $Id: poliglot.cpp,v 1.1.2.4 2008/04/04 20:04:56 rufina Exp $
 *
 * ruffina, 2005
 */

#include "npcharacter.h"
#include "skillreference.h"
#include "skill.h"
#include "loadsave.h"
#include "move_utils.h"
#include "exitsmovement.h"
#include "magic.h"
#include "occupations.h"

#include "poliglot.h"

GSN(fly);
GSN(pass_door);

Poliglot::Poliglot( ) 
{
}

static bool room_has_exits(Room *room)
{
    for (int door = 0; door < DIR_SOMEWHERE; door++)
        if (direction_target(room, door) != NULL)
            return true;

    return false;
}

bool Poliglot::specIdle( ) 
{
    if (number_range( 0, 900 ) == 0 || !room_has_exits(ch->in_room)) {
        Room *room = get_random_room( ch );

        if (!room)
            return false;
        
        transfer_char( ch, ch, room );
        return true;
    }
    
    if (number_range( 0, 15 ))
        return false;
    
    if (path.empty( ))
        pathWithDepth( ch->in_room, 20, 5000 );

    makeOneStep( );
    return true;
}

void Poliglot::speech( Character *victim, const char *speech ) 
{
}

int Poliglot::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_PRACTICER);
}

bool Poliglot::handleMoveResult( Road &road, int rc )
{
    switch (rc) {
    case RC_MOVE_WATER:
    case RC_MOVE_AIR:
        ::spell( gsn_fly, ch->getModifyLevel( ), ch, ch, FSPELL_VERBOSE );
        return false;
    
    case RC_MOVE_PASS_NEEDED:
        ::spell( gsn_pass_door, ch->getModifyLevel( ), ch, ch, FSPELL_VERBOSE );
        return false;
    }

    return Wanderer::handleMoveResult( road, rc );
}

bool Poliglot::isHomesick( )
{
    return false;
}

