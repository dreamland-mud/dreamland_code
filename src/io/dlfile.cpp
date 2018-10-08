/* $Id: dlfile.cpp,v 1.1.2.10 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <fstream>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>

#include "dlfile.h"
#include "logstream.h"

/*------------------------------------------------------------------------
 * DLFile
 *-----------------------------------------------------------------------*/
const char * DLFile::EOL = "\n";
const DLFile DLFile::emptyFile;

DLFile::DLFile( )
{
}

DLFile::DLFile( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    makePath( dirname, filename, fileext );
}

DLFile::DLFile( const DLFile &dir, const DLFile &file )
{
    makePath( dir, file );
}

DLFile::DLFile( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir, filename, fileext );
}

DLFile::~DLFile( )
{
}

void DLFile::makePath( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    path = dirname;

    if (!filename.empty( )) {
        path << PATH_SEP << filename << fileext;
    }
}

void DLFile::makePath( const DLFile &dir, const DLFile &file )
{
    makePath( dir.getPath( ), file.getPath( ) );
}

void DLFile::makePath( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir.getPath( ), filename, fileext );
}


DLString DLFile::getAbsolutePath( ) const
{
    static const char * METHOD = "DLFile::getAbsolutePath";
    char buf[PATH_MAX];
    
    if (realpath( path.c_str( ), buf )) {
        return DLString( buf );
    }
    else {
        LogStream::sendSystem( ) << METHOD << " " << buf << endl;
        return DLString::emptyString;
    }
}

DLFile & DLFile::toAbsolutePath( )
{
    DLString apath = getAbsolutePath( );

    if (!apath.empty( ))
        path = apath;

    return *this;
}

bool DLFile::isDirectory( ) const
{
    static const char * METHOD = "DLFile::isDirectory";
    struct stat statBuf;
    
    if (stat( path.c_str( ), &statBuf ) == 0) {
        if (S_ISDIR( statBuf.st_mode )) {
            return true;
        }
    }
    else {
        LogStream::sendSystem( ) << METHOD << " " << path << endl;
    }

    return false;
}

bool DLFile::exist( ) const
{
    static const char * METHOD = "DLFile::exist";
    struct stat statBuf;
    
    if (stat( path.c_str( ), &statBuf ) != -1)
        return true;

    if (errno == ENOENT)
        return false;

    LogStream::sendSystem( ) << METHOD << " " << path << endl;
    return false;
}

DLString DLFile::getFileName( ) const
{
    DLString::size_type npos = path.rfind( PATH_SEP );
    DLString::size_type epos = path.rfind( EXT_SEP );
    
    if (epos == DLString::npos) {
        epos = path.length( );        
    }

    if (npos == path.length( ) - 1) {
        return DLString::emptyString;
    }
    else if (npos == DLString::npos) {
        npos = 0;
    }
    else {
        npos = npos + 1;
    }

    return path.substr( npos, epos - npos );
}

DLString DLFile::getFileExt( ) const
{
    DLString::size_type npos = path.rfind( PATH_SEP );
    DLString::size_type epos = path.rfind( EXT_SEP );
    
    if (epos == DLString::npos) {
        return DLString::emptyString;
    }
    if (npos != DLString::npos && epos < npos) {
        return DLString::emptyString;
    }
    
    return path.substr( epos );
}

bool DLFile::touch( ) const
{
    static const char * METHOD = "DLFile::touch";

    if (utime( path.c_str( ), NULL ) != -1) 
        return true;

    if (errno == ENOENT)
        if (fopen( path.c_str( ), "a" ))
            return true;

    LogStream::sendSystem( ) << METHOD << " " << path << endl;
    return false;
}

bool DLFile::remove( ) const
{
    static const char * METHOD = "DLFile::remove";

    if (::remove( path.c_str( ) ) < 0) {
        LogStream::sendSystem( ) << METHOD << " " << path << endl;
        return false;
    }

    return true;
}

bool DLFile::rename( const DLFile &newFile ) const
{
    return rename( newFile.getPath( ) );
}

bool DLFile::rename( const DLString &newPath ) const
{
    static const char * METHOD = "DLFile::rename";

#ifdef __MINGW32__
    ::remove( newPath.c_str( ) );
#endif

    if (::rename( path.c_str( ), newPath.c_str( ) ) < 0) {
        LogStream::sendSystem( ) << METHOD << " " << path << " " << newPath << endl;
        return false;
    }

    return true;
}

bool DLFile::copy( const DLFile &newFile ) const
{
    return copy( newFile.getPath( ) );
}

bool DLFile::copy( const DLString &newPath ) const
{
    std::ifstream ifs( getPath( ).c_str( ) );

    if (!ifs)
        return false;

    std::ofstream ofs( newPath.c_str( ) );

    if (!ofs)
        return false;
    
    ofs << ifs.rdbuf( );

    return true;
}

time_t DLFile::getModifyTime( ) const
{
    static const char * METHOD = "DLFile::getModifyTime";
    struct stat statBuf;
    
    if (stat( path.c_str( ), &statBuf ) == 0) 
        return statBuf.st_mtime;
    else {
        LogStream::sendSystem( ) << METHOD << " " << path << endl;
        return 0;
    }
}

int DLFile::getSize( ) const
{
    static const char * METHOD = "DLFile::getSize";
    struct stat statBuf;
    
    if (stat( path.c_str( ), &statBuf ) == 0) 
        return statBuf.st_size;
    else {
        LogStream::sendSystem( ) << METHOD << " " << path << endl;
        return 0;
    }
}

