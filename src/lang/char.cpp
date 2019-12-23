/* $Id: char.cpp,v 1.3.2.1.30.6 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          char.cpp  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "char.h"
#include "dl_ctype.h"

const DLString Char::TYPE = "Char";

void Char::fromString( const DLString & value ) 
{
    this->value = value[0];
}

DLString Char::toString( ) const 
{
    return DLString( value );
}

bool Char::isDelimiter( ) const
{
    return dl_isdelim( value );
}

bool Char::isUpper( ) const
{
    return dl_isupper( value );
}

bool Char::isLower( ) const
{
    return dl_islower( value );
}

char Char::lower( char c )
{
    return dl_tolower( c );
}

char Char::upper( char c )
{
    return dl_toupper( c );
}

