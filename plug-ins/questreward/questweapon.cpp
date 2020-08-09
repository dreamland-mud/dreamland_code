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
#include "weapons.h"
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
    bitnumber_t wclass = obj->value0();
    const int tier = 2;

    obj->value1(
        weapon_value1(level, tier, wclass));
    obj->value2(
        weapon_value2(wclass));

    obj->level = ch->getRealLevel( );

    if( obj->affected )
      for(Affect *paf = obj->affected; paf; paf = paf->next)
        addAffect( ch, paf );
    else {
      Affect af;

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
      if (ch->getProfession( )->getFlags( ).isSet(PROF_CASTER)) 
          paf->modifier += paf->modifier*3/2;

      return;
  }
}


