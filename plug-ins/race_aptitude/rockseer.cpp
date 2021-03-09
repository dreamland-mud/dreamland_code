/* $Id: rockseer.cpp,v 1.1.2.2 2008/03/04 07:24:12 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "skillmanager.h"
#include "spelltemplate.h"

#include "pcharacter.h"
#include "affect.h"
#include "dreamland.h"
#include "magic.h"
#include "handler.h"
#include "fight.h"
#include "merc.h"
#include "act.h"
#include "def.h"

SPELL_DECL(MeldIntoStone);
VOID_SPELL(MeldIntoStone)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if ( victim->isAffected(sn ) )
    {
      if (victim == ch)
               ch->pecho("Твоя кожа уже подобна камню.");
      else
               act("Кожа %2$C2 уже подобна камню.",ch,victim,0,TO_CHAR);
      return;
    }
  
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 5;
  af.location = APPLY_AC;
  af.modifier  = -100;
  af.bitvector.setTable(&res_flags);
  af.bitvector.setValue(RES_PIERCE | RES_SLASH);
  affect_to_char( victim, &af );

  af.bitvector.setTable(&affect_flags);
  af.location = APPLY_DEX;
  af.modifier  = -(1 + level / 20);
  af.bitvector.setValue(AFF_SLOW);
  affect_to_char( ch, &af );


  act("Кожа %C2 затвердевает, становясь подобной камню.",           victim, 0, 0,TO_ROOM);
  victim->pecho("Твоя кожа затвердевает, становясь подобной камню.");
  return;

}

