/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          exceptionbaddatestring.h  -  description
                             -------------------
    begin                : Thu Sep 27 2001
    copyright            : (C) 2001 by nofate
    email                : nofate@black
 ***************************************************************************/

#ifndef EXCEPTIONBADDATESTRING_H
#define EXCEPTIONBADDATESTRING_H

#include "exception.h"
#include "dlstring.h"

/**
 * @author nofate
 */
class ExceptionBadDateString : public Exception
{
public: 
	inline ExceptionBadDateString( const DLString& date, const char* message, int position )
		: Exception( DLString( message ) << " '" << date << "' at position " << position )
	{
	}
};


#endif
