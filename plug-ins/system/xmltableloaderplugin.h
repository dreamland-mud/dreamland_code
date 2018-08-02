/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLTABLELOADERPLUGIN_H
#define XMLTABLELOADERPLUGIN_H

#include "plugin.h"
#include "xmltableloader.h"

/*
 * XMLTableLoaderPlugin
 */
class XMLTableLoaderPlugin : public virtual XMLTableLoader, public virtual Plugin {
public:

    virtual void initialization( );
    virtual void destruction( );
};

#define TABLE_LOADER(className, tableName, nodeName)                          \
    struct className : public XMLTableLoaderPlugin, public DLXMLTableLoader { \
	virtual DLString getTableName( ) const {                              \
	    return tableName;                                                 \
	}                                                                     \
	virtual DLString getNodeName( ) const {                               \
	    return nodeName;                                                  \
	}                                                                     \
    }

#endif
