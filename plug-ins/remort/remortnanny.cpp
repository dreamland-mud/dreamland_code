/* $Id: remortnanny.cpp,v 1.1.2.13.4.6 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include "remortnanny.h"
#include "logstream.h"
#include "class.h"
#include "room.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "merc.h"
#include "descriptor.h"
#include "interp.h"
#include "mercdb.h"
#include "act.h"
#include "move_utils.h"
#include "vnum.h"
#include "def.h"

PROF(universal);


/*-----------------------------------------------------------------------------
 * descriptor state listener for remorting players 
 *----------------------------------------------------------------------------*/

void RemortNanny::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;
    Room *izba;
    
    if (newState != CON_PLAYING)
	return;
    
    if (!d->character || !( ch = d->character->getPC( ) ))
	return;
    
    if (ch->getRemorts( ).size( ) == 0)
	return;
    
    if (ch->getRemorts( ).points == 0)
	return;

    if (!( izba = get_room_index( ROOM_VNUM_REMORT ) )) {
	LogStream::sendError( ) << "Zero remort room!" << endl;
	return;
    }
    
    ch->position = std::max( POS_RESTING, (int)ch->position );

    if (ch->in_room != izba)
	transfer_char( ch, 0, izba,
			"Ветер перемен подхватывает %1$C4 и уносит куда-то.. ", 
			"Ветер перемен переносит тебя в другое место..", 
			"%1$C1 влетает в избу через окошко.",
			"Влетев в избу через окошко, ты шлепаешься на пол." );
    
    if (ch->pet && ch->pet->in_room != izba) {
	ch->pet->position = std::max( POS_RESTING, (int)ch->pet->position );
	transfer_char( ch->pet, 0, izba,
	                NULL, NULL, "%1$C1 влетает в избу через окошко." );
    }
}

/*-----------------------------------------------------------------------------
 * baba yaga 
 *----------------------------------------------------------------------------*/
RemortWitch::RemortWitch( ) 
{
}

void RemortWitch::greet( Character *victim ) 
{
    PCharacter *pch;

    if (!( pch = victim->getPC( ) ))
	return;

    if (pch->getRemorts( ).points == 0)
	return;
    
    act( "$c1, кряхтя, поворачивается к тебе.", ch, 0, pch, TO_VICT );
    act( "$c1, кряхтя, поворачивается к $C3.", ch, 0, pch, TO_NOTVICT );
    act( "$c1 говорит тебе '{G$C1, здравствуй, родненьк$Gое|ий|ая..{x'", ch, 0, pch, TO_VICT );
}

void RemortWitch::speech( Character *victim, const char *msg ) 
{
    tell( victim, msg );
}

bool RemortWitch::canServeClient( Character * )
{ 
    return true; 
}

void RemortWitch::msgArticleNotFound( Character *client ) 
{
    tell_raw( client, ch, "Слыхом о таком не слыхивала." );
}

void RemortWitch::msgProposalNotFound( Character *client )
{
    tell_raw( client, ch, "Нету у тебя такого, не обманывай бабушку." );
}

void RemortWitch::msgListBefore( Character *client )
{
    client->pecho( "\nУ %C2 имеется: ", ch );
}

void RemortWitch::msgArticleTooFew( Character *client, Article::Pointer )
{
    tell_raw( client, ch, "Нехорошо жадничать!" );
}

void RemortWitch::tell( Character *victim, const char *msg ) 
{
    PCharacter *client;
    DLString argument = msg;
    DLString cmd = argument.getOneArgument( );

    if (is_name( cmd.c_str( ), "i я" ))
	cmd = argument.getOneArgument( );

    if (!( client = victim->getPC( ) ) || cmd.empty( ))
	return;
    
    /*
     * buy
     */
    if (is_name( cmd.c_str( ), "дай отдай хочу куплю покупаю buy want беру" )) {
	if (argument.empty( )) {
	    tell_act( client, ch, "И что же тебе дать, родненьк$gое|ий|ая?" );
	    return;
	}

	doBuy( client, argument.quote( ) );
	return;
    }
    
    /*
     * sell
     */
    if (is_name( cmd.c_str( ), "продаю забери take продам sell возвращаю" )) {
	if (argument.empty( )) {
	    tell_act( client, ch, "И что же ты хочешь мне вернуть, родненьк$gое|ий|ая?" );
	    return;
	}
	
	doSell( client, argument );
	return;
    }
    
    /*
     * done
     */
    if (is_name( cmd.c_str( ), "баста хватит enough basta" )) {
	if (client->getRemorts( ).points > 0) {
	    tell_raw( client, ch, "Нет уж, мы с тобой еще не закончили." );
	    tell_raw( client, ch, "Ты не сможешь отсюда выбраться, пока не заберешь у меня все, что тебе причитается." );
	    return;
	}

	transfer_char( client, 0, 
		       get_room_index( client->getRealLevel( ) == 1 ? ROOM_VNUM_HARBOUR : ROOM_VNUM_TEMPLE ),
		       "%1$^C1 вылетает в печную трубу.", 
		       "Ты вылетаешь в печную трубу прочь!", 
		       "%1$^C1 падает с неба." );
	if (client->pet)
	    transfer_char( client->pet, 0, client->in_room );
	
	client->save( );
	return;
    }
    
    tell_raw( client, ch, "Ась?" );
}

bool RemortWitch::look_inv( Character *looker ) 
{
    if (looker->is_npc( ))
	return false;
	
    doList( looker );
    return true;
}

