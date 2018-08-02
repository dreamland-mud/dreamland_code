/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          exceptionxmlerror.h  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef EXCEPTIONXMLERROR_H
#define EXCEPTIONXMLERROR_H

#include "exception.h"
#include "dlstring.h"

/**
 * @author Igor S. Petrenko
 */
class ExceptionXMLError : public Exception
{
public: 
    inline ExceptionXMLError( string errmsg )
	    : Exception( errmsg ), line( 0 )
    {
    }
    inline ExceptionXMLError( string errmsg, char ch, int line )
	    : Exception( errmsg + "::" + ch + ": at line " + DLString(line) ), line( line )
    {
    }

    virtual ~ExceptionXMLError( ) throw( );
    
private:
    int line;
};

#endif
