
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
    if (IS_AWAKE( victim )) {
        if (victim != ch)
            oldact("$E уже не спит.", ch, 0, victim, TO_CHAR);
        else
            ch->pecho("Может, лучше бахнуть кофейку?");

        return;
    }

    for (auto &paf: victim->affected.clone()) {
        int chance;

        chance = max( 5, 50 + 5 * (level - paf->level));

        if ( paf->bitvector == AFF_SLEEP && number_percent() <= chance )
            affect_strip( victim, paf->type );
    }

    if (IS_AFFECTED( victim, AFF_SLEEP )) {
        act("Тебе не удалось разбудить %2$C4.", ch, 0, victim, TO_CHAR);
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
          ch->pecho("Твое зрение в порядке.");
        else
          act("Зрение %2$C2 в порядке.",ch,0,victim,TO_CHAR);
        return;
    }

    if (!checkDispel(level,victim,gsn_blindness))
        ch->pecho("Заклинание слепоты, висящее на %C6, сопротивляется твоим молитвам.", victim);

}



SPELL_DECL(CureDisease);
VOID_SPELL(CureDisease)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if ( !victim->isAffected(gsn_plague ) )
        {
                if (victim == ch)
                        oldact("Ты не бол$gьно|ен|ьна.",ch, 0, 0, TO_CHAR);
                else
                        oldact("$C1 совершенно здоро$Gво|в|ва.", ch,0,victim,TO_CHAR);

                return;
        }

        if (!checkDispel(level,victim,gsn_plague))
            ch->pecho("Чума в теле %C2 сопротивляется твоим молитвам.", victim);

}

SPELL_DECL(CurePoison);
VOID_SPELL(CurePoison)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if ( !victim->isAffected(gsn_poison ) )
        {
                if (victim == ch)
                        oldact("Ты не отравле$gно|н|на.", ch, 0, 0, TO_CHAR);
                else
                        oldact("$C1 не отравле$Gно|н|на.", ch,0,victim,TO_CHAR);

                return;
        }

        if (!checkDispel(level,victim,gsn_poison))
            ch->pecho("Яд в крови %C2 сопротивляется твоим молитвам.", victim);

}

SPELL_DECL(RemoveCurse);
VOID_SPELL(RemoveCurse)::run( Character *ch, Object *obj, int sn, int level ) 
{
    if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
        if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {
            if (!savesDispel(level + 2,obj->level,0)) {
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                obj->getRoom()->echo( POS_RESTING, "%1$^O1 загора%1$nется|ются {1{Cголубым{2 светом!", obj );
                return;
            }
            else {
                ch->pecho("Проклятье, висящее на %O6, сопротивляется твоим молитвам.", obj);
                return;                
            }
        }
        ch->pecho("Проклятье, висящее на %O6, неподвластно твоим молитвам.", obj);
        return;
    }
    else  {
      ch->pecho("Эту молитву стоит использовать только на вещи, которые нельзя снять или бросить.");
      return;
    }
}

VOID_SPELL(RemoveCurse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *obj;
    bool found = false;

    if (!checkDispel(level,victim,gsn_curse)) {
        if (ch == victim)
            ch->pecho("Проклятье, висящее на тебе, сопротивляется твоим молитвам.");
        else
            ch->pecho("Проклятье, висящее на %C6, сопротивляется твоим молитвам.", victim);
    }

   for (obj = victim->carrying; (obj != 0 && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        &&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!savesDispel(level,obj->level,0))
            {
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                obj->getRoom()->echo( POS_RESTING, "%1$^O1 загора%1$nется|ются {1{Cголубым{2 светом!", obj );
                found = true;
            }
         }
    }

}

SPELL_DECL(RemoveFear);
VOID_SPELL(RemoveFear)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (!checkDispel(level,victim,gsn_fear))
        ch->pecho("Заклинание страха, висящее на %C6, сопротивляется твоим молитвам.", victim);
}

