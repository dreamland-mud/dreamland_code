/* $Id: oneallocate.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 * 
 * ruffina, DreamLand, 2003
 */
/***************************************************************************
                          oneallocate.cpp  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <typeinfo>
#include "dlobject.h"
#include "oneallocate.h"
#include "dlstring.h"
#include "exception.h"

OneAllocate::~OneAllocate( )
{
}

void OneAllocate::checkDuplicate( DLObject* object )
{
    if( object != 0 )
	throw Exception( DLString( "Duplicate static class " )
			 + typeid( *object ).name( ) );
}
