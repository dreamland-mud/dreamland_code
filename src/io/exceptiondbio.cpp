/* $Id: exceptiondbio.cpp,v 1.3.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include <errno.h>
#include <string.h>
#include "exceptiondbio.h"

ExceptionDBIO::ExceptionDBIO( const DLString &str ) throw( )
{
    if (errno > 0) {
        setStr( str + " (" + strerror( errno ) + ")" );
    }
    else {
        setStr( str );
    }
}

ExceptionDBIO::~ExceptionDBIO( ) throw( )
{
}
