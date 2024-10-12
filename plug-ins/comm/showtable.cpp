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
#include "player_utils.h"
#include "merc.h"

#include "def.h"

/*
 * 'commands'
 */

// Output all command aliases and command names in all languages except for 'lang'
static DLString show_aliases(Command::Pointer &cmd, lang_t lang)
{
    StringSet aliases;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t l = (lang_t)i;

        if (l != lang)
            aliases.insert(cmd->name.get(l));

        for (auto &alias: cmd->aliases.get(l).split(" "))
            aliases.insert(alias);
    }

    return aliases.toString();
}

static void show_matched_commands( Character *ch, const DLString &arg )
{
    ostringstream buf;
    bool found = false;
    lang_t lang = Player::lang(ch);

    if (arg.empty( )) {
        ch->pecho("Использование: {yкоманды показ{D название{x.");
        return;
    }

    buf << "Найдены такие команды:" << endl << endl;

    for (auto &c: commandManager->getCommands()) {
        ostringstream aliases;
        Command::Pointer cmd = *c;

        if (cmd->getLevel( ) >= LEVEL_HERO && !ch->is_immortal())
            continue;
        
        if (!cmd->matches( arg ))
            continue;
        
        found = true;

        // Header: name, hint in player's target lang
        buf << "Команда {c" << cmd->name.get(lang) << "{x: " << cmd->hint.get(lang) << endl;

        // Output names and aliases in all languages
        buf << "Синонимы: {D" << show_aliases(cmd, lang) << "{x" << endl;

        // Category, position
        DLString cat = cmd->getCommandCategory().messages().toLower();
        if (cat.empty())
            cat = "(нет)";
        buf << "Категория {W" << cat << "{x";

        bitstring_t extra = cmd->getExtra();
        REMOVE_BIT(extra, CMD_HIDDEN|CMD_NO_INTERPRET);
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

        // Command flags and order flags
        buf << "Эта команда {W" << (extra > 0 ? command_flags.messages(extra, true) : "без особенностей") << "{x";
        if (cmd->getOrder().getValue() != 0)
            buf << ", приказы примут {W" << cmd->getOrder().messages(true) << "{x";

        buf << "." << endl << endl;             
    }

    if (found)
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->pecho("Не найдено ни одной команды, начинающейся с '%s'.", arg.c_str( ));
}

typedef map<DLString, StringList> Categories;

static Categories group_by_categories(Character *ch)
{
    Categories categories;
    lang_t lang = Player::lang(ch);

    categories["info"].push_back("?");

    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;

        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) >= LEVEL_HERO)
            continue;

        DLString name = cmd->name.get(lang);
 
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

static void show_commands_by_categories( Character *ch)
{
    ostringstream buf;
    Categories categories = group_by_categories(ch);
    
    for (int i = 0; i < command_category_flags.size; i++) {
        DLString name = command_category_flags.fields[i].name;
        const StringList &commands = categories[name];
        DLString msg = command_category_flags.fields[i].message;
        msg = "{c" + msg.toUpper() + "{x: ";

        if (!commands.empty())
            buf << setiosflags(ios::right) << setw(21) << msg << resetiosflags(ios::right)
                << categories[name].join(" ") << endl;
    }

    buf << "Также смотри {y{hcкоманды список{x и {yкоманды показать{D слово{x." << endl;
    ch->send_to(buf);
}

static void show_commands_list( Character *ch )
{
    ostringstream buf;
    lang_t lang = Player::lang(ch);


    buf << fmt( 0, "%-12s | %-45s| %s", 
                "Название", "Справка", "Синонимы")
        << endl
        << "-------------+------------------+--------------------------------------------" 
        << endl;

    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;
        
        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) >= LEVEL_HERO)
            continue;

        DLString name = cmd->name.get(lang);
        DLString hint = cmd->hint.get(lang);
        DLString aliases = show_aliases(cmd, lang);
                
        buf << fmt( 0, "{c%-12s {x: %-45s: %s",
                        name.c_str(),
                        hint.c_str(),
                        aliases.c_str() )
            << endl;
    }

    buf << endl;
    buf << "Также смотри {y{hcкоманды{x и {yкоманды показ {Dслово{x." << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}

CMDRUN( commands )
{
    DLString arg, args = constArguments; 
    
    arg = args.getOneArgument( );
    
    if (arg_is_show(arg)) {
        show_matched_commands( ch, args );
        return;
    }

    if (arg.empty( )) {
        show_commands_by_categories(ch);
        return;
    }
 
    if (arg_is_list(arg)) {
        show_commands_list(ch);
        return;
    }

    ch->pecho("Использование:\n"
    "{y{hcкоманды{x        - таблица всех команд\n"
    "{y{hcкоманды список{x - список команд с краткой справкой\n"
    "{yкоманды показ{x слово - показать синонимы и справку по команде.\n");
}

/*
 * 'wizhelp'
 */
CMDRUN( wizhelp )
{
    ostringstream buf;

    // TODO rework when most wizhelp commands have UA, RU aliases
    for (auto &c: commandManager->getCommands()) {
        Command::Pointer cmd = *c;

        if (!cmd->visible( ch ))
            continue;
        
        if (cmd->getLevel( ) <  LEVEL_HERO)
            continue;

        buf << fmt( 0, "{c%-12s {x: %-45s\n",
                        cmd->getName( ).c_str( ),
                        cmd->getHint( ).c_str( ));        
    }

    page_to_char( buf.str( ).c_str( ), ch );
}
