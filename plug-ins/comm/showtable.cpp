/* $Id: showtable.cpp,v 1.1.2.9 2009/09/11 11:24:54 rufina Exp $
 *
 * ruffina, 2004
 */
/*
 *
 * sturm, 2003
 */
#include <iomanip>
#include "commandflags.h"
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
    FCMD_ALIASES = (B),
    FCMD_HINTS = (C),
    FCMD_WIZARD = (D),
    FCMD_SHOW_FIRST_RUS = (E),
    FCMD_IMPORTANT = (F)
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
    list<Command::Pointer>::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );
    bool found = false;

    if (arg.empty( )) {
        ch->println("Использование: {y{lRкоманды показ{lEcommand show{lx{D название{x.");
        return;
    }

    buf << "Найдены такие команды:" << endl << endl;

    for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
        ostringstream aliases;
        Command::Pointer cmd = *c;

        if (cmd->getLevel( ) >= LEVEL_HERO && !ch->is_immortal())
            continue;
        
        if (!cmd->matches( arg ) && !cmd->matchesAlias( arg ))
            continue;
        
        found = true;
        show_aliases( cmd, aliases );
        buf << "Команда {c" << cmd->getName() << "{x, {c" << cmd->getRussianName() << "{x: "
            << cmd->getHint() << endl;

        if (!aliases.str().empty())
            buf << "Синонимы: {D" << aliases.str() << "{x" << endl;

        DLString cat = cmd->getCommandCategory().messages().toLower();
        if (cat.empty())
            cat = "(нет)";
        buf << "Категория {W" << cat << "{x";

        bitstring_t extra = cmd->getExtra();
        REMOVE_BIT(extra, CMD_HIDDEN|CMD_IMPORTANT|CMD_NO_INTERPRET);
        buf << ", можно выполнить {W";
        switch (cmd->getPosition().getValue()) {
            default: buf << "всегда"; break;
            case POS_STANDING: buf << "только стоя и вне боя"; break;
            case POS_FIGHTING: buf << "сражаясь"; break;
            case POS_SITTING: buf << "сидя"; break;
            case POS_RESTING: buf << "на отдыхе"; break;
            case POS_SLEEPING: buf << "во сне"; break;
        }
        
        buf << ".{x" << endl;

        buf << "Эта команда {W" << (extra > 0 ? command_flags.messages(extra, true) : "без особенностей") << "{x";
        if (cmd->getOrder().getValue() != 0)
            buf << ", приказы примут {W" << cmd->getOrder().messages(true) << "{x";

        buf << "." << endl << endl;             
    }

    if (found)
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->printf("Не найдено ни одной команды, начинающейся с '%s'.\r\n", arg.c_str( ));
}

typedef map<DLString, StringList> Categories;

static Categories group_by_categories(Character *ch, int flags)
{
    Categories categories;
    list<Command::Pointer>::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );
    bool fRus = ch->getConfig( ).rucommands;

    categories["info"].push_back("?");

    for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
        Command::Pointer cmd = *c;

        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) >= LEVEL_HERO)
            continue;

        if (IS_SET(flags, FCMD_IMPORTANT) && !cmd->getExtra( ).isSet( CMD_IMPORTANT ))
            continue;
        
        DLString name = (fRus && !cmd->getRussianName().empty()) ? cmd->getRussianName() : cmd->getName();
 
        if (cmd->getCommandCategory().getValue() == 0) {
            categories["misc"].push_back(name);
        } else {
            StringList myCategories(cmd->getCommandCategory().names());
            for (StringList::const_iterator s = myCategories.begin(); s != myCategories.end(); s++)
                categories[*s].push_back(name);
        }
    }

    categories["client"].push_back("!");
    categories["client"].push_back("\\");
    categories["client"].push_back("|");
    return categories;
}

static void show_commands_by_categories( Character *ch, int flags )
{
    ostringstream buf;
    Categories categories = group_by_categories(ch, flags);
    
    for (int i = 0; i < command_category_flags.size; i++) {
        DLString name = command_category_flags.fields[i].name;
        const StringList &commands = categories[name];
        DLString msg = command_category_flags.fields[i].message;
        msg = "{c" + msg.toUpper() + "{x: ";

        if (!commands.empty())
            buf << setiosflags(ios::right) << setw(21) << msg << resetiosflags(ios::right)
                << categories[name].join(" ") << endl;
    }
    buf << "Также смотри {y{lRкоманды список{lEcommand list{x, {y{lRкоманды синоним{lEcommand alias{x и {y{lRкоманды показ{lEcommand show{x." << endl;
    ch->send_to(buf);
}

static void show_commands( Character *ch, int flags )
{
    ostringstream buf;
    list<Command::Pointer>::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );

    if (IS_SET(flags, FCMD_ALIASES|FCMD_HINTS)) {
        buf << fmt( 0, "%-12s | %-17s| %s", 
                    "По-английски", "По-русски", 
                    IS_SET(flags, FCMD_ALIASES) ? "Синонимы" : "Справка" ) 
            << endl
            << "-------------+------------------+--------------------------------------------" 
            << endl;

        for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
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

        buf << endl;
        if (IS_SET(flags, FCMD_ALIASES)) 
            buf << "Также смотри {y{lRкоманды{lEcommand{x, {y{lRкоманды список{lEcommand list{x и {y{lRкоманды показ{lEcommand show{x." << endl;
        else
            buf << "Также смотри {y{lRкоманды{lEcommad{x, {y{lRкоманды синоним{lEcommand alias{x и {y{lRкоманды показ{lEcommand show{x." << endl;

    }
    else if (IS_SET(flags, FCMD_WIZARD)) {
        buf << fmt( 0, "%-12s | %-45s | %s", "По-английски", "Справка", "Синонимы" )
            << endl
            << "-------------+-----------------------------------------------+---------------" 
            << endl;
        for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
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

    if (arg.empty( )) {
        show_commands_by_categories(ch, flags);
        return;
    }
 
    if (arg_is_list(arg))
        SET_BIT(flags, FCMD_HINTS);

    if (arg_oneof( arg, "aliases", "синонимы" ))
        SET_BIT(flags, FCMD_ALIASES);
    
    if (!flags) {
        ch->println("Использование:\n"
        "{lRкоманды{lEcommands{x - таблица всех команд\n"
        "{lRкоманды список{lEcommands list{x - список команд с краткой справкой\n"
        "{lRкоманды синонимы{lEcommands aliases{x - список команд и их синонимов\n"
        "{lRкоманды показ{lEcommands show{x слово - показать синонимы и справку по команде.\n");
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
