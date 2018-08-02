/* $Id$
 *
 * ruffina, 2004
 */
#include "xmltableloaderplugin.h"

/*-------------------------------------------------------------------------
 * XMLTableLoaderPlugin
 *------------------------------------------------------------------------*/
void XMLTableLoaderPlugin::initialization( )
{
    readAll( );
    loadAll( );
}

void XMLTableLoaderPlugin::destruction( )
{
//    saveAll( );
    unloadAll( );
}


