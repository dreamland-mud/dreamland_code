/* $Id: xmltimestamp.cpp,v 1.1.2.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include "xmltimestamp.h"
#include "date.h"

XMLTimeStamp::XMLTimeStamp( )
       : XMLLong( Date::getCurrentTime( ) )
{
}

bool XMLTimeStamp::toXML( XMLNode::Pointer& parent ) const
{
    return XMLLong( Date::getCurrentTime( ) ).toXML( parent );
}

void XMLTimeStamp::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLLong::fromXML( parent );
}

