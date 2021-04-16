/* $Id: group_defensive.cpp,v 1.1.2.13.6.6 2010-09-01 21:20:45 rufina Exp $
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "skill_utils.h"
#include "handler.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "def.h"


/*
 * 'rescue' skill command
 */

SKILL_RUNP( rescue )
{
        char arg[MAX_INPUT_LENGTH];
        Character *victim;
        Character *fch;

        one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
                ch->pecho("Спасти кого?");
                return;
        }

        if ( ( victim = get_char_room( ch, arg ) ) == 0 )
        {
                ch->pecho("Таких нет здесь.");
                return;
        }

        if ( victim == ch )
        {
                ch->pecho("Себя?");
                return;
        }

        if ( ch->fighting == victim )
        {
                ch->pecho("Слишком поздно...");
                return;
        }

        if ( ( ( fch = victim->fighting ) == 0 )
                && ( victim->death_ground_delay == 0 ) )
        {
                ch->pecho("Но никто не нуждается в помощи..");
                return;
        }

        if (fch && is_safe_nomessage(ch, fch) ) {
                ch->pecho( "Ты не можешь вступить в бой против %C2. Вы вне ПК.", fch);
                return;
        }

        if ((!victim->is_npc( ) || (victim->is_npc( ) && victim->master != 0 ) ) && !is_same_group(ch, victim) && fch) {
                ch->pecho("Вы не в одной группе!");
                return;
        }

        ch->setWait( gsn_rescue->getBeats(ch)  );

        if ( ( number_percent( ) > gsn_rescue->getEffective( ch ) )
                || ( victim->getModifyLevel() > ( skill_level(*gsn_rescue, ch) + 30) )
                || ( ( fch == 0  ) && victim->trap.isSet( TF_NO_RESCUE ) ) )
        {
                ch->pecho("Твоя попытка спасти не удалась.");
                gsn_rescue->improve( ch, false, victim );
                return;
        }

        oldact("Ты спасаешь $C4!",  ch, 0, victim, TO_CHAR);
        oldact("$c1 спасает тебя!", ch, 0, victim, TO_VICT);
        oldact("$c1 спасает $C4!",  ch, 0, victim, TO_NOTVICT);
        gsn_rescue->improve( ch, true, victim );

        if ( fch )
        {
                stop_fighting( fch, false );
//                stop_fighting( victim, false );

                set_fighting( ch, fch );
                set_fighting( fch, ch );
        }
        else
        {
                victim->death_ground_delay = 0;
                victim->trap.clear( );
        }
        return;
}

