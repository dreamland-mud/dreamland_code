/* $Id: xmlglobalbitvector.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "logstream.h"
#include "xmlglobalbitvector.h"
#include "xmlstring.h"

XMLGlobalBitvector::XMLGlobalBitvector( )
                        : registryFromAttribute(false)
{
}

XMLGlobalBitvector::XMLGlobalBitvector( GlobalRegistryBase *reg )                    
                      :  GlobalBitvector( reg ), registryFromAttribute(false)
{
}

void XMLGlobalBitvector::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLString str;
    str.fromXML( parent );

    // Try to read registry from an attribute and remember we've done so.
    DLString registryName = parent->getAttribute("registry");
    if (!registryName.empty()) {
        registry = const_cast<GlobalRegistryBase *>(registryMap[registryName]);
        registryFromAttribute = (registry != 0);
    }

    if (!registry) {
        LogStream::sendError() << "Registry not set or incorrectly specified for " << str << endl;
        return;
    }

    fromString( str );
}

bool XMLGlobalBitvector::toXML( XMLNode::Pointer& parent ) const
{
    DLString str = toString( );

    // If registry was not set in the constructor but instead assigned in fromXML(),
    // serialize it again.
    if (registry)
        parent->insertAttribute("registry", registry->getRegistryName());

    if (str.empty( ))
        return false;
    else
        return XMLString( str ).toXML( parent );
}
