/* $Id: commandplugin.cpp,v 1.1.2.3 2008/03/22 00:07:27 rufina Exp $
 *
 * ruffina, 2007
 */

#include "commandplugin.h"
#include "commandmanager.h"
#include "commandpluginloader.h"

bool CommandPlugin::saveCommand() const
{
    getLoader()->saveCommand(Pointer(this));
    return true;
}

void CommandPlugin::initialization( )
{
    // getLoader() (commandPluginLoader global) can be NULL mid 'plug reload
    // most': reloadNonCritical() destructs [command] (nulling the loader in its
    // dtor) and may re-init a command template from another .so (e.g. religion's
    // altar cmd) before the loader plugin is reconstructed. Dereferencing it here
    // SIGSEGV'd the server. Guard mirrors the configReg check in configurable.cpp
    // (code #659): boot order is always correct, so the XML load is skipped only
    // in that reload window. The command still registers and stays callable.
    if (getLoader())
        getLoader()->loadCommand( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
    linkWrapper();
}

void CommandPlugin::destruction( )
{
    unlinkWrapper();
//    getLoader()->saveCommand( Pointer( this ) );
    commandManager->unregistrate( Pointer( this ) );
}

CommandLoader * CommandPlugin::getLoader( ) const
{
    return commandPluginLoader;
}

