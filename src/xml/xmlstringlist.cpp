/* $Id: xmlstringlist.cpp,v 1.1.2.5 2011-04-17 19:37:54 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include "xmlstringlist.h"

XMLStringList::XMLStringList()
{
}

void XMLStringList::toSet( StringSet &aset ) const
{
    const_iterator i;

    for (i = begin( ); i != end( ); i++)
        if (*i != "\'" && *i != "\"")
            aset.insert( *i );
}


bool XMLStringSet::toXML( XMLNode::Pointer& node ) const
{
    return XMLStringNoEmpty( toString( ) ).toXML( node );
}

void XMLStringSet::fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
{
    XMLString str;

    str.fromXML( node );
    fromString( str );
}


bool XMLNumberSet::toXML( XMLNode::Pointer& node ) const
{
    return XMLStringNoEmpty( 
                toStringSet( ).toString( ) )
                    .toXML( node );
}

void XMLNumberSet::fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
{
    XMLStringSet str;

    str.fromXML( node );
    fromStringSet( str );
}


