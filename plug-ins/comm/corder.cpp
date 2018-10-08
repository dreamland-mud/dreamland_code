/* $Id: corder.cpp,v 1.1.2.14.6.6 2009/01/01 20:07:46 rufina Exp $
 *
 * ruffina, 2004
 * logic based on 'do_order' from DreamLand 2.0
 */

#include "corder.h"
#include "commandinterpreter.h"

#include "pcharacter.h"
#include "room.h"
#include "clanreference.h"
#include "skillreference.h"

#include "follow_utils.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

CLAN(ruler);
GSN(manacles);

COMMAND(COrder, "order")
{
    Character *victim;
    InterpretArguments iargs;
    DLString argTarget, argOrder;
    DLString argument = constArguments;
    
    argTarget = argument.getOneArgument( );
    argOrder = argument;
    
    if (argTarget.empty( ) || argOrder.empty( )) {
	ch->println( "Приказать кому и что?" );
	return;
    }

    if (IS_AFFECTED( ch, AFF_CHARM )) {
	ch->println( "Ты можешь только принимать приказы, а не отдавать их." );
	return;
    }
    
    if (argTarget == "all") {
	ch->println( "Ты не можешь отдать приказ всем сразу." );
	return;
    }
    
    victim = findVictim( ch, argTarget );
    
    if (!victim) {
	ch->println( "Среди твоих последователей такого нет." );
	return;
    }

    interpretOrder( victim, iargs, argOrder );
    
    if (!iargs.pCommand || !iargs.pCommand->properOrder( victim )) {
	if (victim->isAffected( gsn_manacles ))
	    act( "$C1 говорит тебе '{GЯ не буду делать это.{x'", ch, 0, victim, TO_CHAR );
	else
	    act( "$C1 говорит тебе '{GЯ не понимаю, чего ты хочешь, хозя$gин|ин|йка.{x'", ch, 0, victim, TO_CHAR );
    }
    else {
	act( "$c1 приказывает тебе '$t', ты покорно исполняешь приказ.", ch, iargs.pCommand->getName( ).c_str( ), victim, TO_VICT );
	
	if (iargs.pCommand->dispatchOrder( iargs ))
	    iargs.pCommand->run( victim, iargs.cmdArgs );
    }


    ch->setWaitViolence( 1 );
    ch->println( "Ok.");
}

bool COrder::canOrder( Character *ch, Character *victim )
{
    if (ch == victim)
	return false;

    if (ch->getClan( ) == clan_ruler && victim->isAffected(gsn_manacles))
	return true;
	
    if (!IS_AFFECTED(victim, AFF_CHARM))
	return false;
    
    if (victim->master != ch)
	return false;
	
    if (victim->is_immortal( ) 
	&& victim->get_trust( ) >= ch->getModifyLevel( ))
	return false;
    
    return true;
}

Character * COrder::findVictim( Character *ch, DLString &argument )
{
    Character *rch;
    int number, count;
    
    count  = 0;
    number = argument.getNumberArgument( );
    
    for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
    {
	Character *tch;
	
	if (!canOrder( ch, rch ))
	    continue;

	tch = rch->getDoppel( ch );

	if (tch->is_npc( ))
	{
	    if (!is_name( argument.c_str( ), tch->getNameP( ) ))
		continue;
	}
	else
	{
	    if (!is_name( argument.c_str( ), tch->getNameP( '7' ).c_str() ))
		continue;
	}

	if (++count == number)
	    return rch;
    }

    return NULL;
}

void COrder::interpretOrder( Character *och, InterpretArguments &iargs, const DLString &args )
{
    static int phases [] = { 
	CMDP_LOG_INPUT,
	CMDP_GRAB_WORD,
	CMDP_FIND,
	CMDP_LOG_CMD,
	0
    };

    iargs.ch = och;
    iargs.line = args;
    iargs.phases = phases;

    CommandInterpreter::getThis( )->run( iargs );
}
