/* $Id: dlfileop.cpp,v 1.1.2.8 2010-01-01 15:14:15 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "logstream.h"
#include "dlfileop.h"

/*------------------------------------------------------------------------
 * DLFileOp
 *-----------------------------------------------------------------------*/
DLFileOp::DLFileOp( )
          : fp( 0 )
{
}

DLFileOp::~DLFileOp( )
{
    close( );
}

bool DLFileOp::open( )
{
    static const char * METHOD = "DLFileOp::open";
    
    if (fp)
	return true;

    if (path.empty( )) {
	LogStream::sendError( ) << METHOD << " empty path" << endl;
	return false;
    }

    fp = fopen( path.c_str( ), getOpenMode( ) );
    if (!fp) {
	LogStream::sendSystem( ) << METHOD << " " << path << endl;
	return false;
    }

    return true;
}

bool DLFileOp::close( )
{
    static const char * METHOD = "DLFileOp::close";
    bool rc = true;

    if (!fp)
	return rc;
    
    if (fclose( fp ) != 0) {
	LogStream::sendSystem( ) << METHOD << " " << path << endl;
	rc = false;
    }

    fp = 0;
    return rc;
}

bool DLFileOp::eof( )
{
    return feof( fp );
}

/*------------------------------------------------------------------------
 * DLFileWriteable
 *-----------------------------------------------------------------------*/
DLFileWriteable::~DLFileWriteable( )
{
    close( );
}

bool DLFileWriteable::close( )
{
    flush( );
    return DLFileOp::close( );
}

bool DLFileWriteable::flush( )
{
    static const char * METHOD = "DLFileWriteable::flush";
    
    if (!fp)
	return true;

    if (fflush( fp ) != 0) {
	LogStream::sendSystem( ) << METHOD << " " << path << endl;
	return false;
    }
    
    return true;
}

bool DLFileWriteable::printf( const char *fmt, ... )
{
    va_list ap;
    bool rc;

    va_start( ap, fmt );
    rc = vprintf( fmt, ap );
    va_end( ap );

    return rc;
}

bool DLFileWriteable::vprintf( const char *fmt, va_list ap )
{
    static const char * METHOD = "DLFileWriteable::vprintf";

    if (!open( ))
	return false;
    
    if (vfprintf( fp, fmt, ap ) < 0) {
	LogStream::sendSystem( ) << METHOD << " " << path << " " << fmt << endl;
	return false;
    }

    return true;
}

bool DLFileWriteable::write( const char *str )
{
    static const char * METHOD = "DLFileWriteable::write";

    if (!open( ))
	return false;

    if (fputs( str, fp ) < 0) {
	LogStream::sendSystem( ) << METHOD << " " << path << " " << str << endl;
	return false;
    }

    return true;
}

bool DLFileWriteable::write( const DLString &str )
{
    return write( str.c_str( ) );
}

bool DLFileWriteable::write( ostringstream &buf )
{
    return write( buf.str( ).c_str( ) );
}

bool DLFileWriteable::writeln( )
{
    return write( EOL );
}

bool DLFileWriteable::writeln( const char *str )
{
    return write( str ) && writeln( );
}

/*------------------------------------------------------------------------
 * DLFileReadable
 *-----------------------------------------------------------------------*/
DLFileReadable::~DLFileReadable( )
{
}

/*------------------------------------------------------------------------
 * DLFileAppend
 *-----------------------------------------------------------------------*/
DLFileAppend::DLFileAppend( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    makePath( dirname, filename, fileext );
}

DLFileAppend::DLFileAppend( const DLFile &dir, const DLFile &file )
{
    makePath( dir, file );
}

DLFileAppend::DLFileAppend( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir, filename, fileext );
}

const char * DLFileAppend::getOpenMode( ) const
{
    static const char * MODE = "a";
    return MODE;
}

/*------------------------------------------------------------------------
 * DLFileWrite
 *-----------------------------------------------------------------------*/
DLFileWrite::DLFileWrite( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    makePath( dirname, filename, fileext );
}

DLFileWrite::DLFileWrite( const DLFile &dir, const DLFile &file )
{
    makePath( dir, file );
}

DLFileWrite::DLFileWrite( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir, filename, fileext );
}

const char * DLFileWrite::getOpenMode( ) const
{
    static const char * MODE = "w";
    return MODE;
}

/*------------------------------------------------------------------------
 * DLFileRead
 *-----------------------------------------------------------------------*/
DLFileRead::DLFileRead( )
{
}

DLFileRead::DLFileRead( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    makePath( dirname, filename, fileext );
}

DLFileRead::DLFileRead( const DLFile &dir, const DLFile &file )
{
    makePath( dir, file );
}

DLFileRead::DLFileRead( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir, filename, fileext );
}

const char * DLFileRead::getOpenMode( ) const
{
    static const char * MODE = "r";
    return MODE;
}

bool DLFileRead::scanf( const char *fmt, ... )
{
    va_list ap;
    bool rc;

    va_start( ap, fmt );
    rc = vscanf( fmt, ap );
    va_end( ap );

    return rc;
}

bool DLFileRead::vscanf( const char *fmt, va_list ap )
{
    static const char * METHOD = "DLFileWriteable::vscanf";

    if (!open( ))
	return false;
    
    if (vfscanf( fp, fmt, ap ) < 0) {
	LogStream::sendSystem( ) << METHOD << " " << path << " " << fmt << endl;
	return false;
    }

    return true;
}

