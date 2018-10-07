/* $Id: xmlboolean.h,v 1.7.2.2.28.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// xmlboolean.h: interface for the XMLBoolean class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLBOOLEAN_H
#define XMLBOOLEAN_H

#include "boolean.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная bool
 */
class XMLBoolean : public Boolean
{
public:
	/** По умолчанию значение true */
	inline XMLBoolean( ) : Boolean( )
	{
	}
	
	inline XMLBoolean( bool value ) : Boolean( value )
	{
	}
	
	inline XMLBoolean( const DLString& value ) throw( ExceptionBadType )
		: Boolean( value )
	{
	}

	/** Возвращает xml представление переменной */
	bool toXML( XMLNode::Pointer& node ) const;
	/** Инициализация класса из xml данных */
	void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
};

class XMLBooleanNoFalse : public XMLBoolean {
public:
	inline XMLBooleanNoFalse( ) : XMLBoolean( )
	{
	}
	
	inline XMLBooleanNoFalse( bool value ) : XMLBoolean( value )
	{
	}
	
	inline XMLBooleanNoFalse( const DLString& value ) throw( ExceptionBadType )
		: XMLBoolean( value )
	{
	}

	bool toXML( XMLNode::Pointer& node ) const;
};

class XMLBooleanNoTrue : public XMLBoolean {
public:
	inline XMLBooleanNoTrue( ) : XMLBoolean( )
	{
	}
	
	inline XMLBooleanNoTrue( bool value ) : XMLBoolean( value )
	{
	}
	
	inline XMLBooleanNoTrue( const DLString& value ) throw( ExceptionBadType )
		: XMLBoolean( value )
	{
	}

	bool toXML( XMLNode::Pointer& node ) const;
};

template <bool def>
class XMLBooleanNoDef : public XMLBoolean {
public:
    XMLBooleanNoDef( ) : XMLBoolean(def) { }

    inline bool toXML(XMLNode::Pointer &parent) const {
        if (getValue( ) == def)
            return false;
        else 
            return XMLBoolean::toXML(parent);
    }
};


/** Вывод в ostream типа XMLBoolean */
inline std::ostream& operator << ( std::ostream& ostr, const XMLBoolean& xmlBoolean )
{
	ostr << xmlBoolean.getValue( );
	return ostr;
}

#endif
