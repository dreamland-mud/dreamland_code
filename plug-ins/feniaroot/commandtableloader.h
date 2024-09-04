#ifndef COMMANDTABLELOADER_H
#define COMMANDTABLELOADER_H

#include "xmltableloaderplugin.h"


/** 
 * A loader that can load/save a single command and also load and instantiate 
 * all commands from XMLs in a catalog. This specific implementation loads
 * from 'commands/fenia' folder.
*/

TABLE_LOADER_DECL(CommandTableLoader)


#endif