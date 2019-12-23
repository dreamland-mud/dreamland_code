/* $Id: xmlchar.cpp,v 1.9.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// string.cpp: implementation of the String class.
//
//////////////////////////////////////////////////////////////////////

#include "xmlchar.h"

void XMLChar::fromXML( const XMLNode::Pointer& parent ) 
{
        XMLNode::Pointer node = parent->getFirstNode( );

        if (!node.isEmpty( ))
                fromString( node->getCData( ) );
}

bool XMLChar::toXML( XMLNode::Pointer& parent ) const
{
        XMLNode::Pointer node( NEW );

        node->setType( XMLNode::XML_TEXT );
        node->setCData( toString( ) );
        parent->appendChild( node );
        return true;
}

