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
                ch->send_to("Спасти кого?\n\r");
                return;
        }

        if ( ( victim = get_char_room( ch, arg ) ) == 0 )
        {
                ch->send_to("Этого нет здесь.\n\r");
                return;
        }

        if ( victim == ch )
        {
                ch->send_to("Себя?\n\r");
                return;
        }

        if ( !ch->is_npc() && victim->is_npc() )
        {
                ch->send_to("Твоя помощь не нужна!\n\r");
                return;
        }

        if ( ch->fighting == victim )
        {
                ch->send_to("Слишком поздно...\n\r");
                return;
        }

        if ( ( ( fch = victim->fighting ) == 0 )
                && ( victim->death_ground_delay == 0 ) )
        {
                ch->send_to("Но никто не нуждается в помощи..\n\r");
                return;
        }

        if ( ch->is_npc() && ch->master != 0 && victim->is_npc() )
                return;
        
        if ((fch && is_safe(ch, fch)) || is_safe( ch, victim ))
            return;

        if (ch->is_npc( ) && ch->master != 0)
            if ((fch && is_safe(ch->master, fch)) || is_safe( ch->master, victim ))
                return;

        ch->setWait( gsn_rescue->getBeats( )  );

        if ( ( number_percent( ) > gsn_rescue->getEffective( ch ) )
                || ( victim->getModifyLevel() > ( ch->getModifyLevel() + 30) )
                || ( ( fch == 0  ) && victim->trap.isSet( TF_NO_RESCUE ) ) )
        {
                ch->send_to("Твоя попытка спасти не удалась.\n\r");
                gsn_rescue->improve( ch, false, victim );
                return;
        }

        act_p( "Ты спасаешь $C4!",  ch, 0, victim, TO_CHAR,POS_RESTING);
        act_p( "$c1 спасает тебя!", ch, 0, victim, TO_VICT,POS_RESTING);
        act_p( "$c1 спасает $C4!",  ch, 0, victim, TO_NOTVICT,POS_RESTING);
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

