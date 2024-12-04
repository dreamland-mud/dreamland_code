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
#include "xmlmap.h"
#include "playerattributes.h"

class XMLEmptyAttribute: public virtual XMLAttribute {
public:
    typedef ::Pointer<XMLEmptyAttribute> Pointer;

    XMLEmptyAttribute( );
    virtual ~XMLEmptyAttribute( );

    virtual void fromXML( const XMLNode::Pointer& node ) ;
    virtual bool toXML( XMLNode::Pointer& node ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

class XMLStringAttribute: public virtual XMLAttribute, public XMLStringVariable, public RemortAttribute {
public:
    typedef ::Pointer<XMLStringAttribute> Pointer;

    XMLStringAttribute( );
    virtual ~XMLStringAttribute( );
    virtual Scripting::Register toRegister() const;

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
    virtual Scripting::Register toRegister() const;

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
    virtual Scripting::Register toRegister() const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

class XMLStringMapAttribute : public virtual XMLAttribute,
                              public XMLMapBase<XMLString> 
{
public:        
        typedef ::Pointer<XMLStringMapAttribute> Pointer;

        XMLStringMapAttribute( );
        virtual ~XMLStringMapAttribute( );
        virtual Scripting::Register toRegister() const;

        static const DLString TYPE;

        virtual const DLString & getType( ) const
        {
            return TYPE;
        }
};

class PCMemoryInterface;
namespace Json {
    class Value;
}

const DLString & get_string_attribute(PCMemoryInterface *player, const DLString &attrName);
XMLStringMapAttribute & get_map_attribute(PCMemoryInterface *player, const DLString &attrName);
const DLString & get_map_attribute_value(PCMemoryInterface *player, const DLString &attrName, const DLString &key);
void set_map_attribute_value(PCMemoryInterface *player, const DLString &attrName, const DLString &key, const DLString &value);

bool get_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue);
void set_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue);
PCMemoryInterface * find_player_by_attribute(const DLString &attrName, const DLString &attrValue);
list<PCMemoryInterface *> find_players_by_json_attribute(const DLString &attrName, const DLString &name, const DLString &value);
#endif
