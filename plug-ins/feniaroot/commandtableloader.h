#ifndef COMMANDTABLELOADER_H
#define COMMANDTABLELOADER_H

#include "xmltableloaderplugin.h"


/** 
 * A loader that can load/save a single command and also load and instantiate 
 * all commands from XMLs in a catalog. This specific implementation loads
 * from 'commands/fenia' folder.
*/
/*
class CommandTableLoader : public XMLTableLoaderPlugin {
public:
    typedef ::Pointer<CommandTableLoader> Pointer;

    CommandTableLoader();
    virtual ~CommandTableLoader();

    virtual DLString getTableName() const; 
    virtual DLString getNodeName() const;
    virtual DLString getTablePath() const;

private:
    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
};

extern CommandTableLoader *commandTableLoader;
*/

TABLE_LOADER_DECL(CommandTableLoader)


#endif