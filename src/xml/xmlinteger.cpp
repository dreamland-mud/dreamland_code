/* $Id: xmlinteger.cpp,v 1.10.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// xmlinteger.cpp: implementation of the XMLInteger class.
//
//////////////////////////////////////////////////////////////////////

#include "exceptionskipvariable.h"
#include "xmlinteger.h"

void XMLInteger::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::Pointer node = parent->getFirstNode( );
    
    if (!node.isEmpty( ))
            fromString( node->getCData( ) );
}

bool XMLInteger::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );

    node->setType( XMLNode::XML_TEXT );
    node->setCData( toString( ) );
    parent->appendChild( node );
    return true;
}

bool XMLIntegerNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (getValue( ) == 0)
        return false;
    else 
        return XMLInteger::toXML( parent );
}

void XMLIntegerVariable::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLInteger::fromXML( parent );
}

bool XMLIntegerVariable::toXML( XMLNode::Pointer& parent ) const
{
    return XMLInteger::toXML( parent );
}

