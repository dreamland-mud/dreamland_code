/* $Id: commandmanager.cpp,v 1.1.6.4.6.5 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CommandManager by NoFate
 */
#include <algorithm>

#include "commandmanager.h"
#include "commandinterpreter.h"
#include "commandhelp.h"

#include "logstream.h"
#include "xmlvector.h"
#include "dbio.h"

#include "character.h"

/*-----------------------------------------------------------------------
 * CommandList
 *-----------------------------------------------------------------------*/
Command::Pointer CommandList::findExact( const DLString& name ) const
{
    list<Command::Pointer>::const_iterator ipos;

    if (name.isRussian( )) {
        for (ipos = commands_ru.begin( ); ipos != commands_ru.end( ); ipos++) {
            if ((*ipos)->getRussianName( ) == name) 
                return *ipos;

            const XMLStringList &aliases = (*ipos)->getRussianAliases( );
            if (find(aliases.begin( ), aliases.end( ), name) != aliases.end( ))
                return *ipos;
        }

        return Command::Pointer( );
    }

    for (ipos = commands.begin( ); ipos != commands.end( ); ipos++) {
        if ((*ipos)->getName( ) == name) 
            return *ipos;

        const XMLStringList &aliases = (*ipos)->getAliases( );
        if (find(aliases.begin( ), aliases.end( ), name) != aliases.end( ))
            return *ipos;
    }

    return Command::Pointer( );
}

Command::Pointer CommandList::chooseCommand( Character *ch, const DLString &name ) const
{
    list<Command::Pointer>::const_iterator i;
    const list<Command::Pointer> &mylist = name.isRussian( ) ? commands_ru : commands;

    for (i = mylist.begin( ); i != mylist.end( ); i++) {
        Command::Pointer pCommand = *i;

        if (pCommand->available( ch ) && pCommand->matches( name ))  {
            return pCommand;
        }
    }

    for (i = mylist.begin( ); i != mylist.end( ); i++) {
        Command::Pointer pCommand = *i;

        if (pCommand->available( ch ) && pCommand->matchesAlias( name ))  {
            return pCommand;
        }
    }

    return Command::Pointer( );
}

static bool compare( Command::Pointer a, Command::Pointer b )
{
    return commandManager->compare( **a, **b, false );
}

static bool compare_ru( Command::Pointer a, Command::Pointer b )
{
    return commandManager->compare( **a, **b, true );
}

void CommandList::add( Command::Pointer &cmd )
{
    commands.push_back( cmd ); 
    commands.sort( compare );
    
    if (!cmd->getRussianName( ).empty( )) {
        commands_ru.push_back( cmd );
        commands_ru.sort( compare_ru );
    }
}

void CommandList::remove( Command::Pointer &cmd )
{
    commands.remove( cmd );
    commands_ru.remove( cmd );
}

/*-----------------------------------------------------------------------
 * CommandManager
 *-----------------------------------------------------------------------*/
const DLString CommandManager::TABLE_NAME = "commands";
const DLString CommandManager::PRIO_FILE = "cmdpriority";
const DLString CommandManager::PRIO_FILE_RU = "cmdpriority_ru";
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

// Do a one-off save of Russian command priorities, if the file doesn't yet exist.
void CommandManager::savePrioritiesRU( )
{
    // Do nothing if the file is already there.
    DBIO dbio( getTablePath( ), getTableName( ) );
    dbio.open( );
    if (dbio.getEntryAsFile( PRIO_FILE_RU ).exist( )) {
        return;
    }

    // Collect all Russian aliases in the order they exist now.
    XMLVectorBase<XMLString> prio;                                                 
    list<Command::Pointer>::const_iterator cmd;
    for (cmd = commands.getCommandsRU( ).begin( ); cmd != commands.getCommandsRU( ).end( ); cmd++) {
        const DLString &rname = (*cmd)->getRussianName( );
        if (!rname.empty( ))
            prio.push_back( rname );

        for (XMLStringList::const_iterator a = (*cmd)->getRussianAliases( ).begin( ); 
                a != (*cmd)->getRussianAliases( ).end( ); 
                a++)
            if (*a != rname)
                prio.push_back( *a );
    }


    LogStream::sendNotice( ) << "Saving " << prio.size( ) << " Russian aliases in priority order." << endl;
    // Save the file to disc.
    saveXML( &prio, PRIO_FILE_RU, true );
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
    command->getLoader( )->loadCommand( command );
}

void CommandManager::save( XMLCommand::Pointer command )
{
    command->getLoader( )->saveCommand( command );
}

void CommandManager::registrate( Command::Pointer command )
{
    commands.add( command );

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
    XMLVectorBase<XMLString> v, rv;                                                 

    if (!loadXML( &v, PRIO_FILE )) {
        LogStream::sendError( ) << "Command priorities file not found!" << endl;
        return;
    }

    loadXML( &rv, PRIO_FILE_RU );

    for (unsigned int i = 0; i < v.size( ); i++)
        priorities[v[i].getValue( )] = i;

    for (unsigned int i = 0; i < rv.size( ); i++)
        priorities_ru[rv[i].getValue( )] = i;

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
    return commands.findExact( cmdName );
}

CommandManager::CategoryMap CommandManager::getCategorizedCommands( ) const
{
    CategoryMap cats;

    list<Command::Pointer>::const_iterator cmd;
    for (cmd = commands.getCommands( ).begin( ); cmd != commands.getCommands( ).end( ); cmd++) 
        cats[(*cmd)->getCommandCategory()].push_back(*cmd);

    return cats;
}

bool CommandManager::compare( const Command &a, const Command &b, bool fRussian ) const
{       
    Priorities::const_iterator i_a, i_b, i_end;
    const Priorities &prio = fRussian ? priorities_ru : priorities;

    i_a = prio.find( fRussian ? a.getRussianName( ) : a.getName( ) );
    i_b = prio.find( fRussian ? b.getRussianName( ) : b.getName( ) );
    i_end = prio.end( );

    if (i_a != i_end && i_b != i_end)
        return (i_a->second < i_b->second);

    return (i_a != i_end);  
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
//    saveXML( *command, command->getName( ) );
}

DLString CommandLoader::getNodeName( ) const
{
    return NODE_NAME;
}

