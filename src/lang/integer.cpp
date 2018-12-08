/* $Id: integer.cpp,v 1.9.34.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          integer.cpp  -  description
                             -------------------
    begin                : Mon Oct 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "integer.h"

const DLString Integer::TYPE = "Integer";


void Integer::fromString( const DLString & value ) throw( ExceptionBadType )
{
        std::basic_istringstream<char> s( value.c_str( ) );
        int i;
                  
        s >> i;

        if( s )
        {
                this->value = i;
                return;
        }
        throw ExceptionBadType( TYPE, value );
}

DLString Integer::toString( ) const throw( )
{
        std::basic_ostringstream<char>  buf;

        buf << value;
        return buf.str( );
}

bool Integer::tryParse(Integer &target, const DLString &value)
{
    try {
        target.fromString(value);
    } catch (const ExceptionBadType &ex) {
        return false;
    }

    return true;
}

