/* $Id$
 *
 * ruffina, 2004
 */
#include "xmltableloaderplugin.h"
#include "logstream.h"
#include "exceptionskipvariable.h"
#include "xmlvariable.h"
#include "xmlstreamable.h"
#include "xmldocument.h"
#include "dbio.h"


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

