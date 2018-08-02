/* $Id: dlfilestream.cpp,v 1.1.2.4 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <sstream>

#include "logstream.h"
#include "dlfilestream.h"

DLFileStream::DLFileStream( const DLString &dirname, const DLString &filename, const DLString &fileext )
{
    makePath( dirname, filename, fileext );
}

DLFileStream::DLFileStream( const DLFile &dir, const DLFile &file )
{
    makePath( dir, file );
}

DLFileStream::DLFileStream( const DLFile &dir, const DLString &filename, const DLString &fileext )
{
    makePath( dir, filename, fileext );
}

DLFileStream::~DLFileStream( )
{
}

void DLFileStream::openInputStream( ) throw( ExceptionDBIO )
{
    istr.open( path.c_str( ) );
    
    if (!istr) 
	throw ExceptionDBIO( "Unable to open input stream for '" + path + "'" );
}

void DLFileStream::openOutputStream( ) throw( ExceptionDBIO )
{
    ostr.open( path.c_str( ) );
    
    if (!ostr) 
	throw ExceptionDBIO( "Unable to open output stream for '" + path + "'" );
}

void DLFileStream::toStream( std::ostream &buf ) throw( ExceptionDBIO )
{
    char c;

    openInputStream( );

    while (istr.get( c )) {
	buf << c;
    }

    istr.close( );
}


void DLFileStream::fromString( const DLString &source ) throw( ExceptionDBIO )
{
    openOutputStream( );
    ostr << source;
    ostr.close( );
}


