/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "personalchannel.h"
#include "replay.h"

#include "logstream.h"
#include "commonattributes.h"
#include "skillreference.h"
#include "pcharacter.h"

#include "dreamland.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

GSN(deafen);

/*-----------------------------------------------------------------------
 * PersonalChannel
 *-----------------------------------------------------------------------*/
PersonalChannel::PersonalChannel( )
		    : storeAFK( false ), storeFight( false ), storeDisco( false )
{
}

Character * PersonalChannel::findListener( Character *ch, const DLString &arg ) const
{
    Character *victim = get_char_world( ch, arg.c_str( ) );

    if (!victim)
	ch->println( "Ты не находишь этого персонажа." );
    
    return victim;
}

void PersonalChannel::tellToBuffer( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!victim->is_npc( )) {
        DLString message = fmt( victim, msgVict.c_str( ), ch, msg.c_str( ), victim );

	victim->getPC( )->getAttributes( ).getAttr<XMLStringListAttribute>( 
				    "tells" )->push_back( message );
        remember_history_private( victim->getPC( ), message );
    }
    
    victim->reply = ch;
}

bool PersonalChannel::checkIgnore( Character *ch, Character *victim ) const
{
    if (CommunicationChannel::checkIgnore( ch, victim )) {
	act_p( "$E не желает тебя слышать.", ch, 0, victim, TO_CHAR, position );
	return true;
    }

    return false;
}

bool PersonalChannel::checkAFK( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!IS_SET(victim->comm, COMM_AFK))
	return false;
   
    if (victim->is_npc( ) || !victim->getPC( )->getAttributes( ).isAvailable( "afk" ))
	act_p( "$C1 отсутствует и не может сейчас получить твое сообщение.", 
		ch, 0, victim, TO_CHAR, position );
    else
	act_p("$C1 не может сейчас получить твое сообщение, т.к. $E отсутствует: {c$t{x.", 
		ch, 
		victim->getPC( )->getAttributes( ).findAttr<XMLStringAttribute>( 
				    "afk" )->getValue( ).c_str( ), 
		victim, TO_CHAR, position );

    if (storeAFK) {
	act_p( "Сообщение будет прочитано, когда $E вернется.", ch, 0, victim, TO_CHAR, position );
	tellToBuffer( ch, victim, msg );
    }

    return true;
}

bool PersonalChannel::checkAutoStore( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!victim->fighting)
	return false;

    if (victim->is_npc( )) {
	act_p("$E сейчас сражается и не может получить твое сообщение.",ch,0,victim,TO_CHAR,position);
	return true;
    }
    
    if (!IS_SET( victim->add_comm, COMM_STORE ) || !storeFight)
	return false;

    act_p("$E сейчас сражается, но твое сообщение будет прочитано, когда $E закончит бой.",
	    ch,0,victim,TO_CHAR,position);
    tellToBuffer( ch, victim, msg );

    return true;
}

bool PersonalChannel::checkDisconnect( Character *ch, Character *victim, const DLString &msg ) const
{
    if (victim->is_npc( ))
	return false;

    if (victim->desc)
	return false;

    act_p("У $C2 нет связи с этим миром... попробуй позже.",
	    ch,0,victim,TO_CHAR,position);
    
    if (storeDisco)
	tellToBuffer( ch, victim, msg );

    return true;
}

bool PersonalChannel::checkPosition( Character *ch, Character *victim ) const
{
    if (ch->is_immortal( ))
	return false;
    
    if (positionOther <= victim->position)
	return false;
    
    act_p( "$E не слышит тебя.", ch, 0, victim, TO_CHAR, position );
    return true;
}

bool PersonalChannel::checkVictimDeaf( Character *ch, Character *victim ) const
{
    if (ch->is_immortal( ))
	return false;

    if (IS_SET(victim->comm, COMM_QUIET) || IS_SET(victim->comm, COMM_DEAF))
    {
	act_p( "Твое сообщение не дошло $M.", ch, 0, victim, TO_CHAR, position );
	return true;
    }

    return false;
}


void PersonalChannel::run( Character *ch, const DLString &constArguments )
{
    Character *victim;
    DLString name, msg;
    
    if (!canTalkPersonally( ch ))
	return;
    
    if (!parseArguments( ch, constArguments, msg, name ))
	return;
    
    if (!( victim = findListener( ch, name ) ))
	return;
    
    if (!isPersonalListener( ch, victim, msg ))
	return;
    
    if (needOutputChar( ch )) {
	bool fAuto = (ch == victim && !msgAuto.empty( ));
	const DLString &fmtChar = fAuto ? msgAuto : msgChar;
	
	DLString outChar = msg;
	applyGarble( ch, outChar, ch );
	
	outputChar( ch, victim, fmtChar, outChar );
    }

    if (needOutputVict( ch, victim )) {
	const DLString &fmtVict = msgVict;

	DLString outVict = msg;
	applyGarble( ch, outVict, victim );

	outputVict( ch, victim, fmtVict, outVict );
    }

    triggers( ch, victim, msg );
}

bool PersonalChannel::canTalkPersonally( Character *ch ) const
{
    if (IS_SET(ch->comm, COMM_NOTELL)) {
	ch->println( "Боги лишили тебя возможности личного общения." );
	return false;
    }

    if (IS_SET(ch->comm, COMM_QUIET)) {
	ch->println( "Сперва выйди из режима тишины (quiet)." );
	return false;
    }

    if (IS_SET(ch->comm, COMM_DEAF)) {
	ch->println("Сперва выйди из режима глухоты (deaf).");
	return false;
    }

    return true;
}

bool PersonalChannel::isPersonalListener( Character *ch, Character *victim, const DLString &msg ) const
{
    if (checkIgnore( ch, victim )) 
	return false;

    if (checkAFK( ch, victim, msg ))
	return false;
    
    if (checkAutoStore( ch, victim, msg ))
	return false;
    
    if (checkDisconnect( ch, victim, msg ))
	return false;
    
    if (checkPosition( ch, victim ))
	return false;
    
    if (checkVictimDeaf( ch, victim ))
	return false;

    return true;
}

bool PersonalChannel::parseArguments( Character *ch, const DLString &constArguments,
                                      DLString &msg, DLString &name ) const
{
    msg = constArguments;
    name = msg.getOneArgument( );

    if (name.empty( )) {
	ch->println( msgNoName );
	return false; 
    }

    if (msg.empty( )) {
	ch->println( msgNoArg );
	return false;
    }

    return true;
}

void PersonalChannel::triggers( Character *ch, Character *victim, const DLString &msg ) const
{
}

bool PersonalChannel::needOutputChar( Character *ch ) const
{
    if (deafen && ch->isAffected( gsn_deafen )) 
	return false;

    return true;
}

bool PersonalChannel::needOutputVict( Character *ch, Character *victim ) const
{
    return ch != victim;
}

void PersonalChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (outputTo->getPC( ))
        remember_history_private( outputTo->getPC( ), message );
}

