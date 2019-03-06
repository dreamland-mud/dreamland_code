/* $Id: logstream.h,v 1.4.2.1.6.9 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          logstream.h  -  description
                             -------------------
    begin                : Thu Dec 7 2000
    copyright            : (C) 2000 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/



#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <string>
#include <fstream>

using std::ostream;
using std::ofstream;
using std::endl;
using std::string;

/**
 * @author Igor S. Petrenko
 * @short Вывод всякой информации на консоль
 */
class LogStream
{
public:
    static const char LSP_FATAL = 'F';
    static const char LSP_ERROR = 'E';
    static const char LSP_WARNING = 'W';
    static const char LSP_NOTICE = 'N';
    static const char LSP_SYSTEM = 'S';

public:
    virtual ~LogStream( );

    static ostream& send( char level );
    static void redirect(LogStream *s);
    
    static inline ostream& sendFatal( ) {
            return send( LSP_FATAL );
    }
    static inline ostream& sendError( ) {
            return send( LSP_ERROR );
    }
    static inline ostream& sendWarning( ) {
            return send( LSP_WARNING );
    }
    static inline ostream& sendNotice( ) {
            return send( LSP_NOTICE );
    }
    static inline ostream& sendSystem( ) {
            return send( LSP_SYSTEM );
    }
protected:
    virtual ostream &getStream(int level) = 0;
    virtual bool open( ) = 0;
    virtual void close( ) = 0;

    static LogStream *thiz;
};

class ConsoleLogStream : public LogStream {
protected:
    virtual ostream &getStream(int level);
    virtual bool open( );
    virtual void close( );
};

class FileLogStream : public LogStream {
public:
    FileLogStream(const string &pat);
    virtual ~FileLogStream( );
    
protected:
    virtual ostream &getStream(int level);
    virtual bool open( );
    virtual void close( );

private:
    string pattern;
    ofstream file;
};


/*
 * convenience routines 
 */
void formattedLog( char logLevel, const char *format, ... );
#define bug(format...)             formattedLog( LogStream::LSP_ERROR, format)
#define logerror             bug
#define warn(format...)             formattedLog( LogStream::LSP_WARNING, format)
#define notice(format...)    formattedLog( LogStream::LSP_NOTICE, format)
#define logsystem(format...) formattedLog( LogStream::LSP_SYSTEM, format)
#define syserr               logsystem

#endif
