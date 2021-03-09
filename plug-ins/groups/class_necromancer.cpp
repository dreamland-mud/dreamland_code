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
VOID_SPELL(PowerWordKill)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        int dam;

        if ( victim->fighting )
        {
                ch->pecho("Ты не можешь сосредоточиться.. Жертва слишком быстро движется.");
                return;
        }

        ch->setWait( skill->getBeats( ) );

        act("Поток мрака, созданный тобой, окутывает %2$C4.",  ch,  victim,  0, TO_CHAR);
        oldact("$c1 создает поток мрака, окутывая $C4.", ch, 0, victim, TO_NOTVICT);
        act("%2$^C1 создает поток мрака, окутывая тебя.",                 victim,  ch,  0, TO_CHAR);

        if ( victim->is_immortal()
                || saves_spell(level,victim,DAM_MENTAL, ch, DAMF_MAGIC)
                || number_percent () > 50 )
        {
                dam = dice( level , 24 ) ;
                damage_nocatch(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_MAGIC);
                return;
        }

        victim->pecho("Тебя {RУБИЛИ{x!");

        group_gain( ch, victim );
        raw_kill( victim, -1, ch, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
        pk_gain( ch, victim );
}



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
            act("%2$^C1 уже жаждет крови!", ch, victim, 0,TO_CHAR);
            return;
        }

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = level / 10;
        af.bitvector.setValue(AFF_BLOODTHIRST);
        affect_to_char( victim, &af );
        victim->pecho("Безумие охватывает тебя!");
        act("Глаза %C2 наливаются кровью.",victim,0,0,TO_ROOM);
        return;

}

