/* $Id$
 *
 * ruffina, 2004
 */
#include "commandhelp.h"
#include "command.h"
#include "commandmanager.h"
#include "character.h"

const DLString CommandHelp::TYPE = "CommandHelp";
static const DLString LABEL_COMMAND = "cmd";

bool CommandHelp::visible( Character *ch ) const
{
    if (!HelpArticle::visible( ch ))
        return false;
    
    if (getLevel( ) <= 0)
        return true;

    return command->available( ch );
}

void CommandHelp::save() const
{
    if (command) {
        const XMLCommand *cmd = command.getDynamicPointer<XMLCommand>();
        if (cmd)
            commandManager->save(cmd);
    }
}

void CommandHelp::setCommand( Command::Pointer command )
{
    StringSet::const_iterator r;

    this->command = command;
    
    keywords.insert( command->getName( ) );    
    keywords.insert( command->getRussianName( ) );    
    command->getAliases( ).toSet( keywords );
    command->getRussianAliases( ).toSet( keywords );

    labels.fromString(
        command->getCommandCategory().names());
    labels.insert(LABEL_COMMAND);

    if (!keyword.empty( ))
        keywords.fromString( keyword.toLower() );

    for (r = ref.begin( ); r != ref.end( ); r++) {
        Command::Pointer cmd = commandManager->findExact( *r );

        if (cmd) 
            keywords.fromString( cmd->getHelp()->getKeyword().toLower() );
    }
    
    fullKeyword = keywords.toString( ).toUpper( );

    for (r = refby.begin( ); r != refby.end( ); r++) {
        Command::Pointer cmd = commandManager->findExact( *r );

        if (cmd && cmd->getHelp( ))
            cmd->getHelp( )->addKeyword( fullKeyword );
    }
   
    if (!empty( ))
        helpManager->registrate( Pointer( this ) );
}

void CommandHelp::unsetCommand( )
{
    if (!empty( ))
        helpManager->unregistrate( Pointer( this ) );
    
    /* XXX remove refby keyword */

    command.clear( );
    keywords.clear();
    fullKeyword = "";
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
        fRusCmd = ch->getConfig( )->rucommands;
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



