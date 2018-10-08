/* $Id: cardskillloader.h,v 1.1.2.5.6.2 2008/02/24 17:22:37 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __CARDSKILLLOADER_H__
#define __CARDSKILLLOADER_H__

#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"

class CardSkillLoader : public DLXMLTableLoader, public XMLTableLoaderPlugin {
public:
    typedef ::Pointer<CardSkillLoader> Pointer;
    
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
};

#endif
