/* $Id: lastlogstream.cpp,v 1.1.2.1.6.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include "lastlogstream.h"
#include "date.h"

ostringstream LastLogStream::data;

LastLogStream::~LastLogStream( )
{
}

ostream & LastLogStream::send( )
{
    data << Date::getCurrentTimeAsString( "[%b %d %H:%M:%S]: " );
    return data;
}

void LastLogStream::clear( )
{
    data.str( "" );
}
