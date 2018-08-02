/* $Id: byte.cpp,v 1.3.34.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          byte.cpp  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "byte.h"

const DLString Byte::TYPE = "Byte";


void Byte::fromString( const DLString & value ) throw( ExceptionBadType )
{
	std::basic_istringstream<char> s( value.c_str( ) );
	unsigned char i;
  		
	s >> i;

	if( s )
	{
		this->value = i;
		return;
	}
	throw ExceptionBadType( TYPE, value );
}

DLString Byte::toString( ) const throw( )
{
	std::basic_ostringstream<char>  buf;

	buf << value;
	return buf.str( );
}
