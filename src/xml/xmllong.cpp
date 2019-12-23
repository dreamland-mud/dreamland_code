/* $Id: xmllong.cpp,v 1.9.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// xmllong.cpp: implementation of the XMLLong class.
//
//////////////////////////////////////////////////////////////////////


#include "xmllong.h"

void XMLLong::fromXML( const XMLNode::Pointer& parent ) 
{
        XMLNode::Pointer node = parent->getFirstNode( );
        
        if (!node.isEmpty( ))
                fromString( node->getCData( ) );
}

bool XMLLong::toXML( XMLNode::Pointer& parent ) const
{
        XMLNode::Pointer node( NEW );

        node->setType( XMLNode::XML_TEXT );
        node->setCData( toString( ) );
        parent->appendChild( node );
        return true;
}

bool XMLLongNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == 0)
        return false;
    else 
        return XMLLong::toXML( parent );
}
