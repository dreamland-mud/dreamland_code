/* $Id: xmlglobalreference.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "xmlglobalreference.h"
#include "xmlstring.h"

void XMLGlobalReference::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLString str;

    str.fromXML( parent );
    setName( str.getValue( ) );
}

bool XMLGlobalReference::toXML( XMLNode::Pointer& parent ) const
{
    if (name == "none")
        return false;
    else
        return XMLString( name ).toXML( parent );
}
