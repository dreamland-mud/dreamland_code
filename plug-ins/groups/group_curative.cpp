
/* $Id: group_curative.cpp,v 1.1.2.10.6.6 2008/05/10 01:22:35 rufina Exp $
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
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "interp.h"
#include "handler.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"


SPELL_DECL(Awakening);
VOID_SPELL(Awakening)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect *paf, *paf_next;

    if (IS_AWAKE( victim )) {
	if (victim != ch)
	    act_p("$E уже не спит.", ch, 0, victim, TO_CHAR, POS_RESTING);
	else
	    ch->send_to("Что, не спится?\r\n");

	return;
    }

    for (paf = victim->affected; paf != NULL; paf = paf_next) {
        int chance;

        paf_next = paf->next;

        while (paf_next != NULL && paf_next->type == paf->type)
            paf_next = paf_next->next;

        chance = max( 5, 50 + 5 * (level - paf->level));

        if ( paf->bitvector == AFF_SLEEP && number_percent() <= chance )
            affect_strip( victim, paf->type );
    }

    if (IS_AFFECTED( victim, AFF_SLEEP )) {
	act_p("Тебе не удалось разбудить $C4.", ch, 0, victim, TO_CHAR, POS_RESTING);
	return;
    }

    interpret_raw( victim, "wake" );

}


SPELL_DECL(CureBlindness);
VOID_SPELL(CureBlindness)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    

    if ( !victim->isAffected(gsn_blindness ) )
    {
	if (victim == ch)
	  ch->send_to("Твое зрение в порядке.\n\r");
	else
	  act_p("Зрение $C2 в порядке.",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (!checkDispel(level,victim,gsn_blindness))
	ch->send_to("Твоя попытка закончилась неудачей.\n\r");

}



SPELL_DECL(CureDisease);
VOID_SPELL(CureDisease)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	if ( !victim->isAffected(gsn_plague ) )
	{
		if (victim == ch)
			act("Ты не бол$gьно|ен|ьна.",ch, 0, 0, TO_CHAR);
		else
			act( "$C1 совершенно здоро$Gво|в|ва.", ch,0,victim,TO_CHAR);

		return;
	}

	if (!checkDispel(level,victim,gsn_plague))
	    ch->send_to("Твоя попытка закончилась неудачей.\n\r");

}

SPELL_DECL(CurePoison);
VOID_SPELL(CurePoison)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	if ( !victim->isAffected(gsn_poison ) )
	{
		if (victim == ch)
			act("Ты не отравле$gно|н|на.", ch, 0, 0, TO_CHAR);
		else
			act("$C1 не отравле$Gно|н|на.", ch,0,victim,TO_CHAR);

		return;
	}

	if (!checkDispel(level,victim,gsn_poison))
	    ch->send_to("Твоя попытка закончилась неудачей.\n\r");

}

SPELL_DECL(RemoveCurse);
VOID_SPELL(RemoveCurse)::run( Character *ch, Object *obj, int sn, int level ) 
{
    if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	&&  !savesDispel(level + 2,obj->level,0))
	{
	    REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
	    REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
	    act("$o1 загорается голубым светом.", ch,obj,0,TO_ALL);
	    return;
	}

	act("Проклятие $o2 пока неподвластно твоим молитвам.", ch,obj,0,TO_CHAR);
	return;
    }
    else  {
      ch->send_to("Ничего не произошло...\n\r");
      return;
    }
}

VOID_SPELL(RemoveCurse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *obj;
    bool found = false;

    if (!checkDispel(level,victim,gsn_curse))
	    ch->send_to("Не получилось.\n\r");

   for (obj = victim->carrying; (obj != 0 && !found); obj = obj->next_content)
   {
	if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
	{   /* attempt to remove curse */
	    if (!savesDispel(level,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act_p("$o1 вспыхивает голубым светом.",
                       victim,obj,0,TO_CHAR,POS_RESTING);
		act_p("$o1 $c2 вспыхивает голубым светом.",
                       victim,obj,0,TO_ROOM,POS_RESTING);
		found = true;
	    }
	 }
    }

}

SPELL_DECL(RemoveFear);
VOID_SPELL(RemoveFear)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (!checkDispel(level,victim,gsn_fear))
	ch->send_to("Твоя попытка закончилась неудачей.\n\r");
}

