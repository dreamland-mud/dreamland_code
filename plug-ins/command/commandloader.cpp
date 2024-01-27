#include "commandloader.h"

/*-----------------------------------------------------------------------
 * CommandLoader
 *-----------------------------------------------------------------------*/
const DLString CommandLoader::NODE_NAME = "Command";

bool CommandLoader::loadCommand( Command::Pointer command )
{
    return loadXML( *command, command->getName( ) );
}

bool CommandLoader::saveCommand( Command::Pointer command )
{
    return saveXML( *command, command->getName( ) );
}

DLString CommandLoader::getNodeName( ) const
{
    return NODE_NAME;
}

