/* $Id: short.h,v 1.2.2.1.30.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          short.h  -  description
                             -------------------
    begin                : Mon Oct 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef SHORT_H
#define SHORT_H

#include "dlstring.h"
/**
 * @author Igor S. Petrenko
 */
class Short
{
public:
	static const DLString TYPE;

public:	
	/** По умолчанию значение 0 */
	inline Short( ) : value( 0 )
	{
	}

	inline Short( short value ) : value( value )
	{
	}

	inline Short( const DLString& value ) throw( ExceptionBadType )
	{
		fromString( value );
	}

	
	DLString toString( ) const throw( );
	void fromString( const DLString& value ) throw( ExceptionBadType );
	
	inline short getValue( ) const
	{
		return value;
	}

	inline void setValue( short value )
	{
		this->value = value;
	}

	inline operator short & ( ) {
	    return value;
	}

private:
	short value;
};





/** Вывод в ostream типа Short */
inline std::ostream& operator << ( std::ostream& ostr, const Short& shortValue )
{
	ostr << shortValue.getValue( );
	return ostr;
}


#endif
