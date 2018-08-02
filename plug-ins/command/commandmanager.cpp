/* $Id: commandmanager.cpp,v 1.1.6.4.6.5 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CommandManager by NoFate
 */

#include "commandmanager.h"
#include "commandinterpreter.h"
#include "commandhelp.h"

#include "logstream.h"
#include "xmlvector.h"

#include "character.h"

/*-----------------------------------------------------------------------
 * CommandList
 *-----------------------------------------------------------------------*/
CommandList::const_iterator CommandList::findExact( const DLString& name ) const
{
    const_iterator ipos;

    for (ipos = begin( ); ipos != end( ); ipos++)
	if ((*ipos)->getName( ) == name) 
	    return ipos;

    return end( );
}

Command::Pointer CommandList::chooseCommand( Character *ch, const DLString &name ) const
{
    const_iterator i;

    for (i = begin( ); i != end( ); i++) {
	Command::Pointer pCommand = *i;

	if (pCommand->available( ch ) && pCommand->matches( name )) 
	    return pCommand;
    }

    return Command::Pointer( );
}

static bool compare( Command::Pointer a, Command::Pointer b )
{
    return a->compare( **b );
}

void CommandList::sort( )
{
    std::list<Command::Pointer>::sort( compare );
}

/*-----------------------------------------------------------------------
 * CommandManager
 *-----------------------------------------------------------------------*/
const DLString CommandManager::TABLE_NAME = "commands";
const DLString CommandManager::PRIO_FILE = "cmdpriority";
CommandManager* commandManager = NULL;

CommandManager::CommandManager( ) 
{
    checkDuplicate( commandManager );
    commandManager = this;
}

CommandManager::~CommandManager( )
{
    commandManager = NULL;
}

void CommandManager::initialization( )
{
    loadPriorities( );
    InterpretLayer::initialization( );
}

void CommandManager::destruction( )
{
    InterpretLayer::destruction( );
}

void CommandManager::load( XMLCommand::Pointer command )
{
    CommandLoader *loader = command->getLoader( );
    if (loader == NULL) {
        LogStream::sendError( ) << "No loader for command " << command->getName( ) << endl;
        return;
    }
    LogStream::sendNotice( ) << "Loader for command " << command->getName( ) << ": " << loader->getTableName( ) << endl;
    command->getLoader( )->loadCommand( command );
}

void CommandManager::save( XMLCommand::Pointer command )
{
    command->getLoader( )->saveCommand( command );
}

void CommandManager::registrate( Command::Pointer command )
{
    commands.push_back( command );
    commands.sort( );

    if (command->getHelp( ))
	command->getHelp( )->setCommand( command );
}

void CommandManager::unregistrate( Command::Pointer command )
{
    if (command->getHelp( ))
	command->getHelp( )->unsetCommand( );
    
    commands.remove( command );
}

void CommandManager::loadPriorities( )
{
    XMLVectorBase<XMLString> v;                                                 

    if (!loadXML( &v, PRIO_FILE )) {
	LogStream::sendError( ) << "Command priorities file not found!" << endl;
	return;
    }
    
    for (unsigned int i = 0; i < v.size( ); i++)
        priorities[v[i].getValue( )] = i;

    LogStream::sendNotice( ) 
	<< "Loaded " << priorities.size( ) << " command priorities" << endl;
}

DLString CommandManager::getTableName( ) const
{
    return TABLE_NAME;
}


void CommandManager::putInto( )
{
    interp->put( this, CMDP_FIND, 10 );	
}

bool CommandManager::process( InterpretArguments &iargs )
{
    iargs.pCommand = commands.chooseCommand( iargs.ch, iargs.cmdName );

    if (iargs.pCommand)
	iargs.advance( );

    return true;
}

Command::Pointer CommandManager::findExact( const DLString &cmdName ) const
{
    CommandList::const_iterator c = commands.findExact( cmdName ); 

    if (c != commands.end( )) 
	return *c;
    else
	return Command::Pointer( );
}

CommandManager::CategoryMap CommandManager::getCategorizedCommands( ) const
{
    CategoryMap cats;

    CommandList::const_iterator cmd;
    for (cmd = commands.begin( ); cmd != commands.end( ); cmd++) {
        CategoryMap::iterator c = cats.find( (*cmd)->getCommandCategory( ) );

        if (c == cats.end( )) {
            CategoryCommands list;
            list.push_back(*cmd);
            cats[(*cmd)->getCommandCategory()] = list;
        } else {
            c->second.push_back(*cmd);
        }
    }

    return cats;
}

/*-----------------------------------------------------------------------
 * CommandLoader
 *-----------------------------------------------------------------------*/
const DLString CommandLoader::NODE_NAME = "Command";

void CommandLoader::loadCommand( XMLCommand::Pointer command )
{
    loadXML( *command, command->getName( ) );
}

void CommandLoader::saveCommand( XMLCommand::Pointer command )
{
    saveXML( *command, command->getName( ) );
}

DLString CommandLoader::getNodeName( ) const
{
    return NODE_NAME;
}

