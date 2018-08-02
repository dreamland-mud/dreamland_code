/* $Id: xmlclause.cpp,v 1.1.2.3.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include "xmlclause.h"
#include "boolean.h"

const DLString XMLClause::TYPE = "XMLClause";
const DLString XMLClause::ATTRIBUTE_ALLOW = "allow";

XMLClause::XMLClause( )
             : XMLPredicatesList( true )
{
}

bool XMLClause::match( DLObject * arg ) const
{
    for (const_iterator i = begin( ); i != end( ); i++)
	if (!(*i)->eval( arg )) 
	    return false;
    
    return true;
}

void XMLClause::fromXML( const XMLNode::Pointer& parent ) throw (ExceptionBadType)  
{
    allow = Boolean(parent->getAttribute( ATTRIBUTE_ALLOW ));
    XMLPredicatesList::fromXML( parent );
}

bool XMLClause::toXML( XMLNode::Pointer& parent ) const 
{
    if (XMLPredicatesList::toXML( parent )) {
	parent->insertAttribute( 
		ATTRIBUTE_ALLOW, Boolean( allow ).toString( ) );
	return true;
    }
    else
	return false;
}
