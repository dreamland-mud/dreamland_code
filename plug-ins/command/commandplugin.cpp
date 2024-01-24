/* $Id: commandplugin.cpp,v 1.1.2.3 2008/03/22 00:07:27 rufina Exp $
 *
 * ruffina, 2007
 */

#include "logstream.h"
#include "commandplugin.h"
#include "commandmanager.h"
#include "feniamanager.h"

void CommandPlugin::initialization( )
{
    commandManager->load( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );

    if (FeniaManager::wrapperManager) {
        FeniaManager::wrapperManager->linkWrapper(this);
        if (wrapper)
            LogStream::sendNotice() << "Fenia command: linked wrapper for " << getName() << endl;
    }
}

void CommandPlugin::destruction( )
{
    if (FeniaManager::wrapperManager)
        if (wrapper && wrapper->getHandler()->getType() == "FeniaCommandWrapper")
            extractWrapper(false);
    
//    commandManager->save( Pointer( this ) );
    commandManager->unregistrate( Pointer( this ) );
}

CommandLoader * CommandPlugin::getLoader( ) const
{
    return commandManager;
}
