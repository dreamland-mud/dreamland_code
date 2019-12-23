/* $Id: xmlstringlist.cpp,v 1.1.2.5 2011-04-17 19:37:54 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include "xmlstringlist.h"

XMLStringList::XMLStringList()
{
}

StringSet XMLStringList::toSet() const
{
    StringSet aset;
    const_iterator i;

    for (i = begin( ); i != end( ); i++)
        if (*i != "\'" && *i != "\"")
            aset.insert( *i );

    return aset;
}


bool XMLStringSet::toXML( XMLNode::Pointer& node ) const
{
    return XMLStringNoEmpty( toString( ) ).toXML( node );
}

void XMLStringSet::fromXML( const XMLNode::Pointer& node ) 
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

void XMLNumberSet::fromXML( const XMLNode::Pointer& node ) 
{
    XMLStringSet str;

    str.fromXML( node );
    fromStringSet( str );
}


