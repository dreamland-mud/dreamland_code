/* $Id: configs.cpp,v 1.1.2.9.6.8 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2005
 * command syntax and messages from DreamLand 2.0
 */

#include "configs.h"

#include "commandmanager.h"
#include "commandtemplate.h"

#include "pcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "interp.h"
#include "def.h"

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_HEADER(ch)  (MILD(ch) ? "w" : "W")
#define CLR_NAME(ch)    (MILD(ch) ? "c" : "C")
#define CLR_YES(ch)     (MILD(ch) ? "g" : "R")
#define CLR_NO(ch)      (MILD(ch) ? "G" : "G")

/*-------------------------------------------------------------------------
 * ConfigElement
 *------------------------------------------------------------------------*/
const DLString & ConfigElement::getRussianName( ) const
{
    return rname;
}

void ConfigElement::init( )
{
    if (autocmd) {
	commandManager->registrate( Pointer( this ) );
    }
}

void ConfigElement::destroy( )
{
    if (autocmd) {
	commandManager->unregistrate( Pointer( this ) );
    }
}

void ConfigElement::run( Character *ch, const DLString & )
{
    if (!ch->is_npc( ))
	handleArgument( ch->getPC( ), "toggle" );
}

bool ConfigElement::handleArgument( PCharacter *ch, const DLString &arg ) const
{
    if (arg.empty( )) {
	printLine( ch );
	return true;
    }
    
    Flags &field = getField( ch );

    if (arg == "yes" || arg == "да")
	field.setBit( bit.getValue( ) );
    else if (arg == "no" || arg == "нет")
	field.removeBit( bit.getValue( ) );
    else if (arg.strPrefix( "toggle" ) || arg.strPrefix( "переключить" ))
	field.toggleBit( bit.getValue( ) );
    else 
	return false;
    
    if (!printText( ch ))
	printLine( ch );

    return true;
}

bool ConfigElement::isSetBit( PCharacter *ch ) const
{
    return getField( ch ).isSet( bit );
}

bool ConfigElement::printText( PCharacter *ch ) const
{
    const DLString &msg = (isSetBit( ch ) ? msgOn.getValue( ) : msgOff.getValue( ) );

    if (!msg.empty( )) {
	ch->println( msg );
	return true;
    }

    return false;
}

void ConfigElement::printRow( PCharacter *ch ) const
{
    bool yes = isSetBit( ch );
    bool rus = ch->getConfig( )->rucommands;

    ch->printf( "| {%s%-14s {x|  {%s%-7s {x|\r\n", 
		      CLR_NAME(ch), 
		      rus ? rname.getValue( ).c_str( ) : name.getValue( ).c_str( ), 
		      yes ? CLR_YES(ch) : CLR_NO(ch),
		      yes ? "ВКЛ." : "ВЫКЛ." );
}

void ConfigElement::printLine( PCharacter *ch ) const
{
    bool yes = isSetBit( ch );

    if (ch->getConfig( )->rucommands)
	ch->printf( "  {%s%-12s {%s%5s {x%s\n",
			CLR_NAME(ch),
			rname.getValue( ).c_str( ),
			yes ? CLR_YES(ch) : CLR_NO(ch),
			yes ? "ДА" : "НЕТ",
			hint.c_str( ) );
    else
	ch->printf( "  {%s%-12s {%s%5s {x%s\n",
			CLR_NAME(ch),
			name.getValue( ).c_str( ),
			yes ? CLR_YES(ch) : CLR_NO(ch),
			yes ? "YES" : "NO",
			hint.c_str( ) );
}

Flags & ConfigElement::getField( PCharacter *ch ) const
{
    static Flags zero;
    const FlagTable *table = bit.getTable( );

    if (table == &config_flags)
	return ch->config;
    
    if (table == &comm_flags)
	return ch->comm;
    
    if (table == &plr_flags)
	return ch->act;

    if (table == &add_comm_flags)
	return ch->add_comm;
    
    return zero;
}

/*-------------------------------------------------------------------------
 * ConfigGroup
 *------------------------------------------------------------------------*/
void ConfigGroup::printHeader( PCharacter *ch ) const
{
    ch->printf( "\r\n{%s%s{x\r\n", 
		    CLR_HEADER(ch),
		    name.getValue( ).c_str( ) );
}

/*-------------------------------------------------------------------------
 * ConfigCommand
 *------------------------------------------------------------------------*/
ConfigCommand * ConfigCommand::thisClass = NULL;

void ConfigCommand::initialization( )
{
    Groups::iterator g;
    
    thisClass = this;
    Class::regMoc<ConfigElement>( );
    Class::regMoc<ConfigGroup>( );
    Class::regMoc<ConfigCommand>( );
    CommandPlugin::initialization( );

    for (g = groups.begin( ); g != groups.end( ); g++) {
	ConfigGroup::iterator c;

	for (c = g->begin( ); c != g->end( ); c++) 
	    (*c)->init( );
    }
}

void ConfigCommand::destruction( )
{
    Groups::iterator g;
    
    for (g = groups.begin( ); g != groups.end( ); g++) {
	ConfigGroup::iterator c;

	for (c = g->begin( ); c != g->end( ); c++) 
	    (*c)->destroy( );
    }

    CommandPlugin::destruction( );
    Class::unregMoc<ConfigCommand>( );
    Class::unregMoc<ConfigGroup>( );
    Class::unregMoc<ConfigElement>( );
    thisClass = NULL;
}

void ConfigCommand::printAllRows( PCharacter *pch ) const
{
    Groups::const_iterator g;
    ConfigGroup::const_iterator c;

    for (g = groups.begin( ); g != groups.end( ); g++) 
	for (c = g->begin( ); c != g->end( ); c++) 
	    if ((*c)->available( pch ))
		if ((*c)->autolist.getValue( ) && !(*c)->autotext.getValue( )) 
		    (*c)->printRow( pch );
}

void ConfigCommand::printAllTexts( PCharacter *pch ) const
{
    Groups::const_iterator g;
    ConfigGroup::const_iterator c;

    for (g = groups.begin( ); g != groups.end( ); g++) 
	for (c = g->begin( ); c != g->end( ); c++) 
	    if ((*c)->available( pch ))
		if ((*c)->autolist.getValue( ) && (*c)->autotext.getValue( )) 
		    (*c)->printText( pch );
}

COMMAND(ConfigCommand, "config")
{
    PCharacter *pch;
    DLString arguments, arg1, arg2;
    Groups::iterator g;
    ConfigGroup::iterator c;

    if (ch->is_npc( ))
	return;
    
    pch = ch->getPC( );

    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );

    if (arg1.empty( )) {
	for (g = groups.begin( ); g != groups.end( ); g++) {
	    g->printHeader( pch );
	    
	    for (c = g->begin( ); c != g->end( ); c++) 
		if ((*c)->available( pch ))
		    (*c)->printLine( pch );
	}

	return;
    }

    for (g = groups.begin( ); g != groups.end( ); g++) 
	for (c = g->begin( ); c != g->end( ); c++) 
	    if ((*c)->available( pch ) 
                    && ((*c)->matches( arg1 ) || (*c)->matchesAlias( arg1 ))) 
            {
		if (!(*c)->handleArgument( pch, arg2 ))
		    pch->println("Неправильный переключатель. См. {W? {lRрежим{lEconfig{x.");

		return;
	    }

    
    pch->println("Опция не найдена. Используйте '{y{lRрежим{lEconfig{lx{w' для списка.");
}

/*-------------------------------------------------------------------------
 * 'autolist' command 
 *------------------------------------------------------------------------*/
CMDRUN( autolist )
{
    static const char *line = "+---------------------------+\r\n";

    PCharacter *pch;
    
    if (ch->is_npc( ))
	return;

    pch = ch->getPC( );
    
    pch->send_to( line );
    pch->printf( "|  {%sНаименование   Состояние{x |\r\n", CLR_HEADER(ch) );
    pch->send_to( line );

    ConfigCommand::getThis( )->printAllRows( pch );
    pch->send_to( line );

    if (pch->lines != PAGELEN) {
	if (pch->lines)
	    pch->printf( "Тебе выводится непрерывно %d линий текста.\r\n", pch->lines.getValue( ) + 2 );
	else
	    pch->send_to( "Буфер прокрутки выключен.\r\n" );
    }

    ConfigCommand::getThis( )->printAllTexts( pch );
    pch->send_to( "\r\n" );
}


/*-------------------------------------------------------------------------
 * 'scroll' command 
 *------------------------------------------------------------------------*/
CMDRUN( scroll )
{
    int lines;
    DLString arg;

    arg = constArguments;
    arg = arg.getOneArgument( );

    if (arg.empty( ))
    {
	if (ch->lines == 0)
	    ch->send_to("Ты не можешь получать длинные сообщения.\n\r");
	else
	    ch->printf( "Тебе непрерывно выводится %d лин%s текста.\n\r",
		        ch->lines.getValue( ) + 2, GET_COUNT(ch->lines.getValue( ) + 2, "ия","ии","ий") );

	return;
    }

    if (!arg.isNumber( )) {
	ch->send_to("Ты должен ввести количество линий.\n\r");
	return;
    }

    try {
        lines = arg.toInt( );
    }
    catch (const ExceptionBadType& ) {
        ch->send_to("Неправильное число.\r\n");
        return;
    }

    if (lines == 0)
    {
        ch->send_to("Вывод отключен.\n\r");
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	ch->send_to("Ты должен ввести допустимое количество линий.\n\r");
	return;
    }

    ch->lines = lines - 2;
    ch->printf( "Вывод установлен на %d лин%s.\n\r", lines,
		GET_COUNT(lines, "ию","ии","ий") );
}
