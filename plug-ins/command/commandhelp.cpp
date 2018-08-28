/* $Id$
 *
 * ruffina, 2004
 */
#include "commandhelp.h"
#include "command.h"
#include "commandmanager.h"
#include "character.h"

const DLString CommandHelp::TYPE = "CommandHelp";

bool CommandHelp::visible( Character *ch ) const
{
    if (!HelpArticle::visible( ch ))
	return false;
    
    if (getLevel( ) <= 0)
	return true;

    return command->available( ch );
}

void CommandHelp::setCommand( Command::Pointer command )
{
    StringSet kwd;
    StringSet::const_iterator r;

    this->command = command;
    
    kwd.insert( command->getName( ) );    
    kwd.insert( command->getRussianName( ) );    
    command->getAliases( ).toSet( kwd );
    command->getRussianAliases( ).toSet( kwd );
    
    if (!keyword.empty( ))
	kwd.fromString( keyword );

    for (r = ref.begin( ); r != ref.end( ); r++) {
	Command::Pointer cmd = commandManager->findExact( *r );

	if (cmd) 
	    kwd.fromString( cmd->getHelp( )->getKeyword( ) );
    }
    
    fullKeyword = kwd.toString( ).toUpper( );

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



bool CommandHelp::toXML( XMLNode::Pointer &parent ) const
{
    return XMLHelpArticle::toXML( parent ); 
}

