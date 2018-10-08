/* $Id: xmlstring.h,v 1.9.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// string.h: interface for the String class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLSTRING_H
#define XMLSTRING_H

#include "dlstring.h"
#include "xmlnode.h"
#include "xmlvariable.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная DLString
 */
class XMLString : public DLString 
{
public:
    inline XMLString( ) 
    {
    }
    
    inline XMLString( const DLString& value ) : DLString( value ) 
    {
    }
    
    inline XMLString( const char* value ) : DLString( value )
    {
    }
    
    /** Возвращает xml представление переменной */
    bool toXML( XMLNode::Pointer& node ) const;
    void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
    
    // compat
    inline const DLString& getValue( ) const
    {
	    return *this;
    }
    inline void setValue( const DLString& value )
    {
	    assign( value );
    }
};

class XMLStringNoEmpty : public XMLString {
public:
    inline XMLStringNoEmpty( ) 
    {
    }
    
    inline XMLStringNoEmpty( const DLString& value ) : XMLString( value ) 
    {
    }

    inline XMLStringNoEmpty( const char * value ) : XMLString( value ) 
    {
    }

    bool toXML( XMLNode::Pointer& node ) const;
};

class XMLStringVariable : public virtual XMLVariable, public XMLString {
public:
    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
};

class XMLStringNode {
public:
    XMLStringNode( );

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
   
    inline bool empty( ) const
    {
	return node.isEmpty( );
    }
    
    inline const DLString & getString( ) const
    {
	return node->getCData( );
    }

    inline void setNode( const XMLNode::Pointer node ) 
    {
	this->node = node;
    }

    inline XMLNode::Pointer getNode( ) const
    {
	return node;
    }

private:	
    XMLNode::Pointer node;
};

#endif 
