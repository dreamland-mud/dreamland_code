/* $Id: xmlglobalbitvector.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "xmlglobalbitvector.h"
#include "xmlstring.h"

XMLGlobalBitvector::XMLGlobalBitvector( )
{
}

XMLGlobalBitvector::XMLGlobalBitvector( GlobalRegistryBase *reg )
                      : GlobalBitvector( reg )
{
}

void XMLGlobalBitvector::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLString str;

    str.fromXML( parent );
    fromString( str );
}

bool XMLGlobalBitvector::toXML( XMLNode::Pointer& parent ) const
{
    DLString str = toString( );

    if (str.empty( ))
        return false;
    else
        return XMLString( str ).toXML( parent );
}
