#include "commandpluginloader.h"
#include "plugininitializer.h"

CommandPluginLoader * commandPluginLoader = 0;

const DLString CommandPluginLoader::TABLE_NAME = "commands";

CommandPluginLoader::CommandPluginLoader( ) 
{
    commandPluginLoader = this;
}

CommandPluginLoader::~CommandPluginLoader( ) 
{
    commandPluginLoader = NULL;
}

DLString CommandPluginLoader::getTableName( ) const
{
    return TABLE_NAME;
}

PluginInitializer<CommandPluginLoader> commandPluginLoader_init(INITPRIO_CMDLOADERS);
