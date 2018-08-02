/* $Id: fight_cmds.cpp,v 1.1.2.7 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko	    {NoFate, Demogorgon}                           *
 *    Koval Nazar	    {Nazar, Redrum}                 		   *
 *    Doropey Vladimir	    {Reorx}		                           *
 *    Kulgeyko Denis	    {Burzum}		                           *
 *    Andreyanov Aleksandr  {Manwe}		                           *
 *    и все остальные, кто советовал и играл в этот MUD	                   *
 ***************************************************************************/

#include "fleemovement.h"
#include "commandtemplate.h"
#include "skillcommand.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act_move.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "stats_apply.h"
#include "mercdb.h"
#include "handler.h"
#include "fight.h"
#include "act.h"
#include "def.h"


PROF(samurai);


CMDRUN( kill )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("Убить кого?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("Этого нет здесь.\n\r");
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	ch->send_to("Ты делаешь лучшее из того, что можешь!\n\r");
	return;
    }

    if ( !victim->is_npc() )
    {
	ch->send_to("Игроков убивают с помощью MURDER.\n\r");
	return;
    }

    if ( victim == ch )
    {
	ch->send_to("{RТЫ БЬЕШЬ СЕБЯ!{x Ого...\n\r");
	multi_hit( ch, ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act_p( "Но $C1 твой любимый хозяин!", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

     ch->setWaitViolence( 1 );

    
    if (gsn_mortal_strike->getCommand( )->run( ch, victim ))
	return;

    multi_hit( ch, victim );
}

CMDRUN( murder )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("Порешить кого?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("Этого нет здесь.\n\r");
	return;
    }

    if ( victim == ch )
    {
	ch->send_to("Самоубийство - это смертельный грех.\n\r");
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act_p( "Но $C1 твой любимый хозяин.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	ch->send_to("Ты делаешь лучшее из того, что можешь!\n\r");
	return;
    }

     ch->setWaitViolence( 1 );

//    if ( !victim->is_npc()
//	|| ( ch->is_npc() && victim->is_npc() ) )
    yell_panic( ch, victim,
		"Помогите! На меня кто-то напал!",
		"Помогите! На меня напа%1$Gло|л|ла %1$C1!",
		FYP_VICT_ANY );
    
    if (gsn_mortal_strike->getCommand( )->run( ch, victim ))
	return;

    multi_hit( ch, victim );
}



CMDRUN( flee )
{
    if (ch->fighting == 0) {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;

	ch->send_to("Ты ни с кем не сражаешься.\n\r");
	return;
    }

    if (ch->getProfession( ) == prof_samurai
	&& ch->getRealLevel( ) > 10
	&& number_percent( ) < min( ch->getRealLevel( ) - 10, 90 ))
    {
	ch->send_to("Это будет слишком большим позором для тебя!\n\r");
	return;
    }

    FleeMovement( ch ).move( );
}

CMDRUN( slay )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("Умертвить кого?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("Этого нет здесь.\n\r");
	return;
    }

    if ( ch == victim )
    {
	ch->send_to("Самоубийство - это смертельный грех.\n\r");
	return;
    }

    if ( ( !ch->is_npc()
	    && !victim->is_npc()
	    && victim->getRealLevel( ) >= ch->get_trust( ) )
	|| ( ch->is_npc()
	    && !victim->is_npc()
	    && !victim->is_immortal( )
	    && victim->get_trust( ) >= ch->getRealLevel( ) ) )
    {
	ch->send_to("Твоя попытка безуспешна.\n\r");
	return;
    }

    act_p( "Ты хладнокровно умерщвляешь $C4!", ch, 0, victim, TO_CHAR,POS_RESTING);
    act_p( "$c1 хладнокровно умерщвляет тебя!", ch, 0, victim, TO_VICT,POS_RESTING);
    act_p( "$c1 хладнокровно умерщвляет $C4!", ch, 0, victim, TO_NOTVICT,POS_RESTING);
    raw_kill( victim, -1, 0, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    if( !ch->is_npc() && !victim->is_npc() && ch != victim )
    {
	set_slain( victim );
    }
}




