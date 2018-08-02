/* $Id: boolean.cpp,v 1.2.34.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          boolean.cpp  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "boolean.h"

const DLString Boolean::TYPE = "Boolean";
const DLString Boolean::VAL_TRUE = "true";
const DLString Boolean::VAL_FALSE = "false";

void Boolean::fromString( const DLString & value ) throw( ExceptionBadType )
{
    if( value == VAL_TRUE )
	this->value = true;
    else if( value == VAL_FALSE )
	this->value = false;
    else
	throw ExceptionBadType( TYPE, value );
}

DLString Boolean::toString( ) const throw( )
{
    return value ? VAL_TRUE : VAL_FALSE;
}
