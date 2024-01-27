#include "commandtableloader.h"
#include "dreamland.h"

/*
CommandTableLoader *commandTableLoader = 0;
const DLString CommandTableLoader::TABLE_NAME = "fenia";
const DLString CommandTableLoader::NODE_NAME = "Command";

CommandTableLoader::CommandTableLoader()
{
    commandTableLoader = this;
}

CommandTableLoader::~CommandTableLoader()
{
    commandTableLoader = 0;
}

DLString CommandTableLoader::getTablePath( ) const
{
    DLDirectory cmdDir(dreamland->getTableDir(), "commands");
    return cmdDir.getPath();
}

DLString CommandTableLoader::getTableName( ) const
{
    return TABLE_NAME;
}

DLString CommandTableLoader::getNodeName() const
{
    return NODE_NAME;
}
*/
TABLE_LOADER_IMPL(CommandTableLoader, "commands/fenia", "Command");
