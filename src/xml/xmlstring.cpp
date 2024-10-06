/* $Id: xmlstring.cpp,v 1.8.2.3.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/*
 * NoFate, 2001
 */
// string.cpp: implementation of the String class.
//
//////////////////////////////////////////////////////////////////////
 
#include "exceptionskipvariable.h"
#include "xmlstring.h"

void XMLString::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLNode::Pointer node = parent->getFirstNode( );

    if (!node.isEmpty( )) 
            *this = node->getCData( );
}

bool XMLString::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );
    node->setCData( *this );
    
    parent->appendChild( node );
    return true;
}

bool XMLStringNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (empty( ))
        return false;
    else
        return XMLString::toXML( parent );
}

void XMLStringVariable::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLString::fromXML( parent );
}

bool XMLStringVariable::toXML( XMLNode::Pointer& parent ) const
{
    return XMLString::toXML( parent );
}


XMLStringNode::XMLStringNode( )
{
}


bool XMLStringNode::toXML( XMLNode::Pointer& parent ) const
{
    if (empty( )) {
        return false;
    }

    XMLNode::Pointer grandma = parent->getParent();
    if (grandma) {
        grandma->popChild();
    }

    parent = node;

    if (grandma) {
        grandma->appendChild(parent);
    }

    return true;
}

void XMLStringNode::fromXML( const XMLNode::Pointer& parent ) 
{
    node = parent;
}

