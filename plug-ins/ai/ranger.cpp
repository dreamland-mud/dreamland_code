/* $Id: ranger.cpp,v 1.1.2.3 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2007
 */
#include "basicmobilebehavior.h"

#include "npcharacter.h"
#include "room.h"
#include "affect.h"
#include "object.h"
#include "skillreference.h"

#include "dreamland.h"
#include "fight.h"
#include "magic.h"
#include "interp.h"
#include "act_move.h"
#include "handler.h"
#include "act.h"
#include "merc.h"

#include "def.h"

GSN(bow);
GSN(herbs);
GSN(plague);
GSN(poison);

/*----------------------------------------------------------------------------
 *                         RANGER BRAIN
 *----------------------------------------------------------------------------*/
bool BasicMobileBehavior::canAggressDistanceRanger( )
{
    Object *wield;
    
    if (!IS_SET(ch->act, ACT_RANGER))
        return false;

    if (gsn_bow->usable( ch )
        && ( wield = get_eq_char( ch, wear_wield ) )
        && wield->item_type == ITEM_WEAPON
        && wield->value0() == WEAPON_BOW)
    {
        return true;
    }
    else
        return false;
}

bool BasicMobileBehavior::aggressRanger( )
{
    Character *victim;
    int victDoor, victRange;
    
    if (!canAggressDistanceRanger( )) 
        return false;
    
    victim = findRangeVictim( ch->getModifyLevel( ) / 10 + 1, victDoor, victRange );

    if (!victim) 
        return false;
    
    oldact("$c1 пристально смотрит $T и натягивает тетиву $o2.",
         ch, get_eq_char( ch, wear_wield ), dirs[victDoor].leave, TO_ROOM);

    interpret_raw( ch, "shoot", "%s %s", 
                   dirs[victDoor].name, victim->getNameC() );    
    return true;
}

bool BasicMobileBehavior::healRanger( Character *patient )
{
    if (ch->isAffected(gsn_herbs))
        return false;

    if (patient->isAffected( gsn_poison )
        || patient->isAffected( gsn_plague )
        || number_percent( ) > HEALTH(patient))
    {
        interpret_raw( ch, "herbs", 
                       get_char_name_list( patient, ch->in_room->people, ch ).c_str( ) );
        return true;
    }

    return false;
}
