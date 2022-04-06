/* $Id: xmlenumeration.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "xmlenumeration.h"
#include "xmlvector.h"
#include "xmlinteger.h"
#include "exceptionskipvariable.h"

/*
 * XMLEnumeration
 */
XMLEnumeration::XMLEnumeration( int v, const FlagTable *t ) 
        : Enumeration( v, t ) 
{
}

void XMLEnumeration::fromXML( const XMLNode::Pointer& parent ) 
{
    int value;
    XMLNode::Pointer node = parent->getFirstNode( );

    if (node) 
        if (( value = table->value( node->getCData( ) ) ) != NO_FLAG)
            setValue( value );
}

bool XMLEnumeration::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    if (getTable( ) == NULL)
        return false;

    node->setType( XMLNode::XML_TEXT );
    node->setCData( name( ) );
    parent->appendChild( node );
    return true;
}

/*
 * XMLEnumerationArray
 */
XMLEnumerationArray::XMLEnumerationArray( const FlagTable *t, int def ) 
                       : EnumerationArray( t ), defaultValue( def )
{
    clear( );
}

void XMLEnumerationArray::clear( )
{
    vector<int>::clear( );
    resize( table->max + 1, defaultValue );
}

bool XMLEnumerationArray::toXML( XMLNode::Pointer& parent ) const
{
    unsigned int n;
    int i;
    
    for (n = 0; n < size( ); n++) {
        if (at( n ) != defaultValue) {
            XMLNode::Pointer node( NEW );
            XMLInteger value( at( n ) );
            
            if (( i = table->reverse[n] ) != NO_FLAG) {
                node->setName( table->fields[i].name );
                value.toXML( node );
                parent->appendChild( node );
            }
        }
    }

    if (parent->getNodeList( ).empty( ))
        return false;
    else 
        return true;
}

void XMLEnumerationArray::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLNode::NodeList::const_iterator n;
    
    clear( );

    for (n = parent->getNodeList( ).begin( ); n != parent->getNodeList( ).end( ); n++) {
        int i;
        XMLInteger value;
        
        i = table->index( (*n)->getName( ) );
        
        if (i == NO_FLAG)
            throw ExceptionBadType( parent->getName( ), (*n)->getName( ) );
        
        value.fromXML( *n );
        at( table->fields[i].value ) = value;
    }
}

/*
 * XMLEnumerationNoEmpty
 */
XMLEnumerationNoEmpty::XMLEnumerationNoEmpty( int v, const FlagTable *t ) 
        : XMLEnumeration( v, t ) 
{
}

bool XMLEnumerationNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (name( ).empty( ))
        return false;
    else 
        return XMLEnumeration::toXML( parent );
}

