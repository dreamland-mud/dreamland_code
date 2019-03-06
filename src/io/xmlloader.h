/* $Id: xmlloader.h,v 1.1.2.6.6.5 2010-09-01 08:21:11 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLLOADER_H__
#define __XMLLOADER_H__

#include "dlstring.h"

class XMLVariable;

class XMLLoader {
public:
    virtual ~XMLLoader( );    
    virtual DLString getTablePath( ) const = 0;
    virtual DLString getTableName( ) const = 0;
    virtual DLString getNodeName( ) const = 0;

    bool loadXML( XMLVariable *, const DLString &, bool fIgnoreNotFound = false ) const;
    bool saveXML( const XMLVariable *, const DLString &, bool fSafe = false ) const;
};

#endif
