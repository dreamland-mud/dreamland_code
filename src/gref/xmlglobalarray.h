/* $Id: xmlglobalarray.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef XMLGLOBALARRAY_H
#define XMLGLOBALARRAY_H

#include "globalarray.h"
#include "xmlnode.h"

class XMLGlobalArray : public GlobalArray {
public:
    XMLGlobalArray( GlobalRegistryBase * );
    
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;
};

#endif
