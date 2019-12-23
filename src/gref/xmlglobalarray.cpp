/* $Id: xmlglobalarray.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "xmlglobalarray.h"
#include "xmlinteger.h"

XMLGlobalArray::XMLGlobalArray( GlobalRegistryBase *reg )
                      : GlobalArray( reg )
{
}

void XMLGlobalArray::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLNode::NodeList::const_iterator n;
    
    clear( );

    if (!registry)
        return;

    for (n = parent->getNodeList( ).begin( ); n != parent->getNodeList( ).end( ); n++) {        
        DLString name = (*n)->getName();
        name = name.replaces("_", " ");

        size_type ndx;
        ndx = registry->lookup(name);

        XMLInteger value;
        value.fromXML( *n );
        
        (*this)[ndx] = value;
    }
}

bool XMLGlobalArray::toXML( XMLNode::Pointer& parent ) const
{
    size_type n;
    
    if (!registry)
        return false;

    for (n = 0; n < size( ); n++) 
        if (at( n ) != 0) {
            XMLNode::Pointer node( NEW );

            XMLInteger value( at( n ) );            
            value.toXML( node );

            DLString name = registry->getName(n);
            name = name.replaces(" ", "_");
            node->setName(name);

            parent->appendChild( node );
        }

    if (parent->getNodeList( ).empty( ))
        return false;
    else 
        return true;
}
