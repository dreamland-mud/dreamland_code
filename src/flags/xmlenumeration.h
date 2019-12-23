/* $Id: xmlenumeration.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __XMLENUMERATIONS_H__
#define __XMLENUMERATIONS_H__

#include "enumeration.h"
#include "xmlnode.h"

/*
 * XMLEnumeration
 */
class XMLEnumeration : public Enumeration {
public:
    XMLEnumeration( int, const FlagTable * );

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    inline XMLEnumeration & operator = ( int value ) { 
        setValue( value );
        return *this;
    }
};

/*
 * XMLEnumerationArray
 */
class XMLEnumerationArray : public EnumerationArray {
public:
    XMLEnumerationArray( const FlagTable *, int = 0 );
    
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    void clear( );
protected:
    int defaultValue;
};

/*
 * XMLEnumerationNoEmpty
 */
class XMLEnumerationNoEmpty : public XMLEnumeration {
public:
    XMLEnumerationNoEmpty( int, const FlagTable * );
    
    bool toXML( XMLNode::Pointer& ) const;
};


#endif
