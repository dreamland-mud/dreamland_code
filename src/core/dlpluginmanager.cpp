/* $Id$
 *
 * ruffina, 2004
 */
#include "dlpluginmanager.h"
#include "dreamland.h"


DLString DLPluginManager::getTablePath( ) const
{
    return dreamland->getLibexecDir( ).getPath( );
}

