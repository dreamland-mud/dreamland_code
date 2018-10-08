/* $Id: xmlbyte.cpp,v 1.9.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// xmlbyte.cpp: implementation of the XMLByte class.
//
//////////////////////////////////////////////////////////////////////


#include "xmlbyte.h"

void XMLByte::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType ) {
        XMLNode::Pointer node = parent->getFirstNode( );

        if (!node.isEmpty( ))
                fromString( node->getCData( ) );
}

bool XMLByte::toXML( XMLNode::Pointer& parent ) const
{
        XMLNode::Pointer node( NEW );

        node->setType( XMLNode::XML_TEXT );
        node->setCData( toString( ) );
        parent->appendChild( node );
        return true;
}

