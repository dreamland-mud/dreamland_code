/* $Id$
 *
 * ruffina, 2004
 */

#include "dreamland.h"

#include "dlxmlloader.h"

DLString DLXMLLoader::getTablePath( ) const
{
    return dreamland->getTableDir( ).getPath( );
}

DLString DLXMLTableLoader::getTablePath( ) const
{
    return dreamland->getTableDir( ).getPath( );
}

DLString DLXMLRuntimeLoader::getTablePath( ) const
{
    return dreamland->getDbDir( ).getPath( );
}

DLString DLXMLRuntimeTableLoader::getTablePath( ) const
{
    return dreamland->getDbDir( ).getPath( );
}

