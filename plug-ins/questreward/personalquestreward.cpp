/* $Id: personalquestreward.cpp,v 1.1.2.10.6.3 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "personalquestreward.h"
#include "class.h"
#include "pcharacter.h"
#include "object.h"
#include "act.h"
#include "loadsave.h"

bool PersonalQuestReward::canEquip( Character *ch )
{
    if (!obj->hasOwner( ch )) {
        ch->pecho( "Ты не можешь владеть %1$O5 и бросаешь %1$P2.", obj );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}
    
void PersonalQuestReward::get( Character *ch ) 
{ 
    if (ch->is_immortal())
        return;
    
    if (!canEquip( ch ))
        return;

    act_p("{BМерцающая аура окружает $o4.\n\r{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
}


bool PersonalQuestReward::save( ) {
    Character *ch = obj->getCarrier( );

    if (!ch || ch->is_immortal( ))
        return false;
    
    if (obj->hasOwner( ch )) 
        return false;
    
    act_p("$o1 исчезает!", ch, obj, 0, TO_CHAR, POS_RESTING);
    extract_obj(obj);
    return true;
}

void PersonalQuestReward::delete_( Character *ch ) {
    if (obj->hasOwner( ch )) 
        extract_obj( obj );
}

bool PersonalQuestReward::isLevelAdaptive( ) {
   return true; 
}

bool PersonalQuestReward::canSteal( Character * ) { 
    return false;
}

