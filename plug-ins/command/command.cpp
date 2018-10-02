/* $Id: command.cpp,v 1.1.2.12.6.12 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * logic based on interpret() from DreamLand 2.0
 */

#include "command.h"
#include "commandhelp.h"
#include "commandmanager.h"
#include "commandflags.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "helpmanager.h"
#include "skillreference.h"

#include "descriptor.h"
#include "loadsave.h"
#include "merc.h"
#include "act.h"
#include "def.h"

GSN(manacles);
GSN(improved_invis);

/*--------------------------------------------------------------------------
 * Command
 *-------------------------------------------------------------------------*/
const Flags Command::defaultOrder( 0, &order_flags );
const Flags Command::defaultExtra( 0, &command_flags );
const Enumeration Command::defaultPosition( POS_DEAD, &position_table );
const Enumeration Command::defaultCategory( CMD_CAT_OTHER, &command_category_table );

Command::Command( ) 
{
}

Command::~Command( ) 
{
    
}

CommandHelp::Pointer Command::getHelp( ) const
{
    return CommandHelp::Pointer( );
}

bool Command::available( Character *ch ) const 
{
    if (getLevel( ) > ch->get_trust( ))
	return false;

    if (ch->desc && ch->desc->connected != CON_PLAYING)
	return getExtra( ).isSet( CMD_NANNY );

    return true;
}

bool Command::visible( Character *ch ) const
{
    if (!available( ch ))
	return false;
    
    if (getExtra( ).isSet( CMD_HIDDEN ))
	return false;

    return true;
}

const DLString & Command::getRussianName( ) const
{
    return DLString::emptyString;
}

short Command::getLog( ) const
{
    return LOG_NORMAL;
}

const Flags & Command::getExtra( ) const
{
    return defaultExtra;
}

short Command::getLevel( ) const
{
    return 0;
}

const Enumeration & Command::getPosition( ) const
{
    return defaultPosition;
}

const Flags & Command::getOrder( ) const
{
    return defaultOrder;
}

const Enumeration & Command::getCommandCategory( ) const
{
    return defaultCategory;
}

bool Command::matchesExactly( const DLString &cmdName ) const
{
    return getName( ) == cmdName || getRussianName( ) == cmdName;
}

bool Command::matches( const DLString& command ) const
{
    if (command.empty( ))
	return false;

    if (command.strPrefix( getName( ) ))
	return true;

    if (command.strPrefix( getRussianName( ) ))
	return true;
    
    return false;
}

bool Command::matchesAlias( const DLString& command ) const
{
    return false;
}

bool Command::properOrder( Character *ch )
{
    if (!ch->is_npc( )) 
	return (getOrder( ).isSet( ORDER_ALLOW_RULER ) || !ch->isAffected(gsn_manacles ));
    
    if (getOrder( ).isSet( ORDER_FIGHT_ONLY ) && ch->fighting == 0)
	return false;
    
    if (getOrder( ).isSet( ORDER_PLAYER_ONLY ))
	return false;
    
    if (getOrder( ).isSet( ORDER_THIEF_ONLY ) && !IS_SET(ch->act, ACT_THIEF))
	return false;

    return true;
}    

bool Command::dispatchOrder( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (!ch->is_npc( ) 
	&& IS_SET(ch->act, PLR_FREEZE)
	&& !getExtra( ).isSet( CMD_FREEZE ))
    {
	ch->pecho("Ты полностью замороже%Gно|н|на!", ch);
	return false;
    }

    if (IS_SET( ch->comm, COMM_AFK ) && !getExtra( ).isSet( CMD_AFK )) {
	ch->send_to( "Выйди сначала из {WAFK{x\n\r" );
	return false;
    }

    if (getExtra( ).isSet( CMD_SPELLOUT ) && !matchesExactly( iargs.cmdName.toLower( ) )) {
	ch->printf("Команду '%s' необходимо ввести полностью.\n\r", getName( ).c_str( ) );
	return false;
    }
		    
    if (IS_AFFECTED( ch, AFF_STUN ) && !getExtra( ).isSet( CMD_KEEP_HIDE )) {
	ch->send_to( "Ты не в состоянии сделать это.\n\r" );
	return false;
    }

    // prevent ghosts from doing a bunch of commands
    if (IS_GHOST( ch ) && !getExtra( ).isSet( CMD_GHOST )) {
	ch->send_to( "У тебя нет тела... А твой немощный дух не в состоянии тебе помочь.\n\r" );
	return false;
    }

    // Come out of hiding for most commands
    visualize( ch );

    // Character not in position for command?
    if (!checkPosition( ch ))
	return false;

    return true;
}

bool Command::dispatch( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (IS_AFFECTED(ch, AFF_CHARM) 
	&& !(ch->is_npc( ) && ch->getNPC( )->switchedFrom)) 
    {
	ch->send_to( "Сперва спроси разрешения у любимого хозяина!\n\r" );
	return false;
    }

    if (ch->isAffected(gsn_manacles ) && !getExtra( ).isSet( CMD_MANACLES )) {
	ch->send_to( "Ты не можешь этого сделать - мешают кандалы!\n\r" );
	return false;
    }

    return dispatchOrder( iargs );
}

void Command::visualize( Character *ch )
{
    if (ch->is_npc( )) /* ? */
	return;

    if (IS_AFFECTED( ch, AFF_HIDE | AFF_FADE ) && !getExtra( ).isSet( CMD_KEEP_HIDE ))
    {
	REMOVE_BIT( ch->affected_by, AFF_HIDE | AFF_FADE );
	ch->send_to( "Ты выходишь из тени.\n\r" );
	act_p( "$c1 выходит из тени.", ch, 0, 0, TO_ROOM, POS_RESTING );
    }

    if (IS_AFFECTED( ch, AFF_IMP_INVIS ) && getPosition( ).getValue( ) == POS_FIGHTING)
    {
	affect_strip( ch, gsn_improved_invis );
	REMOVE_BIT( ch->affected_by, AFF_IMP_INVIS );
	act_p( "Ты становишься видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_CHAR, POS_RESTING );
	act_p( "$c1 становится видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_ROOM, POS_RESTING );
    }

    if (DIGGED(ch) && (getPosition( ).getValue( ) > POS_RESTING || getExtra( ).isSet( CMD_UNDIG )))
	undig(ch);
}


bool Command::checkPosition( Character *ch )
{
    if (ch->position >= getPosition( ).getValue( ))
	return true;

    switch (ch->position.getValue( )) {
	case POS_DEAD:
	    ch->send_to("Лежи смирно! Ты {RТРУП{x.\n\r");
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    ch->send_to( "Даже не думай об этом! Ты в ужасном состоянии.\n\r" );
	    break;

	case POS_STUNNED:
	    ch->send_to( "Ты не в состоянии сделать это.\n\r" );
	    break;

	case POS_SLEEPING:
	    ch->send_to( "Во сне? Или может сначала проснешься...\n\r" );
	    break;

	case POS_RESTING:
	    ch->send_to( "Уфф... Но ведь ты отдыхаешь...\n\r" );
	    break;

	case POS_SITTING:
	    ch->send_to( "Сидя? Или может сначала встанешь...\n\r" );
	    break;

	case POS_FIGHTING:
	    act_p( "Куда! Ты долж$gно|ен|на сражаться.", ch, 0, 0, TO_CHAR, POS_FIGHTING );
	    break;
    }
    
    return false;
}

static const XMLStringList emptyList;

const XMLStringList &Command::getAliases( ) const
{
    return emptyList;
}

const XMLStringList &Command::getRussianAliases( ) const
{
    return emptyList;
}

const DLString & Command::getHint( ) const
{
    return DLString::emptyString;
}

/*--------------------------------------------------------------------------
 * XMLCommand
 *-------------------------------------------------------------------------*/
XMLCommand::XMLCommand( ) 
{
}

XMLCommand::~XMLCommand( ) 
{
}

CommandLoader * XMLCommand::getLoader( ) const 
{
    return commandManager;
}

