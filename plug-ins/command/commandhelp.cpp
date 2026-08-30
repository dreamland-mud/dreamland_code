/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "commandhelp.h"
#include "commandplugin.h"
#include "commandmanager.h"
#include "character.h"
#include "player_utils.h"
#include "helpmanager.h"
#include "l10n.h"

const DLString CommandHelp::TYPE = "CommandHelp";
static const DLString LABEL_COMMAND = "cmd";

bool CommandHelp::visible( Character *ch ) const
{
    if (!HelpArticle::visible( ch ))
        return false;

    if (text.get(RU).empty())
        return false;
    
    if (getLevel( ) <= 0)
        return true;

    return command->available( ch );
}

void CommandHelp::save() const
{
    if (command && command->saveCommand())
        return;

    LogStream::sendError() << "Failed to save command help on command " << command->getName() << endl;
}

DLString CommandHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc")
        return command->getRussianName().upperFirstCharacter();

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    if (!title.get(RU).empty() || !command)
        return MarkupHelpArticle::getTitle(label);

    // One name. This non-lang path is a fallback (website/toc); prefer the
    // Russian name, fall back to the English one when a command has no RU name.
    buf << "Команда {c";
    if (!command->getRussianName().empty())
        buf << command->getRussianName();
    else
        buf << command->getName();
    buf << "{x";
    return buf.str();
}

/**
 * Same title, composed in the viewer's language. CommandHelp had no per-language
 * title, so every viewer saw "Команда <ru>, <en>" -- two alphabets, Russian
 * first. A command is resolvable by its name in every language (setCommand
 * registers EN/RU/UA names and aliases), so one name in the reader's language
 * round-trips fine.
 */
DLString CommandHelp::getTitle(const DLString &label, lang_t lang) const
{
    if (!command || !title.get(RU).empty())
        return MarkupHelpArticle::getTitle(label, lang);

    if (label == "title")
        return DLString::emptyString;

    DLString name = command->getNameFor(lang);

    if (label == "toc")
        return name.upperFirstCharacter();

    return help_title_fmt(lang, _("Команда {c%1$s{x"), name);
}

void CommandHelp::setCommand( Command::Pointer command )
{
    StringSet::const_iterator r;

    this->command = command;
    
    addAutoKeyword(command->getName());
    addAutoKeyword(command->getRussianName());    
    addAutoKeyword(command->getNameFor(UA));
    addAutoKeyword(command->aliases.get(EN).split(" "));
    addAutoKeyword(command->aliases.get(RU).split(" "));
    addAutoKeyword(command->aliases.get(UA).split(" "));

    labels.addTransient(
        command->getCommandCategory().names());
    labels.addTransient(LABEL_COMMAND);

    for (r = ref.begin( ); r != ref.end( ); r++) {
        Command::Pointer cmd = commandManager->findExact( *r );

        if (cmd) {
            addAutoKeyword(cmd->getName());
            addAutoKeyword(cmd->getRussianName());
        }
    }

    for (r = refby.begin( ); r != refby.end( ); r++) {
        Command::Pointer cmd = commandManager->findExact( *r );

        if (cmd && cmd->getHelp( )) {
            cmd->getHelp( )->addAutoKeyword(getAllKeywords());
        }
    }
   
    helpManager->registrate( Pointer( this ) );
}

CommandHelp::Pointer CommandHelp::getReferencedBy()
{
    CommandHelp::Pointer refHelp;

    if (!refby.empty()) {
        DLString cmdName = *(refby.begin());
        Command::Pointer cmd = commandManager->findExact(cmdName);
        if (cmd && cmd->getHelp()) {
            refHelp = cmd->getHelp();
        }
    }
    
    return refHelp;
}


void CommandHelp::unsetCommand( )
{
    helpManager->unregistrate( Pointer( this ) );
    
    command.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}


void CommandHelp::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    CommandHelpFormatter( in.str( ).c_str( ), 
                          command 
                        ).run( ch, out );
}

CommandHelpFormatter::CommandHelpFormatter( const char *text, Command::Pointer cmd )
{
    this->text = text;
    this->cmd = cmd;
    lang = LANG_DEFAULT;
}

CommandHelpFormatter::~CommandHelpFormatter( )
{
}

void CommandHelpFormatter::reset( )
{
    HelpFormatter::reset( );
    lang = LANG_DEFAULT;
}

void CommandHelpFormatter::setup( Character *ch )
{
    if (ch)
        lang = Player::displayLang(ch);

    HelpFormatter::setup( ch );
}

/*
 * CMD -> the command's own name in the viewer's language.
 *
 * It used to be a two-way choice, English name or Russian one, which handed a
 * Ukrainian reader the Russian command in all 499 places this keyword appears
 * -- even though commands carry <name l="ua"> (cast / колдовать / чаклувати).
 * getNameFor() already does the per-language pick with a RU-then-EN fallback.
 */
bool CommandHelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (HelpFormatter::handleKeyword( kw, out ))
        return true;
    
    if (kw == "CMD" && cmd) {
        out << cmd->getNameFor( lang );
        return true;
    }
    
    return false;
}



