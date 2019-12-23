/* $Id: xmlpredicate.cpp,v 1.1.2.3.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include "xmlpredicate.h"
#include "boolean.h"

const DLString XMLPredicate::ATTRIBUTE_INVERT = "invert";

void XMLPredicate::fromXML( const XMLNode::Pointer& parent )   
{
    invert = Boolean(parent->getAttribute( ATTRIBUTE_INVERT ));
}

bool XMLPredicate::toXML( XMLNode::Pointer& parent ) const 
{
    parent->insertAttribute( ATTRIBUTE_INVERT, Boolean( invert ).toString( ) );
    return true;
}
