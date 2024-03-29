/* $Id: logstream.cpp,v 1.2.34.8 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          logstream.cpp  -  description
                             -------------------
    begin                : Thu Dec 7 2000
    copyright            : (C) 2000 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <iostream>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "logstream.h"
#include "date.h"

LogStream *LogStream::thiz = new ConsoleLogStream( );

LogStream::~LogStream( )
{
}

ostream&
LogStream::send( char level )
{
    int error = errno;
    DLString date = Date::getCurrentTimeAsString( "[%b %d %H:%M:%S]:" );
    ostream &stream = thiz->getStream(level);

    stream << date << level << ": ";

    if (level == LSP_SYSTEM) {
        if (error > 0) {
            stream << "(" << strerror( error ) << ") ";
        }
    }

    return stream;
}

void 
LogStream::redirect(LogStream *s) 
{
    if(s->open( )) {
        sendNotice( ) << "log stream closed" << endl;
        thiz->close( );
        thiz = s;
        sendNotice( ) << "log stream opened" << endl;
    }
}

ostream &
ConsoleLogStream::getStream(int level)
{
    if(level == LSP_NOTICE) {
        return std::cout;
    } else {
        return std::cerr;
    }
}

bool
ConsoleLogStream::open( )
{
    return true;
}

void 
ConsoleLogStream::close( )
{
}

FileLogStream::FileLogStream(const string &pat) : pattern(pat)
{
}

FileLogStream::~FileLogStream( )
{
}

ostream &
FileLogStream::getStream(int level)
{
    return file;
}

bool 
FileLogStream::open( )
{
    DLString fn = Date::getCurrentTimeAsString(pattern.c_str( ));
    file.open(fn.c_str( ), ios::app);
    
    if(file)
        return true;
    
    sendSystem() << "failed to open " << fn << endl;
    return false;
}

void 
FileLogStream::close( )
{
    file.close( );
}

/*
 * Convenience routines for perror-like logging 
 */
void formattedLog( char logLevel, const char *format, ... )
{
    char buf[4000];
    va_list ap;

    va_start( ap, format );
    vsprintf( buf, format, ap );
    va_end( ap );

    LogStream::send( logLevel ) << buf << endl;
}

