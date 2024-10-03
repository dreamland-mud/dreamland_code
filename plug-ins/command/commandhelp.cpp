/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "commandhelp.h"
#include "commandplugin.h"
#include "commandmanager.h"
#include "character.h"

const DLString CommandHelp::TYPE = "CommandHelp";
static const DLString LABEL_COMMAND = "cmd";

bool CommandHelp::visible( Character *ch ) const
{
    if (!HelpArticle::visible( ch ))
        return false;

    if (empty())
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
    if (label == "toc") {
        buf << "Команда '" << command->getRussianName() << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    if (!titleAttribute.empty() || !command)
        return MarkupHelpArticle::getTitle(label);

    buf << "Команда {c";
    if (!command->getRussianName().empty())
        buf << command->getRussianName() << "{x, {c";
    buf << command->getName() << "{x";
    return buf.str();
}

void CommandHelp::setCommand( Command::Pointer command )
{
    StringSet::const_iterator r;

    this->command = command;
    
    addAutoKeyword(command->getName());
    addAutoKeyword(command->getRussianName());    
    addAutoKeyword(command->aliases.get(EN).split(" "));
    addAutoKeyword(command->aliases.get(RU).split(" "));

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
    fRusCmd = false;
}

CommandHelpFormatter::~CommandHelpFormatter( )
{
}

void CommandHelpFormatter::reset( )
{
    HelpFormatter::reset( );
    fRusCmd = false;
}

void CommandHelpFormatter::setup( Character *ch )
{
    if (ch) {
        fRusCmd = ch->getConfig( ).rucommands;
    }
    
    HelpFormatter::setup( ch );
}

/*
 * CMD    ->  {lEeng_name{lRрусское_имя{lx
 */
bool CommandHelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (HelpFormatter::handleKeyword( kw, out ))
        return true;
    
    if (kw == "CMD" && cmd) {
        out << (fRusCmd && !cmd->getRussianName( ).empty( )
                        ? cmd->getRussianName( )
                        : cmd->getName( ));
        return true;
    }
    
    return false;
}



