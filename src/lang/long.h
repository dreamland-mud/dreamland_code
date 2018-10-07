/* $Id: long.h,v 1.1.2.1.30.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          long.h  -  description
                             -------------------
    begin                : Thu Nov 1 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef LONG_H
#define LONG_H

#include "dlstring.h"
/**
 * @author Igor S. Petrenko
 */
class Long 
{
public:
	static const DLString TYPE;

public:	
	/** По умолчанию значение 0 */
	inline Long( ) : value( 0 )
	{
	}

	inline Long( long value ) : value( value )
	{
	}

	inline Long( const DLString& value ) throw( ExceptionBadType )
	{
		fromString( value );
	}

	DLString toString( ) const throw( );
	void fromString( const DLString& value ) throw( ExceptionBadType );
	
	inline long getValue( ) const
	{
		return value;
	}

	inline void setValue( long value )
	{
		this->value = value;
	}

	inline operator long & ( ) {
	    return value;
	}
private:
	long value;
};





/** Вывод в ostream типа Long */
inline std::ostream& operator << ( std::ostream& ostr, const Long& longValue )
{
	ostr << longValue.getValue( );
	return ostr;
}

#endif
