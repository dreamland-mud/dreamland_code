#include "outofband.h"
#include "character.h"
#include "descriptor.h"
#include "logstream.h"

void OutOfBandCommand::initialization( )
{
    outOfBandManager->registrate( Pointer( this ) );
}

void OutOfBandCommand::destruction( ) 
{
    outOfBandManager->unregistrate( Pointer( this ) );
}

OutOfBandManager* outOfBandManager = NULL;

OutOfBandManager::OutOfBandManager( ) 
{
    checkDuplicate( outOfBandManager );
    outOfBandManager = this;
}

OutOfBandManager::~OutOfBandManager( )
{
    outOfBandManager = NULL;
}

void OutOfBandManager::registrate( OutOfBandCommand::Pointer cmd )
{
    commands.insert(make_pair(cmd->getCommandType(), cmd));
}

void OutOfBandManager::unregistrate( OutOfBandCommand::Pointer cmd )
{
    pair<Commands::iterator, Commands::iterator> range = commands.equal_range(cmd->getCommandType());
    for (Commands::iterator c = range.first; c != range.second; ++c)
        if (c->second == cmd) {
            commands.erase(c);
            break;
        }
}

void OutOfBandManager::run( const DLString &commandType, const OutOfBandArgs &args ) const
{
    pair<Commands::const_iterator, Commands::const_iterator> range = commands.equal_range(commandType);
    for (Commands::const_iterator c = range.first; c != range.second; ++c) 
        c->second->run(args);
}

void OutOfBandManager::initialization( )
{
}

void OutOfBandManager::destruction( )
{
}

