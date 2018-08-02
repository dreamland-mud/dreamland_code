/* $Id: xmlflags.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "xmlflags.h"
#include "xmlvector.h"
#include "xmlinteger.h"
#include "exceptionskipvariable.h"



/*
 * XMLFlags
 */

XMLFlags::XMLFlags( bitstring_t v, const FlagTable *t ) 
	: Flags( v, t ) 
{
}

void XMLFlags::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::Pointer node = parent->getFirstNode( );

    value = 0;

    if (node) 
	if (( value = table->bitstring( node->getCData( ) ) ) == NO_FLAG)
	    value = 0;
}

bool XMLFlags::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    if (table == 0)
	return false;

    node->setType( XMLNode::XML_TEXT );
    node->setCData( names( ) );
    parent->appendChild( node );
    return true;
}

/*
 * XMLFlagsWithTable
 */
const DLString XMLFlagsWithTable::ATTRIBUTE_TABLE = "table";

XMLFlagsWithTable::XMLFlagsWithTable( ) : XMLFlags( 0, NULL )
{
}

void XMLFlagsWithTable::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    setTable( parent->getAttribute( ATTRIBUTE_TABLE ) );

    if (!table)
	throw ExceptionBadType( parent->getName( ), ATTRIBUTE_TABLE );
    
    XMLFlags::fromXML( parent );
}

bool XMLFlagsWithTable::toXML( XMLNode::Pointer& parent ) const
{
    if (XMLFlags::toXML( parent )) {
	parent->insertAttribute( ATTRIBUTE_TABLE, getTableName( ) );
	return true;
    }
    else
	return false;
}

/*
 * XMLFlagsNoEmpty
 */
XMLFlagsNoEmpty::XMLFlagsNoEmpty( bitstring_t v, const FlagTable *t ) 
	: XMLFlags( v, t ) 
{
}

bool XMLFlagsNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (names( ).empty( ))
	return false;
    else 
	return XMLFlags::toXML( parent );
}

