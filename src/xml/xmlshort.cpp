/* $Id: xmlshort.cpp,v 1.9.2.2.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmlshort.cpp  -  description
                             -------------------
    begin                : Wed Apr 25 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "xmlshort.h"

void XMLShort::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType ) {
        XMLNode::Pointer node = parent->getFirstNode( );

        if (!node.isEmpty( ))
                fromString( node->getCData( ) );
}

bool XMLShort::toXML( XMLNode::Pointer& parent ) const
{
        XMLNode::Pointer node( NEW );

        node->setType( XMLNode::XML_TEXT );
        node->setCData( toString( ) );
        parent->appendChild( node );
	return true;
}

bool XMLShortNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == 0)
	return false;
    else 
	return XMLShort::toXML( parent );
}

