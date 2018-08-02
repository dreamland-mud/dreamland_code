/* $Id: dlfile.h,v 1.1.2.7 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef DLFILE_H
#define DLFILE_H

#include "dlstring.h"

/*
 * DLFile
 */
class DLFile {
public:
    DLFile( );
    DLFile( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    DLFile( const DLFile &dir, const DLFile &file = emptyFile );
    DLFile( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );
    virtual ~DLFile( );
    
    inline const DLString &getPath( ) const;
    inline const char * getCPath( ) const;
    DLString getAbsolutePath( ) const;
    DLFile & toAbsolutePath( );
    DLString getFileName( ) const;
    DLString getFileExt( ) const;
    
    bool isDirectory( ) const;
    bool exist( ) const;

    time_t getModifyTime( ) const;
    int getSize( ) const;
    
    bool touch( ) const;
    bool remove( ) const;
    bool rename( const DLFile &newFile ) const;
    bool rename( const DLString &newPath ) const;
    bool copy( const DLFile &newFile ) const;
    bool copy( const DLString &newPath ) const;

public:
    static const char PATH_SEP = '/';
    static const char EXT_SEP = '.';
    static const char * EOL;
    static const DLFile emptyFile;

protected:
    void makePath( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    void makePath( const DLFile &dir, const DLFile &file = emptyFile );
    void makePath( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );

    DLString path;
};

inline const DLString & DLFile::getPath( ) const
{
    return path;
}

inline const char * DLFile::getCPath( ) const
{
    return path.c_str( );
}

#endif

