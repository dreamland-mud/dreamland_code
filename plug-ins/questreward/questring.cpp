/* $Id: questring.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "questring.h"
#include "class.h"
#include "affect.h"
#include "pcharacter.h"
#include "object.h"
#include "profflags.h"
#include "act.h"
#include "merc.h"
#include "loadsave.h"
#include "def.h"

void QuestRing::wear( Character *ch ) 
{
    ch->send_to( "{CТвое кольцо ярко вспыхивает.{x\r\n" );
}

void QuestRing::equip( Character *ch ) {
    Affect *paf;
    Affect af;
    
    obj->level = ch->getRealLevel( );
    
    if( obj->affected ) {
        for( paf = obj->affected; paf; paf = paf->next )
            addAffect( ch, paf );
    }
    else {
        af.where = TO_OBJECT;
        af.type  = -1;
        af.duration = -1;
        af.bitvector = 0;

        af.location = APPLY_INT;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_WIS;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_AC;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_HIT;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_MANA;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_MOVE;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_HITROLL;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_DAMROLL;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );
    }
}

void QuestRing::addAffect( Character *ch, Affect *paf ) {
  short level = ch->getModifyLevel();

  switch( paf->location ) {
    case APPLY_DAMROLL:
      paf->level = level;
      paf->modifier = IS_EVIL( ch ) ? ( level / 5 ) :
                        IS_NEUTRAL( ch ) ? ( level / 10 ) : ( level / 20 );
      if (!ch->getTrueProfession( )->getFlags( ).isSet(PROF_CASTER)) // Bonus for battle clases
          paf->modifier += paf->modifier/2;
      return;
    case APPLY_HITROLL:
      paf->level = level;
      paf->modifier = IS_EVIL( ch ) ? ( level / 20 ) :
                        IS_NEUTRAL( ch ) ? ( level / 10 ) : ( level / 5 );
      if (!ch->getTrueProfession( )->getFlags( ).isSet(PROF_CASTER)) // Bonus for battle clases
          paf->modifier += paf->modifier/2;
      return;
    case APPLY_HIT:
      paf->level = level;
      paf->modifier = level * 3 / 2;
      return;
    case APPLY_MANA:
      paf->level = level;
      paf->modifier = level * 3 / 2;
      if (ch->getTrueProfession( )->getFlags( ).isSet(PROF_CASTER)) // Bonus for caster clases
           paf->modifier += paf->modifier*3/2;
      return;
    case APPLY_MOVE:
      paf->level = level;
      paf->modifier = level;
      return;
    case APPLY_INT:
      paf->level = level;
      paf->modifier = 1;
      return;
    case APPLY_WIS:
      paf->level = level;
      paf->modifier = 1;
      return;
    case APPLY_AC:
      paf->level = level;
      paf->modifier = IS_GOOD( ch ) ? -( level * 2 ) :
                        IS_EVIL( ch ) ? -( level ) : -( level * 3 / 2 );
      return;
  }

}


