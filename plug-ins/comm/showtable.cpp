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
#include "l10n.h"

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
        ch->pecho(_("Использование: {yкоманды показ{D название{x."));
        return;
    }

    buf << fmt(ch, _("Найдены такие команды:")) << endl << endl;

    for (auto &c: commandManager->getCommands()) {
        ostringstream aliases;
        Command::Pointer cmd = *c;

        if (cmd->getLevel( ) >= LEVEL_HERO && !ch->is_immortal())
            continue;

        if (!cmd->matches( arg ))
            continue;

        found = true;

        // Header: name, hint in player's target lang
        buf << fmt(ch, _("Команда {c%1$s{x: %2$s"),
                       cmd->name.get(lang).c_str(), cmd->hint.get(lang).c_str()) << endl;

        // Output names and aliases in all languages
        buf << fmt(ch, _("Синонимы: {D%1$s{x"), show_aliases(cmd, lang).c_str()) << endl;

        // Category, position -- flag words come from the (now externalized) tables
        DLString cat = cmd->getCommandCategory().messages(false, '1', lang).toLower();
        if (cat.empty())
            cat = _("(нет)").getMessage(ch);
        buf << fmt(ch, _("Категория {W%1$s{x"), cat.c_str());

        bitstring_t extra = cmd->getExtra();
        REMOVE_BIT(extra, CMD_HIDDEN|CMD_NO_INTERPRET);
        DLString posWord;
        switch (cmd->getPosition().getValue()) {
            default: posWord = _("всегда").getMessage(ch); break;
            case POS_STANDING: posWord = _("только стоя и вне боя").getMessage(ch); break;
            case POS_FIGHTING: posWord = _("сражаясь").getMessage(ch); break;
            case POS_SITTING: posWord = _("сидя").getMessage(ch); break;
            case POS_RESTING: posWord = _("на отдыхе").getMessage(ch); break;
            case POS_SLEEPING: posWord = _("во сне").getMessage(ch); break;
        }
        buf << fmt(ch, _(", можно выполнить {W%1$s.{x"), posWord.c_str()) << endl;

        // Command flags and order flags
        DLString flagMsg = extra > 0 ? command_flags.messages(extra, true, '1', lang)
                                     : _("без особенностей").getMessage(ch);
        buf << fmt(ch, _("Эта команда {W%1$s{x"), flagMsg.c_str());
        if (cmd->getOrder().getValue() != 0)
            buf << fmt(ch, _(", приказы примут {W%1$s{x"),
                           cmd->getOrder().messages(true, '1', lang).c_str());

        buf << "." << endl << endl;
    }

    if (found)
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->pecho(_("Не найдено ни одной команды, начинающейся с '%s'."), arg.c_str( ));
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
    lang_t lang = Player::lang(ch);
    Categories categories = group_by_categories(ch);

    for (int i = 0; i < command_category_flags.size; i++) {
        DLString name = command_category_flags.fields[i].name;
        const StringList &commands = categories[name];
        DLString msg = command_category_flags.message(command_category_flags.fields[i].value, '1', lang);
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


    // Column labels localized; the %-Ns padding realigns them to the data columns
    // regardless of the word's length.
    buf << fmt( ch, "%-12s | %-45s| %s",
                _("Название").getMessage(ch).c_str(),
                _("Справка").getMessage(ch).c_str(),
                _("Синонимы").getMessage(ch).c_str())
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

    ch->pecho(_("Использование:\n"
    "{y{hcкоманды{x        - таблица всех команд\n"
    "{y{hcкоманды список{x - список команд с краткой справкой\n"
    "{yкоманды показ{x слово - показать синонимы и справку по команде.\n"));
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
