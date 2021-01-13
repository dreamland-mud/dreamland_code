
/* $Id: group_meditation.cpp,v 1.1.2.7.6.1 2008/03/04 07:24:12 rufina Exp $
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
#include "spelltemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"

#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

SPELL_DECL(Link);
VOID_SPELL(Link)::run( Character *ch, Character *victim, int sn, int level )
{
    int random = number_percent( );

    if (victim == ch)
    {
        ch->send_to("Это заклинание имеет смысл применять только на других.\n\r");
        return;
    }
    
    ch->endur /= 2;
    victim->mana = victim->mana + (ch->mana / 2 + random) / 2;
    ch->mana = 0;

    ch->pecho("Арка магической энергии возникает между тобой и %C5.", victim);
    victim->pecho("Арка магической энергии возникает между тобой и %C5.", ch);
    ch->recho(victim, "Арка магической энергии возникает между %C5 и %C5.", ch, victim);
}

SPELL_DECL(MagicConcentrate);
VOID_SPELL(MagicConcentrate)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->isAffected(sn)) {
      act("Ты уже достаточно сконцентриров$gано|ан|ана.", ch, 0, 0, TO_CHAR);
      return;
  }

  af.type               = sn;
  af.level              = level;
  af.duration           = 7;
  affect_to_char(ch,&af);

  ch->send_to("Ты чувствуешь, как сверхмощная способность к разрушению заполняет все твое тело.\n\r");
}

