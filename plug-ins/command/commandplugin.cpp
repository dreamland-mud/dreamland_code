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

