/* $Id: short.cpp,v 1.2.34.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          short.cpp  -  description
                             -------------------
    begin                : Mon Oct 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "short.h"

const DLString Short::TYPE = "Short";


void Short::fromString( const DLString & str ) throw( ExceptionBadType )
{
	std::basic_istringstream<char> s( str.c_str( ) );
	short i;
  		
	s >> i;

	if( s )
	{
		this->value = i;
		return;
	}
	throw ExceptionBadType( TYPE, str );
}

DLString Short::toString( ) const throw( )
{
	std::basic_ostringstream<char>  buf;

	buf << value;
	return buf.str( );
}
