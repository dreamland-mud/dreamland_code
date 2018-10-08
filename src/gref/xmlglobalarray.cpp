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

void XMLGlobalArray::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::NodeList::const_iterator n;
    
    clear( );

    if (!registry)
        return;

    for (n = parent->getNodeList( ).begin( ); n != parent->getNodeList( ).end( ); n++) {
        XMLInteger value;
        size_type ndx;
        
        value.fromXML( *n );
        ndx = registry->lookup( (*n)->getName( ) );
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
            node->setName( registry->getName( n ) );
            parent->appendChild( node );
        }

    if (parent->getNodeList( ).empty( ))
        return false;
    else 
        return true;
}
