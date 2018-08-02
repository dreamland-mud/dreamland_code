/* $Id: xmlboolean.cpp,v 1.9.2.2.28.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// xmlboolean.cpp: implementation of the XMLBoolean class.
//
//////////////////////////////////////////////////////////////////////
#include "xmlboolean.h"

void XMLBoolean::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::Pointer node = parent->getFirstNode( );
    
    if (!node.isEmpty( )) 
	fromString( node->getCData( ) );
}

bool XMLBoolean::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );
    node->setCData( toString( ) );
    parent->appendChild( node );
    return true;
}

bool XMLBooleanNoFalse::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == false)
	return false;
    else
	return XMLBoolean::toXML( parent );
}

bool XMLBooleanNoTrue::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == true)
	return false;
    else
	return XMLBoolean::toXML( parent );
}

