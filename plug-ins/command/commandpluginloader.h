#ifndef COMMANDPLUGINLOADER_H
#define COMMANDPLUGINLOADER_H

#include "commandloader.h"

/** Default command loader that reads from 'commands' folder. */
class CommandPluginLoader : public CommandLoader {
public:
    typedef ::Pointer<CommandPluginLoader> Pointer;
    
    CommandPluginLoader();
    virtual ~CommandPluginLoader();
    
    virtual DLString getTableName( ) const; 
private:
    static const DLString TABLE_NAME;
};

extern CommandPluginLoader *commandPluginLoader;


#endif