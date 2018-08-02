/* $Id: socialbase.cpp,v 1.1.2.3.6.8 2009/08/16 02:50:31 rufina Exp $
 * 
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#include "socialbase.h"

#include "skillreference.h"
#include "character.h"
#include "room.h"

#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

GSN(improved_invis);

SocialBase::SocialBase( ) 
{
}

SocialBase::~SocialBase( )
{
}

short SocialBase::getLog( ) const
{
    return LOG_NORMAL;
}

bool SocialBase::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
	return false;

    if (argument.strPrefix( getName( ) )) 
	return true;
    
    if (argument.strPrefix( getRussianName( ) )) 
	return true;

    return false;
}

bool SocialBase::properOrder( Character * )
{
    return true;
}

bool SocialBase::dispatchOrder( const InterpretArguments &iargs )
{
    return dispatch( iargs );
}

bool SocialBase::dispatch( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (!ch->is_npc( )) {
	if (IS_SET(ch->act, PLR_FREEZE)) {
	    ch->pecho("Ты полностью замороже%Gно|н|на!", ch);
	    return false;
	}

	if (IS_SET( ch->comm, COMM_NOEMOTE )) {
	    ch->pecho("Ты анти-социал%Gьно|ен|ьна!", ch);
	    return false;
	}

	if (IS_SET( ch->comm, COMM_AFK )) {
	    ch->send_to( "Выйди сначала из {WAFK{x\n\r" );
	    return false;
	}
    }
    
    if (!checkPosition( ch )) 
	return false;
    
    visualize( ch );
    return true;
}

void SocialBase::run( Character *ch, const DLString &constArguments )
{
    Character *victim;
    int pos;
    DLString argument = constArguments;

    DLString firstArgument =  argument.getOneArgument( );
    DLString secondArgument = argument.getOneArgument( );
    pos = getPosition( );
    victim = 0;

    if (firstArgument.empty( ))
    {
	act( getNoargOther( ).c_str( ), ch, 0, victim, TO_ROOM );
	act_p( getNoargMe( ).c_str( ), ch, 0, victim, TO_CHAR,pos );
    }
    else if (( victim = get_char_room( ch, firstArgument ) ) == 0)
    {
/*	
	if (!getErrorMsg( ).empty( ))
	    act_p( getErrorMsg( ).c_str( ), ch, 0, 0, TO_CHAR, pos );
	else
	    ch->send_to("Нет этого здесь.\n\r");
*/	    
    }
    else if (victim == ch)
    {
	act( getAutoOther( ).c_str( ), ch, 0, victim, TO_ROOM );
	act_p( getAutoMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
    }
    else
    {
        Character *victim2 = victim;

        // See if 2-victim syntax is supported by this social. Find second victim.
        if (!getArgMe2( ).empty( ) && !secondArgument.empty( )) {
            victim2 = get_char_room( ch, secondArgument );
            if (!victim2) {
                ch->pecho( "Ты видишь только %1$C4 здесь, кто такой %2s?", victim, secondArgument.c_str( ));
                return;
            }
        }
        
        if (victim2 == victim) {
            act( getArgOther( ).c_str( ), ch, 0, victim, TO_NOTVICT );
            act_p( getArgMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
            act( getArgVictim( ).c_str( ), ch, 0, victim, TO_VICT );
        } else {
            // Output to actor and both victims.
            ch->pecho( getArgMe2( ).c_str( ), ch, victim, victim2 );
            victim->pecho( getArgVictim2( ).c_str( ), ch, victim, victim2 );
            victim2->pecho( getArgVictim2( ).c_str( ), ch, victim2, victim );

            // Output to everyone else in the room.
            for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
                if (rch != ch && rch != victim && rch != victim2)
                    rch->pecho( getArgOther2( ).c_str( ), ch, victim, victim2 );
        }
    }
    
    reaction( ch, victim, firstArgument );
}

bool SocialBase::checkPosition( Character *ch )
{
    if (ch->position >= getPosition( ))
	return true;

    switch (ch->position.getValue( )) {
    case POS_DEAD:
	ch->send_to("Лежи смирно! Ты {RТРУП{x.\n\r");
	break;

    case POS_INCAP:
    case POS_MORTAL:
	ch->send_to("Даже не думай об этом! Ты в ужасном состоянии.\n\r");
	break;

    case POS_STUNNED:
	ch->send_to("Ты не в состоянии сделать это.\n\r");
	break;

    case POS_SLEEPING:
	ch->send_to("Во сне? Или может сначала проснешься...\n\r");
	break;

    case POS_RESTING:
	ch->send_to( "Уфф... Но ведь ты отдыхаешь...\n\r" );
	break;

    case POS_SITTING:
	ch->send_to( "Сидя? Или может сначала встанешь...\n\r" );
	break;

    case POS_FIGHTING:
	act_p( "Тебе не до того, ты же сражаешься!", ch, 0, 0, TO_CHAR, POS_FIGHTING );
	break;
    }

    return false;
}

void SocialBase::visualize( Character *ch )                                        
{
    if (IS_AFFECTED( ch, AFF_HIDE|AFF_FADE ))  {
	REMOVE_BIT( ch->affected_by, AFF_HIDE|AFF_FADE );
	ch->send_to("Ты выходишь из тени.\n\r");
	act_p( "$c1 выходит из тени.", ch, 0, 0, TO_ROOM,POS_RESTING);
    }

    if (IS_AFFECTED(ch, AFF_IMP_INVIS)) {
	affect_strip(ch,gsn_improved_invis);
	act("Ты становишься видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_CHAR);
	act("$c1 становится видим$gо|ым|ой для окружающих.\n\r", ch,0,0,TO_ROOM);
    }
}
