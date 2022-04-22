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
{
}

Character * PersonalChannel::findListener( Character *ch, const DLString &arg ) const
{
    Character *victim = get_char_world( ch, arg.c_str( ) );

    if (!victim)
        ch->pecho( "Ты не находишь этого персонажа." );
    
    return victim;
}

void PersonalChannel::tellToBuffer( Character *ch, Character *victim, const DLString &messageChar, const DLString &messageVict ) const
{
    if (!victim->is_npc())
        victim->getPC()->getAttributes().getAttr<ReplayAttribute>("replay")->addMessage(messageVict);

    postOutput(victim, messageVict);
    postOutput(ch, messageChar);
    
    victim->reply = ch;
}

bool PersonalChannel::checkIgnore( Character *ch, Character *victim ) const
{
    if (CommunicationChannel::checkIgnore( ch, victim )) {
        oldact_p("$E не желает тебя слышать.", ch, 0, victim, TO_CHAR, position );
        return true;
    }

    return false;
}

bool PersonalChannel::checkAFK( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!IS_SET(victim->comm, COMM_AFK))
        return false;
   
    if (victim->is_npc( ) || !victim->getPC( )->getAttributes( ).isAvailable( "afk" ))
        oldact_p("$C1 отсутствует и не может сейчас получить твое сообщение.", 
                ch, 0, victim, TO_CHAR, position );
    else
        oldact_p("$C1 не может сейчас получить твое сообщение, т.к. $E отсутствует: {c$t{x.", 
                ch, 
                victim->getPC( )->getAttributes( ).findAttr<XMLStringAttribute>( 
                                    "afk" )->getValue( ).c_str( ), 
                victim, TO_CHAR, position );

    oldact_p("Сообщение будет прочитано, когда $E вернется.", ch, 0, victim, TO_CHAR, position );
    return true;
}

bool PersonalChannel::checkAutoStore( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!victim->fighting)
        return false;

    if (!IS_SET( victim->add_comm, COMM_STORE ))
        return false;

    if (victim->is_npc( )) {
        oldact_p("$E сейчас сражается и не может получить твое сообщение.",ch,0,victim,TO_CHAR,position);
        return true;
    }
    
    oldact_p("$C1 сейчас сражается, но твое сообщение будет прочитано, когда $E закончит бой.",
            ch,0,victim,TO_CHAR,position);
    return true;
}

bool PersonalChannel::checkDisconnect( Character *ch, Character *victim, const DLString &msg ) const
{
    if (victim->is_npc( ))
        return false;

    if (victim->desc)
        return false;

    oldact_p("У $C2 нет связи с этим миром, но твое сообщение будет прочитано, когда $E вернется.",
            ch,0,victim,TO_CHAR,position);
    return true;
}

bool PersonalChannel::checkPosition( Character *ch, Character *victim ) const
{
    if (ch->is_immortal( ))
        return false;
    
    if (positionOther <= victim->position)
        return false;
    
    oldact_p("$E не слышит тебя, попробуй позже.", ch, 0, victim, TO_CHAR, position );
    return true;
}

bool PersonalChannel::checkVictimDeaf( Character *ch, Character *victim ) const
{
    if (ch->is_immortal( ))
        return false;

    if (IS_SET(victim->comm, COMM_QUIET) || IS_SET(victim->comm, COMM_DEAF))
    {
        oldact_p("Твое сообщение не дошло $M.", ch, 0, victim, TO_CHAR, position );
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

    if (!isPersonalListener(ch, victim, msg))
        return;

    // Format message to the talker.    
    bool fAuto = (ch == victim && !msgAuto.empty( ));
    const DLString &fmtChar = fAuto ? msgAuto : msgChar;
    DLString outChar = msg;
    applyGarble( ch, outChar, ch );
    DLString messageChar = outputChar( ch, victim, fmtChar, outChar );

    // Format message to the victim.
    const DLString &fmtVict = msgVict;
    DLString outVict = msg;
    applyGarble( ch, outVict, victim );
    DLString messageVict = outputVict( ch, victim, fmtVict, outVict );

    if (checkAFK( ch, victim, msg )) {
        tellToBuffer( ch, victim, messageChar, messageVict );
        return;
    }
    
    if (checkAutoStore( ch, victim, msg )) {
        tellToBuffer( ch, victim, messageChar, messageVict );
        return;
    }
    
    if (checkDisconnect( ch, victim, msg )) {
        tellToBuffer( ch, victim, messageChar, messageVict );
        return;
    }
    
    if (needOutputChar( ch )) {
        ch->pecho(messageChar);
        postOutput(ch, messageChar);
    }

    if (needOutputVict( ch, victim )) {
        victim->pecho(messageVict);
        postOutput(victim, messageVict);
    }

    triggers( ch, victim, msg );
}

bool PersonalChannel::canTalkPersonally( Character *ch ) const
{
    if (IS_SET(ch->comm, COMM_NOTELL)) {
        ch->pecho( "Боги лишили тебя возможности личного общения." );
        return false;
    }

    if (IS_SET(ch->comm, COMM_QUIET)) {
        ch->pecho( "Сперва выйди из режима тишины (quiet)." );
        return false;
    }

    if (IS_SET(ch->comm, COMM_DEAF)) {
        ch->pecho("Сперва выйди из режима глухоты (deaf).");
        return false;
    }

    return true;
}

/**
 * Checks if victim can receive a message from ch via private channels.
 * No side-effects such as storing messages to the buffer (they all occur in the run() method).
 */
bool PersonalChannel::isPersonalListener( Character *ch, Character *victim, const DLString &msg ) const
{
    if (checkIgnore( ch, victim )) 
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
        ch->pecho( msgNoName );
        return false; 
    }

    if (msg.empty( )) {
        ch->pecho( msgNoArg );
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
    if (!outputTo->is_npc())
        remember_history_private( outputTo->getPC( ), message );
}

