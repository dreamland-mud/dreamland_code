/* $Id: commonattributes.h,v 1.1.2.4.6.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __COMMONATTRIBUTES_H__
#define __COMMONATTRIBUTES_H__

#include "xmllist.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlattribute.h"

class XMLEmptyAttribute: public virtual XMLAttribute {
public:
    typedef ::Pointer<XMLEmptyAttribute> Pointer;

    XMLEmptyAttribute( );
    virtual ~XMLEmptyAttribute( );

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

class XMLStringAttribute: public virtual XMLAttribute, public XMLStringVariable {
public:
    typedef ::Pointer<XMLStringAttribute> Pointer;

    XMLStringAttribute( );
    virtual ~XMLStringAttribute( );

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

class XMLIntegerAttribute: public virtual XMLAttribute, public XMLIntegerVariable {
public:
    typedef ::Pointer<XMLIntegerAttribute> Pointer;

    XMLIntegerAttribute( );
    virtual ~XMLIntegerAttribute( );

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

class XMLStringListAttribute: public virtual XMLAttribute, 
                              public XMLListBase<XMLString> 
{
public:
    typedef ::Pointer<XMLStringListAttribute> Pointer;

    XMLStringListAttribute( );
    virtual ~XMLStringListAttribute( );

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

#endif
