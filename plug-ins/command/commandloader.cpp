#include "commandloader.h"
#include "commandplugin.h"

/*-----------------------------------------------------------------------
 * CommandLoader
 *-----------------------------------------------------------------------*/
const DLString CommandLoader::NODE_NAME = "Command";

void CommandLoader::loadCommand( CommandPlugin::Pointer command )
{
    loadXML( *command, command->getName( ) );
}

void CommandLoader::saveCommand( CommandPlugin::Pointer command )
{
    saveXML( *command, command->getName( ) );
}

DLString CommandLoader::getNodeName( ) const
{
    return NODE_NAME;
}

