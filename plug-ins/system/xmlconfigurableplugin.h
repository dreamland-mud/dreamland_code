/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLCONFIGURABLEPLUGIN_H
#define XMLCONFIGURABLEPLUGIN_H

#include "plugin.h"
#include "xmlpolymorphvariable.h"
#include "xmlfile.h"

class XMLConfigurablePlugin : public virtual Plugin, public virtual XMLPolymorphVariable {
public:

    virtual void initialization( );
    virtual void destruction( );
    
protected:
    bool loadConfig( );
    bool saveConfig( );
    XMLFile getConfigFile( );
};

#endif
