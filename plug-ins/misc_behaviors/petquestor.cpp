/* $Id$
 *
 * ruffina, 2004
 */
#include "petquestor.h"
#include "npcharacter.h"
#include "save.h"
#include "handler.h"
#include "def.h"

PetQuestor::PetQuestor( )
               : timer( -1 )
{
}

bool PetQuestor::specIdle( )
{
    if (!ch->master || ch->master->in_room != ch->in_room)
        return false;

    if (chance(99))
        return false;
    
    if (!msgIdleMaster.empty( ))
        ch->master->pecho( msgIdleMaster.c_str( ), ch, ch->master );

    if  (!msgIdleOther.empty( ))
        ch->recho( ch->master, msgIdleOther.c_str( ), ch, ch->master );

    if (!msgIdleRoom.empty( ))
        ch->recho( msgIdleRoom.c_str( ), ch, ch->master );
            
    return true;
}

void PetQuestor::stopfol( Character *master ) 
{
    Pet::stopfol( master );

    if (timer == -1) {
        timer = 5; 
        save_mobs( ch->in_room );
    }
}

bool PetQuestor::area( ) 
{
    if (timer == -1)
        return false;
    
    if (timer > 1) {
        timer = timer - 1;
        return false;
    }
    
    ch->recho( msgDisappear.c_str( ), ch );
    extract_char( ch );
    return true;
}

int PetQuestor::getOccupation( )
{
    return Questor::getOccupation( );
}
