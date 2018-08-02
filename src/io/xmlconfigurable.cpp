/* $Id: xmlconfigurable.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2009
 */
#include "xmlconfigurable.h"

XMLConfigurable::~XMLConfigurable( )
{
}

bool XMLConfigurable::loadConfig( )
{
    return getConfigXMLFile( ).load( );
}

bool XMLConfigurable::saveConfig( )
{
    return getConfigXMLFile( ).save( );
}

XMLFile XMLConfigurable::getConfigXMLFile( )
{
    XMLFile configFile;
    
    configFile.setFile( getConfigFile( ) );
    configFile.setNodeName( getType( ) );
    configFile.setVariable( this );
    
    return configFile;
}

DLFile XMLConfigurable::getConfigFile( ) const
{
    return DLFile( getConfigDirPath( ),
	           getType( ).toLower( ),
		   ".xml" );
}

DLString XMLConfigurable::getConfigDirPath( ) const
{
    return DLString::emptyString;
}

DLFile XMLConfigurableWithPath::getConfigFile( ) const
{
    if (!configFilePath.empty( ))
	return configFilePath;

    return XMLConfigurable::getConfigFile( );
}

