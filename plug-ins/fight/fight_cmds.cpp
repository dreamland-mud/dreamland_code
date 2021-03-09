/* $Id: fight_cmds.cpp,v 1.1.2.7 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko            {NoFate, Demogorgon}                           *
 *    Koval Nazar            {Nazar, Redrum}                                    *
 *    Doropey Vladimir            {Reorx}                                           *
 *    Kulgeyko Denis            {Burzum}                                           *
 *    Andreyanov Aleksandr  {Manwe}                                           *
 *    и все остальные, кто советовал и играл в этот MUD                           *
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
        ch->pecho("Убить кого?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( ch->position == POS_FIGHTING )
    {
        ch->pecho("Ты делаешь лучшее из того, что можешь!");
        return;
    }

    if ( !victim->is_npc() )
    {
        ch->pecho("Игроков убивают с помощью MURDER.");
        return;
    }

    if ( victim == ch )
    {
        ch->pecho("{RТЫ БЬЕШЬ СЕБЯ!{x Ого...");
        multi_hit( ch, ch );
        return;
    }

    if ( IS_CHARMED(ch) && ch->master == victim )
    {
        oldact("Но $C1 тво$Gй|й|я любим$Gый|ый|ая хозя$Gин|ин|йка!", ch, 0, victim, TO_CHAR);
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
        ch->pecho("Порешить кого?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if ( victim == ch )
    {
        ch->pecho("Самоубийство - это смертельный грех.");
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( IS_CHARMED(ch) && ch->master == victim )
    {
        oldact("Но $C1 тво$Gй|й|я любим$Gый|ый|ая хозя$Gин|ин|йка.", ch, 0, victim, TO_CHAR);
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        ch->pecho("Ты делаешь лучшее из того, что можешь!");
        return;
    }

     ch->setWaitViolence( 1 );

//    if ( !victim->is_npc()
//        || ( ch->is_npc() && victim->is_npc() ) )
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

        ch->pecho("Ты ни с кем не сражаешься.");
        return;
    }

    if (ch->getProfession( ) == prof_samurai
        && ch->getRealLevel( ) > 10
        && number_percent( ) < min( ch->getRealLevel( ) - 10, 90 ))
    {
        ch->pecho("Это будет слишком большим позором для тебя!");
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
        ch->pecho("Умертвить кого?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if ( ch == victim )
    {
        ch->pecho("Самоубийство - это смертельный грех.");
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
        ch->pecho("Твоя попытка безуспешна.");
        return;
    }

    act("Ты хладнокровно умерщвляешь %2$C4!", ch, victim, 0,TO_CHAR);
    act("%^C1 хладнокровно умерщвляет тебя!", ch, victim, 0,TO_VICT);
    act("%1$^C1 хладнокровно умерщвляет %2$C4!", ch, victim, 0,TO_NOTVICT);
    raw_kill( victim, -1, 0, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    if( !ch->is_npc() && !victim->is_npc() && ch != victim )
    {
        set_slain( victim );
    }
}




