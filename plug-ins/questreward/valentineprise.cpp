/* $Id: valentineprise.cpp,v 1.1.2.12.6.1 2007/09/11 00:31:25 rufina Exp $
 *
 * ruffina, 2004
 */

#include "valentineprise.h"
#include "class.h"
#include "affect.h"
#include "character.h"
#include "object.h"
#include "act.h"
#include "loadsave.h"

void ValentinePrise::wear( Character *ch ) 
{
    if (msgChar.getValue( ).empty( ))
        oldact("{CАура {Rлюбви{C окружает тебя.{x", ch, obj, 0, TO_CHAR);
    else
        oldact(msgChar.getValue( ).c_str( ), ch, obj, 0, TO_CHAR);

    if (msgRoom.getValue( ).empty( ))
        act("{CАура {Rлюбви{C окружает %C4.{x", ch, obj, 0, TO_ROOM);
    else
        oldact(msgRoom.getValue( ).c_str( ), ch, obj, 0, TO_ROOM);


}

void ValentinePrise::equip( Character *ch ) 
{ 
    int level;
    Affect af;
    std::vector<int> locs; 

    obj->level = 1;
    level = ch->getModifyLevel( );
   
    af.type  = -1;
    af.duration = -1;

    locs.push_back( APPLY_AC );
    locs.push_back( APPLY_CHA );
    locs.push_back( APPLY_HIT );
    locs.push_back( APPLY_MANA );
    locs.push_back( APPLY_MOVE );
    locs.push_back( APPLY_HITROLL );
    locs.push_back( APPLY_DAMROLL );
    locs.push_back( APPLY_SAVES );

    for (unsigned int i = 0; i < locs.size( ); i++) {
        Affect *paf = obj->affected.find(af.type, locs[i]);
        if (paf) {
            addAffect( paf, level );
        } else {
            af.location = locs[i];
            addAffect( &af, level );
            affect_to_obj( obj, &af );
        }
    }
}

void ValentinePrise::addAffect( Affect *af, int level ) {
    af->level = level;
    
    switch (af->location) {
    case APPLY_CHA:
        af->modifier = 2;
        break;
    case APPLY_AC:
        af->modifier = -level;
        break;
    case APPLY_HIT:
        af->modifier = level;
        break;
    case APPLY_MANA:
        af->modifier = level;
        break;
    case APPLY_MOVE:
        af->modifier = level;
        break;
    case APPLY_HITROLL:
        af->modifier = std::max( 1, level / 10 ); 
        break;
    case APPLY_DAMROLL:
        af->modifier = std::max( 1, level / 10 ); 
        break;
    case APPLY_SAVES:
        af->modifier = - std::max( 1, level / 10 );
        break;
    }
}


