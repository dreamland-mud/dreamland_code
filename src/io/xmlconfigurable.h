/* $Id: xmlconfigurable.h,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2009
 */
#ifndef XMLCONFIGURABLE_H
#define XMLCONFIGURABLE_H

#include "xmlfile.h"
#include "xmlpolymorphvariable.h"

class XMLConfigurable : public virtual XMLPolymorphVariable {
public:
    virtual ~XMLConfigurable( );
    
    virtual DLFile getConfigFile( ) const;
protected:
    virtual DLString getConfigDirPath( ) const;

    bool loadConfig( );
    bool saveConfig( );
    XMLFile getConfigXMLFile( );
};


class XMLConfigurableWithPath : public XMLConfigurable {
public:

    inline void setConfigFilePath( const DLString & );

    virtual DLFile getConfigFile( ) const;
protected:

    DLString configFilePath;
};

void XMLConfigurableWithPath::setConfigFilePath( const DLString &path )
{
    configFilePath = path;
}

#endif
