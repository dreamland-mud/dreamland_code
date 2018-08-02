
/* $Id: group_enhancement.cpp,v 1.1.2.11.6.6 2010-09-01 21:20:45 rufina Exp $
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


#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"


SPELL_DECL(GiantStrength);
VOID_SPELL(GiantStrength)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) )
    {
	if (victim == ch)
	  ch->send_to("Ты не можешь быть еще сильнее!\n\r");
	else
	  act_p("$C1 не может быть еще сильнее.",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_AFFECTED(victim,AFF_WEAKEN))
    {
	if (checkDispel(level,victim, gsn_weaken))
	    return;
	
	if (victim != ch)
	    ch->send_to("Твоя попытка закончилась неудачей.\n\r");

	victim->send_to("Слабость проходит... но лишь на мгновение.\n\r");
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (10 + level / 3);
    af.location  = APPLY_STR;
    af.modifier  = max(2,level / 10);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->send_to("Ты становишься намного сильнее!\n\r");
    act_p("$c1 становится намного сильнее.",
           victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(Haste);
VOID_SPELL(Haste)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->isAffected(sn) || IS_QUICK(victim))
    {
	if (victim == ch)
	  ch->send_to("Ты не можешь двигаться быстрее, чем сейчас!\n\r");
	else
	  act_p("$C1 не может двигаться еще быстрее.",
	         ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (checkDispel(level,victim, gsn_slow))
	    return;
	
	if (victim != ch)
	    ch->send_to("Твоя попытка закончилась неудачей.\n\r");

	victim->send_to("Твои движения становятся быстрее... но лишь на мгновение.\n\r");
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = max(2,level / 12 );
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    victim->send_to("Твои движения становятся намного быстрее.\n\r");
    act_p("Движения $c2 становятся намного быстрее.",
           victim,0,0,TO_ROOM,POS_RESTING);
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}




SPELL_DECL(Infravision);
VOID_SPELL(Infravision)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  ch->send_to("Ты уже видишь в темноте.\n\r");
	else
	  act_p("$C1 уже видит в темноте.\n\r",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    act_p( "Глаза $c2 загораются красным светом.\n\r",
            victim, 0, 0, TO_ROOM,POS_RESTING);

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    victim->send_to("Твои глаза загораются красным светом.\n\r");
    return;

}

SPELL_DECL(Learning);
VOID_SPELL(Learning)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect	af;

    if ( victim->is_npc() )
    {
        ch->send_to("Ему это не поможет.\n\r");
        return;
    }

    if( victim->isAffected(gsn_learning) ) 
    {
        if (victim == ch)
            ch->send_to("Куда уж больше.\n\r");
        else
            act_p("$C1 уже учится.\n\r", ch,0,victim,TO_CHAR,POS_RESTING);
        return;
  }

  af.where	= TO_AFFECTS;
  af.type	= sn;
  af.level	= level;
  af.duration	= level / 10 + 1;
  af.location	= APPLY_NONE;
  af.modifier	= 0;
  af.bitvector	= 0;
  affect_to_char( victim, &af );
    
  victim->send_to("Ты концентрируешься на учебе.\n\r");

  if (ch != victim)
      act_p("$C1 будет учиться лучше!\n\r", ch,0,victim,TO_CHAR,POS_RESTING);

}
