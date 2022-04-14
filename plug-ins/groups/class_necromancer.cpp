/* $Id$
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

#include "playerattributes.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "handler.h"
#include "effects.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "interp.h"
#include "def.h"



SPELL_DECL(PowerWordKill);


SPELL_DECL(Insanity);
VOID_SPELL(Insanity)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
        Affect af;

        if ( victim->is_npc() )
        {
                ch->pecho("Это заклинание может использоваться только против игроков.");
                return;
        }

        if ( saves_spell( level, victim,DAM_OTHER, ch, DAMF_MAGIC) )
        {
                ch->pecho("Не получилось...");        
                return;
        }

        if  (IS_AFFECTED(victim,AFF_BLOODTHIRST )) {
            oldact("$C1 уже жаждет крови!", ch, 0, victim, TO_CHAR);
            return;
        }

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = level / 10;
        af.bitvector.setValue(AFF_BLOODTHIRST);
        affect_to_char( victim, &af );
        victim->pecho("Безумие охватывает тебя!");
        oldact("Глаза $c2 наливаются кровью.",victim,0,0,TO_ROOM);
        return;

}

