/* $Id: questweapon.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "questweapon.h"
#include "class.h"
#include "affect.h"
#include "character.h"
#include "object.h"
#include "profflags.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

void QuestWeapon::wear( Character *ch ) 
{
    ch->send_to("{CТвое оружие ярко вспыхивает.{x\r\n");
}

void QuestWeapon::equip( Character *ch ) 
{
  short level = ch->getModifyLevel();
  Affect *paf;
  Affect af;

    if ( level > 17 && level <= 30)	   obj->value[2] = 4;
    else if ( level > 30 && level <= 40)   obj->value[2] = 5;
    else if ( level > 40 && level <= 50)   obj->value[2] = 6;
    else if ( level > 50 && level <= 60)   obj->value[2] = 8;
    else if ( level > 60 && level <= 70)   obj->value[2] = 10;
    else if ( level > 70 && level <= 80)   obj->value[2] = 11;
    else obj->value[2] = 12;
    
    obj->level = ch->getRealLevel( );

    if( obj->affected )
      for( paf = obj->affected; paf; paf = paf->next )
        addAffect( ch, paf );
    else {
      af.where = TO_OBJECT;
      af.type  = -1;
      af.duration = -1;
      af.bitvector = 0;

      if( !IS_GOOD( ch ) ) {
        af.location = APPLY_STR;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );
      }

      if( !IS_EVIL( ch ) ) {
        af.location = APPLY_DEX;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );
      }

      af.location = APPLY_HITROLL;
      addAffect( ch, &af );
      affect_to_obj( obj, &af );

      af.location = APPLY_DAMROLL;
      addAffect( ch, &af );
      affect_to_obj( obj, &af );

	af.location = APPLY_HIT;
	addAffect( ch, &af );
	affect_to_obj( obj, &af );

	af.location = APPLY_MANA;
	addAffect( ch, &af );
	affect_to_obj( obj, &af );
    }
}

void QuestWeapon::addAffect( Character *ch, Affect *paf ) {
  short level = ch->getModifyLevel();

  switch( paf->location )
	{
    case APPLY_DAMROLL:
      paf->level = level;
      paf->modifier = IS_EVIL( ch ) ? ( level / 5 ) :
                        IS_NEUTRAL( ch ) ? ( level / 10 ) : ( level / 20 );
      return;
    case APPLY_HITROLL:
      paf->level = level;
      paf->modifier = IS_EVIL( ch ) ? ( level / 20 ) :
                        IS_NEUTRAL( ch ) ? ( level / 10 ) : ( level / 5 );
      return;
    case APPLY_STR:
      if( IS_GOOD( ch ) ) return;
      paf->level = level;
      paf->modifier = IS_EVIL( ch ) ? 2 : 1;
      return;
    case APPLY_DEX:
      if( IS_EVIL( ch ) ) return;
      paf->level = level;
      paf->modifier = IS_GOOD( ch ) ? 2 : 1;
      return;
    case APPLY_HIT:
      paf->level = level;
      paf->modifier = level * 2;
      return;
    case APPLY_MANA:
      paf->level = level;
      paf->modifier = level * 2;
      if (ch->getTrueProfession( )->getFlags( ).isSet(PROF_CASTER)) 
	  paf->modifier += paf->modifier*3/2;

      return;
  }
}


