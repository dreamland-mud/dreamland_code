/* $Id: genericskillloader.h,v 1.1.2.5.6.2 2008/02/24 17:23:57 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __GENERICSKILLLOADER_H__
#define __GENERICSKILLLOADER_H__

#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"

class GenericSkillLoader : public DLXMLTableLoader, public XMLTableLoaderPlugin {
public:
    typedef ::Pointer<GenericSkillLoader> Pointer;
    
    virtual void initialization( );
    virtual void destruction( );
    
    virtual DLString getTableName( ) const
    {
        return TABLE_NAME;
    }
    virtual DLString getNodeName( ) const
    {
        return NODE_NAME;
    }
    

protected:
    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;

    void resolveAll( );
    void unresolveAll( );
};

#endif
