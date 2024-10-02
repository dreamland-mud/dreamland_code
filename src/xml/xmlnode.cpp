/* $Id: xmlnode.cpp,v 1.8.34.4 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmlnode.cpp  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#undef yyFlexLexer
#define yyFlexLexer xmlmatchpattern
#include <FlexLexer.h>

#include <algorithm>
#include "logstream.h"
#include "xmlnode.h"
#include "xmlmatchpattern.h"
#include "integer.h"

const DLString XMLNode::ATTRIBUTE_TYPE = "type";
const DLString XMLNode::ATTRIBUTE_NODE = "node";
const DLString XMLNode::ATTRIBUTE_NAME = "name";


XMLNode::XMLNode( )
        : type( XML_NODE ), parent( 0 )
{
}

XMLNode::XMLNode( const DLString& name )
        : name( name ), type( XML_NODE ), parent( 0 )
{
}

XMLNode::~XMLNode( )
{
        for( NodeList::iterator pos = nodes.begin( );
                pos != nodes.end( );
                pos++ )
        {
                ( *pos )->parent = 0;
        }
}

void XMLNode::appendChild( XMLNode::Pointer& node )
{
        nodes.push_back( node );
        node->parent = this;
}

void XMLNode::popChild()
{
        if (!nodes.empty()) {
                nodes.back()->parent = 0;
                nodes.pop_back();
        }
}

XMLNode::Pointer XMLNode::clone( bool recursive ) const
{
        XMLNode::Pointer node( NEW, getName( ) );
        node->setType( getType( ) );
        node->attributes = attributes;
        node->parent = parent;
        if( recursive )
        {
                for( NodeList::const_iterator pos = nodes.begin( );
                        pos != nodes.end( );
                        pos++ )
                {
                        XMLNode::Pointer cloneNode = ( *pos )->clone( true );
                        node->appendChild( cloneNode );
                }
        }
        return node;
}

void XMLNode::erase( const XMLNode::Pointer& node )
{
        NodeList::iterator pos = find( nodes.begin( ), nodes.end( ), node );
        if( pos != nodes.end( ) )
        {
                ( *pos )->parent = 0;
                nodes.erase( pos );
        }
}

XMLNode::NodeList::Pointer XMLNode::getElementsByTagName( const DLString& name ) const
{
        NodeList::Pointer list( NEW );
        for( NodeList::const_iterator ipos = nodes.begin( );
                ipos != nodes.end( );
                ipos++ )
        {
                if( ( *ipos )->getName( ) == name )
                {
                        list->push_back( ( *ipos )->clone( true ) );
                }
        }
        return list;
}

XMLNode::NodeList::Pointer XMLNode::selectNodes( const DLString& pattern ) const 
{
        XMLMatchPattern match( this, pattern );
        match.yylex( );
        return match.getList( );
}

XMLNode::Pointer XMLNode::selectSingleNode( const DLString& pattern ) const 
{
        XMLMatchPattern match( this, pattern );
        match.yylex( );
        NodeList::Pointer list = match.getList( );
        if( list->empty( ) )
        {
                return XMLNode::Pointer( );
        }
        else
        {
                return *list->begin( );
        }
}

void XMLNode::getAttribute(const DLString &name, int &value) const
{
    if (hasAttribute(name)) {
        try {
            value = getAttribute(name).toInt();
        } catch (const ExceptionBadType &e) {
            LogStream::sendError() 
                << "XML attribute: expected int for " 
                << name << ", got " << getAttribute(name) << endl;
        }
    }
}

bool XMLNode::equal( const XMLNode &other ) const
{
    NodeList::const_iterator n_my, n_other;
    AttributeListType::const_iterator a_my, a_other;
    
    if (getType( ) != other.getType( )) 
        return false;

    if (getName( ) != other.getName( )) 
        return false;
    
    if (attributes.size( ) != other.attributes.size( )) 
        return false;
    
    for (a_my = attributes.begin( ), a_other = other.attributes.begin( );
         a_my != attributes.end( ) && a_other != other.attributes.end( );
         a_my++, a_other++)
    {
        if (a_my->first != a_other->first) 
            return false;
        
        if (a_my->second != a_other->second) 
            return false;
    }
    
    if (nodes.size( ) != other.nodes.size( )) 
        return false;

    for (n_my = nodes.begin( ), n_other = other.nodes.begin( );
         n_my != nodes.end( ) && n_other != other.nodes.end( );
         n_my++, n_other++)
    {
        if (!(*n_my)->equal( ***n_other )) 
            return false;
    }
    
    return true;
}

XMLNode::Pointer XMLNode::getTopNode( ) const
{
    if (getParent( ))
        return getParent( )->getTopNode( );
    else
        return this;
}

bool XMLNode::hasNoData( ) const
{
    if (attributes.size( ) != 0)
        return false;

    if (getType( ) == XML_LEAF)
        return true;

    if (nodes.empty( ))
        return true;

    if (nodes.size( ) == 1 
        && getFirstNode( )->getType( ) == XML_TEXT
        && getFirstNode( )->getCData( ).empty( ))
        return true;

    return false;
}

