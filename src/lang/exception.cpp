/* $Id: exception.cpp,v 1.2.4.7 2014-09-19 11:41:34 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/*
 * NoFate, 2001
 */
#ifndef __MINGW32__ 
#include <execinfo.h>
#endif
#include <stdio.h>

#include "exception.h"
#include "exceptionbadtype.h"
#include "fileformatexception.h"

Exception::Exception( const string &str )  : message( str )
{
    fillStackFrames(NULL);
}

const char*
Exception::what( ) const throw()
{
    return message.c_str( );
}

Exception::~Exception( ) 
{
}

void
Exception::setStr( const string& str )
{
    message = str;
}

void Exception::fillStackFrames( void *a )
{
#ifndef __MINGW32__
    void *stack[50];
    size_t size;

    size = backtrace(stack, 50);

    callstack.insert(callstack.end(), stack, stack+size);
#endif
}

void Exception::printStackTrace( std::ostream &os ) const
{
    std::vector<void *>::const_iterator it;
    
    os << "Exception: " << message << std::endl;

    for(it = callstack.begin( ); it != callstack.end( ); it++) {
        void *ip = *it;

        os << "  at " << ip << std::endl;
    }
}

FileFormatException::FileFormatException( const char * fmt, ... ) 
{
    va_list ap;
    char buf[1024];
    
    va_start( ap, fmt );
    vsnprintf( buf, sizeof( buf ), fmt, ap );
    va_end( ap );

    setStr( string( buf ) );            
}

FileFormatException::~FileFormatException( ) 
{
}

ExceptionBadType::~ExceptionBadType( ) 
{
}

