/* $Id: group_detection.cpp,v 1.1.2.22.6.20 2010-09-01 21:20:45 rufina Exp $
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
#include "skillcommandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "char.h"
#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"
#include "affect.h"
#include "liquid.h"
#include "magic.h"
#include "fight.h"
#include "material.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "../anatolia/handler.h"
#include "comm.h"
#include "math_utils.h"
#include "recipeflags.h"
#include "act_move.h"
#include "act_lock.h"
#include "attacks.h"
#include "occupations.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"
#include "skill_utils.h"

GSN(none);
DLString quality_percent( int ); /* XXX */

/* From act_info.cpp */
void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName );
void lore_fmt_affect( Object *obj, Affect *paf, ostringstream &buf );

SPELL_DECL(DetectPoison);
VOID_SPELL(DetectPoison)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
        if (IS_SET(obj->value3(), DRINK_POISONED))
            ch->pecho("Ты чувствуешь запах яда!");
        else
            ch->pecho("Уф, кажется, не отравлено.");
    }
    else
    {
        ch->pecho("Это заклинание подействует только на пищу или емкости для жидкости.");
    }

    return;

}


SPELL_DECL(KnowAlignment);
VOID_SPELL(KnowAlignment)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
        const char *msg;

        if ( IS_GOOD(victim) )
                msg = "$C1 имеет светлую и чистую ауру.";
        else if ( IS_NEUTRAL(victim) )
                msg = "$C1 имеет серую ауру.";
        else
                msg = "$C1 -- воплощение {Dзла{x!.";

        oldact( msg, ch, 0, victim, TO_CHAR);

        if (!victim->is_npc())
        {
                switch (victim->ethos.getValue( )) {
                case ETHOS_LAWFUL:
                        msg = "$C1 чтит закон.";
                        break;
                case ETHOS_NEUTRAL:
                        msg = "$C1 относится к закону нейтрально.";
                        break;
                case ETHOS_CHAOTIC:
                        msg = "$C1 имеет хаотический этос.";
                        break;
                default:
                        msg = "$C1 понятия не имеет, как относиться к законам.";
                        break;
                }
                oldact( msg, ch, 0, victim, TO_CHAR);
        }
        return;

}


