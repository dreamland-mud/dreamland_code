/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlconfigurableplugin.h"
#include "dreamland.h"

void XMLConfigurablePlugin::initialization( )
{
    loadConfig( );
}

void XMLConfigurablePlugin::destruction( )
{
// saveConfig( );
}

bool XMLConfigurablePlugin::loadConfig( )
{
    return getConfigFile( ).load( );
}

bool XMLConfigurablePlugin::saveConfig( )
{
    return getConfigFile( ).save( );
}

XMLFile XMLConfigurablePlugin::getConfigFile( )
{
    XMLFile configFile;
    
    configFile.setFile( DLFile( dreamland->getTableDir( ),
                                getType( ).toLower( ),
                                ".xml" ) );
    configFile.setNodeName( getType( ) );
    configFile.setVariable( this );
    
    return configFile;
}

