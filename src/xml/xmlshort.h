/* $Id: xmlshort.h,v 1.7.2.2.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmlshort.h  -  description
                             -------------------
    begin                : Wed Apr 25 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef XMLSHORT_H
#define XMLSHORT_H

#include "short.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная short
 */
class XMLShort : public Short
{
public:
	/** По умолчанию значение \0 */
	inline XMLShort( ) : Short( )
	{
	}
	
	inline XMLShort( short value ) : Short( value )
	{
	}
	
	inline XMLShort( const DLString& value ) throw( ExceptionBadType )
		: Short( value )
	{
	}
	
	/** Возвращает xml представление переменной */
	bool toXML( XMLNode::Pointer& node ) const;
	/** Инициализация класса из xml данных */
	void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
};



/** Вывод в ostream типа XMLShort */
inline std::ostream& operator << ( std::ostream& ostr, const XMLShort& xmlShort )
{
	ostr << xmlShort.getValue( );
	return ostr;
}

class XMLShortNoEmpty : public XMLShort {
public:
	inline XMLShortNoEmpty( )
	{
	}
	
	inline XMLShortNoEmpty( int value ) : XMLShort( value )
	{
	}

	bool toXML( XMLNode::Pointer& node ) const;
};

#endif
