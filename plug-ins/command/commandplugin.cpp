/* $Id: commandplugin.cpp,v 1.1.2.3 2008/03/22 00:07:27 rufina Exp $
 *
 * ruffina, 2007
 */

#include "commandplugin.h"
#include "commandmanager.h"

void CommandPlugin::initialization( )
{
    commandManager->load( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

void CommandPlugin::destruction( )
{
    commandManager->save( Pointer( this ) );
    commandManager->unregistrate( Pointer( this ) );
}

