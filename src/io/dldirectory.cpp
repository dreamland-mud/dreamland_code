/* $Id: dldirectory.cpp,v 1.1.2.7 2014-09-19 11:45:55 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <unistd.h>
#include <stdlib.h>

#include "logstream.h"
#include "dldirectory.h"

DLDirectory::DLDirectory( )
                  : dirp( 0 )
{
}

DLDirectory::DLDirectory( const DLString &dirname, const DLString &filename, const DLString &fileext )
                  : dirp( 0 )
{
    makePath( dirname, filename, fileext );
}

DLDirectory::DLDirectory( const DLFile &dir, const DLFile &file )
                  : dirp( 0 )
{
    makePath( dir, file );
}

DLDirectory::DLDirectory( const DLFile &dir, const DLString &filename, const DLString &fileext )
                  : dirp( 0 )
{
    makePath( dir, filename, fileext );
}

DLDirectory::~DLDirectory( )
{
    close( );
}

void DLDirectory::open( const DLString &path ) 
{
    close( );
    this->path = path;
    open( );
}

void DLDirectory::open( ) 
{
    dirp = opendir( path.c_str( ) );
    
    if (dirp == 0) 
        throw ExceptionDBIO( "Unable to open '" + path + "' directory" );
}

void DLDirectory::close( )
{
    if (dirp != 0) {
        CLOSEDIR( dirp );
        dirp = 0;
    }
}

DLFile DLDirectory::nextEntry( ) 
{
    if (dirp == 0)
        throw ExceptionDBIO( "Directory '" + path + "' already closed" );

    dirent *dp = readdir( dirp );


    if (dp == 0) {
        close( );
        throw ExceptionDBIOEOF( );
    }

    return DLFile( dp->d_name );
}

DLFile DLDirectory::nextTypedEntry( const DLString &fileExt ) 
{
    DLFile local = nextEntry( );
    DLFile global( *this, local );

    if (global.isDirectory( )) {
        return nextTypedEntry( fileExt );
    }
    
    if (local.getFileExt( ) != fileExt) {
        return nextTypedEntry( fileExt );
    }
    
    return local;
}


DLFile DLDirectory::tempEntry( ) 
{
    char templateBuf[1000], *pTemplate;

    sprintf( templateBuf, "%s%cXXXXXXX", path.c_str( ), PATH_SEP );
    pTemplate = templateBuf;
    int fd = mkstemp( pTemplate );
    
    if (fd < 0) 
        throw ExceptionDBIO( "Unable to create tmp file in '" + path + "'" );

    ::close(fd);
    return DLFile( pTemplate );
}

