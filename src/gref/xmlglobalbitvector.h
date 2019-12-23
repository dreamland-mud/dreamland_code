/* $Id: xmlglobalbitvector.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLGLOBALBITVECTOR_H__
#define __XMLGLOBALBITVECTOR_H__

#include "globalbitvector.h"
#include "xmlnode.h"

class XMLGlobalBitvector : public GlobalBitvector {
public:
    XMLGlobalBitvector( );
    XMLGlobalBitvector( GlobalRegistryBase * );
    
    void fromXML( const XMLNode::Pointer& ) ;
    bool toXML( XMLNode::Pointer& ) const;

protected:
    bool registryFromAttribute;
};


#endif
