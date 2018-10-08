/* $Id: dldirectory.h,v 1.1.2.5 2014-09-19 11:45:55 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef DLDIRECTORY_H
#define DLDIRECTORY_H

#include <dirent.h>

#include "config_io.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"
#include "dlfile.h"

class DLDirectory : public DLFile {
public:
    DLDirectory( );
    DLDirectory( const DLString &dirname, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString );
    DLDirectory( const DLFile &dir, const DLFile &file = emptyFile );
    DLDirectory( const DLFile &dir, const DLString &filename, const DLString &fileext = DLString::emptyString );
    virtual ~DLDirectory( );
    
    void open( const DLString &path ) throw( ExceptionDBIO );
    void open( ) throw( ExceptionDBIO );
    void close( );

    DLFile nextEntry( ) throw( ExceptionDBIOEOF );
    DLFile nextTypedEntry( const DLString &fileExt ) throw( ExceptionDBIOEOF );
    DLFile tempEntry( ) throw( ExceptionDBIO );

protected:
    DIR * dirp;
};

#endif        
