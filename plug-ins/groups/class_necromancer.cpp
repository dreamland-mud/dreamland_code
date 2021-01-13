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
                ch->send_to("Ты не можешь сосредоточиться.. Жертва слишком быстро движется.\n\r");
                return;
        }

        ch->setWait( skill->getBeats( ) );

        act_p("Поток мрака, созданный тобой, окутывает $C4.",
                ch, 0, victim, TO_CHAR, POS_RESTING);
        act_p("$c1 создает поток мрака, окутывая $C4.",
                ch, 0, victim, TO_NOTVICT, POS_RESTING);
        act_p("$C1 создает поток мрака, окутывая тебя.",
                victim, 0, ch, TO_CHAR, POS_RESTING);

        if ( victim->is_immortal()
                || saves_spell(level,victim,DAM_MENTAL, ch, DAMF_SPELL)
                || number_percent () > 50 )
        {
                dam = dice( level , 24 ) ;
                damage_nocatch(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_SPELL);
                return;
        }

        victim->send_to("Тебя {RУБИЛИ{x!\n\r");

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
                ch->send_to("Это заклинание может использоваться только против игроков.\n\r");
                return;
        }

        if ( saves_spell( level, victim,DAM_OTHER, ch, DAMF_SPELL) )
        {
                ch->send_to("Не получилось...\n\r");        
                return;
        }

        if  (IS_AFFECTED(victim,AFF_BLOODTHIRST )) {
            act("$C1 уже жаждет крови!", ch, 0, victim, TO_CHAR);
            return;
        }

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = level / 10;
        af.bitvector.setValue(AFF_BLOODTHIRST);
        affect_to_char( victim, &af );
        victim->send_to("Безумие охватывает тебя!\n\r");
        act_p("Глаза $c2 наливаются кровью.",victim,0,0,TO_ROOM,POS_RESTING);
        return;

}

