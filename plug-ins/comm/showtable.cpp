/* $Id: showtable.cpp,v 1.1.2.9 2009/09/11 11:24:54 rufina Exp $
 *
 * ruffina, 2004
 */
/*
 *
 * sturm, 2003
 */
#include "commandtemplate.h"
#include "commandmanager.h"
#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*
 * 'commands'
 */
enum {
    FCMD_TABLE = (A),
    FCMD_ALIASES = (B),
    FCMD_HINTS = (C),
    FCMD_WIZARD = (D),
    FCMD_SHOW_FIRST_RUS = (E),
};

static void show_aliases( Command::Pointer cmd, ostringstream &buf, int flags = 0 )
{
    XMLStringList::const_iterator a, r;

    const XMLStringList &aliases = cmd->getAliases( ); 
    for (a = aliases.begin( ); a != aliases.end( ); a++) 
	buf << *a << " ";

    const XMLStringList &russian = cmd->getRussianAliases( );
    if (!russian.empty( )) {
	r = russian.begin( );

	if (!IS_SET(flags, FCMD_SHOW_FIRST_RUS))
	    r++;
	
	for ( ; r != russian.end( ); r++) 
	    buf << *r << " ";
    }
}

static void show_matched_commands( Character *ch, const DLString &arg )
{
    ostringstream buf;
    CommandList::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );

    for (c = commands.begin( ); c != commands.end( ); c++) {
	ostringstream aliases;
	Command::Pointer cmd = *c;

	if (!cmd->visible( ch ))
	    continue;
	
	if (!cmd->matches( arg ))
	    continue;

	show_aliases( cmd, aliases );

	buf << fmt( 0, "{c%-12s {x: {c%s %s{x\r\n%-12s   %s\r\n",
		       cmd->getName( ).c_str( ),
		       cmd->getRussianName( ).c_str( ),
		       aliases.str( ).c_str( ),
		       " ",
		       cmd->getHint( ).c_str( ) );
    }

    page_to_char( buf.str( ).c_str( ), ch );
}

static void show_commands( Character *ch, int flags )
{
    ostringstream buf;
    CommandList::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );

    if (IS_SET(flags, FCMD_ALIASES|FCMD_HINTS)) {
	buf << fmt( 0, "%-12s | %-17s| %s", 
		    "English name", "Russian name", 
		    IS_SET(flags, FCMD_ALIASES) ? "Aliases" : "Help" ) 
	    << endl
	    << "-------------+------------------+--------------------------------------------" 
	    << endl;

	for (c = commands.begin( ); c != commands.end( ); c++) {
	    ostringstream other;
	    Command::Pointer cmd = *c;
	    
	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) >= LEVEL_HERO)
		continue;
	    
	    if (IS_SET(flags, FCMD_ALIASES)) 
		show_aliases( cmd, other );
	    else 
		other << cmd->getHint( );

	    buf << fmt( 0, "{c%-12s {x: %-17s: %s",
			   cmd->getName( ).c_str( ),
			   cmd->getRussianName( ).c_str( ),
			   other.str( ).c_str( ) )
		<< endl;
	}
    }
    else if (IS_SET(flags, FCMD_TABLE)) {
	int i = 1;
	bool fRus = ch->getConfig( )->rucommands;
	const char *pattern = (fRus ? "%-17s" : "%-13s");
	const int columns = (fRus ? 4 : 6);

	for (c = commands.begin( ); c != commands.end( ); c++) {
	    Command::Pointer cmd = *c;

	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) >= LEVEL_HERO)
		continue;

	    buf << fmt( 0, pattern, 
	                   (fRus && !cmd->getRussianName( ).empty( ) ? 
				cmd->getRussianName( ).c_str( ) : cmd->getName( ).c_str( ) ));
	    
	    if (i++ % columns == 0)
		buf << endl;
	}

	buf << endl;
    }
    else if (IS_SET(flags, FCMD_WIZARD)) {
	buf << fmt( 0, "%-12s | %-45s | %s", "English name", "Help", "Aliases" )
	    << endl
	    << "-------------+-----------------------------------------------+---------------" 
	    << endl;
	for (c = commands.begin( ); c != commands.end( ); c++) {
	    ostringstream aliases;
	    Command::Pointer cmd = *c;

	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) <  LEVEL_HERO)
		continue;

	    show_aliases( cmd, aliases, FCMD_SHOW_FIRST_RUS );

	    buf << fmt( 0, "{c%-12s {x: %-45s : %s",
			   cmd->getName( ).c_str( ),
			   cmd->getHint( ).c_str( ),
			   aliases.str( ).c_str( ) )
		<< endl;
	}
    }

    page_to_char( buf.str( ).c_str( ), ch );
}

CMDRUN( commands )
{
    int flags = 0;
    DLString arg, args = constArguments; 
    
    arg = args.getOneArgument( );
    
    if (arg_oneof( arg, "show", "показать" )) {
	show_matched_commands( ch, args );
	return;
    }

    if (arg.empty( ) || arg_oneof( arg, "hints", "подсказки" ))
	SET_BIT(flags, FCMD_HINTS);
    
    if (arg_oneof( arg, "table", "таблица" ))
	SET_BIT(flags, FCMD_TABLE);

    if (arg_oneof( arg, "aliases", "синонимы" ))
	SET_BIT(flags, FCMD_ALIASES);
    
    if (!flags) {
	ch->println( "Ты можешь задать один из видов таблицы команд: aliases, hints, table,\r\n"
	             "либо отобразить только несколько команд: commands show <имя команды>." );
	return;
    }
    
    show_commands( ch, flags );
}

/*
 * 'wizhelp'
 */
CMDRUN( wizhelp )
{
    show_commands( ch, FCMD_WIZARD );
}
