/* $Id: dlfileop.h,v 1.1.2.5 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef DLFILEOP_H
#define DLFILEOP_H

#include <stdio.h>
#include <stdarg.h>
#include <sstream>

#include "dlfile.h"

/*
 * DLFileOp
 */
class DLFileOp : public DLFile {
public:
    DLFileOp( );
    virtual ~DLFileOp( );
    
    bool open( );
    bool close( );
    bool eof( );

    inline FILE * getFP( );

protected:
    FILE *fp;

private:
    virtual const char * getOpenMode( ) const = 0;
};

inline FILE * DLFileOp::getFP( )
{
    return fp;
}

/*
 * DLFileWriteable
 */
class DLFileWriteable : public DLFileOp {
public:
    virtual ~DLFileWriteable( );
    
    bool close( );
    bool flush( );

    bool write( const char *str );
    bool write( const DLString &str );
    bool write( ostringstream &buf );
    bool writeln( );
    bool writeln( const char * );
    bool printf( const char *fmt, ... );
    bool vprintf( const char *fmt, va_list ap );
};

/*
 * DLFileReadable
 */
class DLFileReadable : public DLFileOp {
public:
    virtual ~DLFileReadable( );
};

/*
 * DLFileAppend
 */
class DLFileAppend : public DLFileWriteable {
public:    
    DLFileAppend( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    DLFileAppend( const DLFile &dir, const DLFile &file = emptyFile );
    DLFileAppend( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );
private:
    virtual const char * getOpenMode( ) const;
};

/*
 * DLFileWrite
 */
class DLFileWrite : public DLFileWriteable {
public:
    DLFileWrite( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    DLFileWrite( const DLFile &dir, const DLFile &file = emptyFile );
    DLFileWrite( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );
private:
    virtual const char * getOpenMode( ) const;
};

/*
 * DLFileRead
 */
class DLFileRead : public DLFileReadable {
public:
    DLFileRead( );
    DLFileRead( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    DLFileRead( const DLFile &dir, const DLFile &file = emptyFile );
    DLFileRead( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );

    bool scanf( const char *fmt, ... );
    bool vscanf( const char *fmt, va_list ap );

private:
    virtual const char * getOpenMode( ) const;
};

#endif
