/* $Id: xmllonglong.cpp,v 1.9.2.2.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// xmllonglong.cpp: implementation of the XMLLongLong class.
//
//////////////////////////////////////////////////////////////////////


#include "xmllonglong.h"

void XMLLongLong::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
        XMLNode::Pointer node = parent->getFirstNode( );
	
        if (!node.isEmpty( ))
                fromString( node->getCData( ) );
}

bool XMLLongLong::toXML( XMLNode::Pointer& parent ) const
{
        XMLNode::Pointer node( NEW );

        node->setType( XMLNode::XML_TEXT );
        node->setCData( toString( ) );
        parent->appendChild( node );
	return true;
}


bool XMLLongLongNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == 0)
	return false;
    else 
	return XMLLongLong::toXML( parent );
}
