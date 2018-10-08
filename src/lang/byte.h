/* $Id: byte.h,v 1.2.34.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          byte.h  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/


#ifndef BYTE_H
#define BYTE_H

#include "dlstring.h"

/**
 * @author Igor S. Petrenko
 */
class Byte
{
public:
	static const DLString TYPE;

public:	
	/** По умолчанию значение 0 */
	inline Byte( ) : value( 0 )
	{
	}

	inline Byte( unsigned char value ) : value( value )
	{
	}

	inline Byte( const DLString& value ) throw( ExceptionBadType )
	{
		fromString( value );
	}
	
	DLString toString( ) const throw( );
	void fromString( const DLString& value ) throw( ExceptionBadType );
	
	inline unsigned char getValue( ) const
	{
		return value;
	}

	inline void setValue( unsigned char value )
	{
		this->value = value;
	}

private:
	unsigned char value;
};




/** Вывод в ostream типа Byte */
inline std::ostream& operator << ( std::ostream& ostr, const Byte& byte )
{
	ostr << byte.getValue( );
	return ostr;
}

#endif
