/* $Id: dbio.cpp,v 1.12.2.2.30.5 2010-09-01 08:21:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// dbio.cpp: implementation of the DBIO class.
//
//////////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include <sys/stat.h>

#include "dlfilestream.h"
#include "dbio.h"

const DLString DBIO::EXT_XML = ".xml";
        
DBIO::DBIO( const DLString & tableName )
        : table( tableName )
{
}

DBIO::DBIO( const DLString & tablePath, const DLString & tableName )
        : table( tablePath, tableName )
{
}

DBIO::DBIO( const DLDirectory & tableDir, const DLString & tableName )
        : table( tableDir, tableName )
{
}

DBIO::~DBIO( )
{
}

void DBIO::open( ) 
{
    table.open( );
}

void DBIO::open( const DLString &tableName ) 
{
    table.open( tableName );
}

DBIO::DBNode DBIO::nextXML( ) 
{
    DLFile entry = table.nextTypedEntry( EXT_XML );
    DLFileStream stream( table, entry );

    std::basic_ostringstream<char> buf;
    stream.toStream( buf );

    return DBNode( entry.getFileName( ), buf.str( ) );
}

void DBIO::insert( const DBIO::DBNode &dbNode ) 
{
    insert( dbNode.getKey( ), dbNode.getXML( ) );
}

void DBIO::safeInsert( const DBIO::DBNode &dbNode ) 
{
    safeInsert( dbNode.getKey( ), dbNode.getXML( ) );
}

void DBIO::insert( const DLString& key, const DLString& xml ) 
{
    // Writing straight onto the live entry truncates it first, so an interrupted
    // write (crash, full disk) destroys the previous contents with nothing to fall
    // back on. Everything goes through the verified temp-and-rename path now; the
    // fSafe flag on saveXML is kept only so existing call sites keep compiling.
    safeInsert( key, xml );
}

void DBIO::safeInsert( const DLString& key, const DLString& xml ) 
{
    DLFile tmpEntry = table.tempEntry( );

    {
        DLFileStream tmpStream( tmpEntry );
        tmpStream.fromString( xml );
    }

    // fromString cannot report a failed write -- it closes the stream and drops
    // the error state on the floor -- so a full disk leaves an empty or short
    // temp file behind. Renaming that over the live entry is exactly how three
    // player files were zeroed on 2026-08-02. Verify the bytes actually landed
    // before replacing anything: a save that cannot complete must leave the
    // previous entry untouched rather than truncate it.
    int written = tmpEntry.getSize( );

    if (written != (int)xml.size( )) {
        tmpEntry.remove( );
        throw ExceptionDBIO( "Refusing to replace '" + key + "': wrote "
                             + DLString( written ) + " of "
                             + DLString( (int)xml.size( ) )
                             + " bytes (out of disk space?)" );
    }

    // mkstemp makes the temp file 0600 and rename carries that mode over, which
    // would silently tighten entries created group-readable (languages/words.xml
    // is 0640). Carry the replaced entry's own mode across instead.
    DLFile entry = getEntryAsFile( key );
    struct stat st;
    bool hadMode = (::stat( entry.getCPath( ), &st ) == 0);

    if (!tmpEntry.rename( entry )) {
        tmpEntry.remove( );
        throw ExceptionDBIO( "Unable to replace entry '" + key + "'" );
    }

    if (hadMode)
        ::chmod( entry.getCPath( ), st.st_mode & 07777 );
}
        
DBIO::DBNode DBIO::select( const DLString& key ) 
{
    DLFileStream stream( getEntryAsFile( key ) );

    std::basic_ostringstream<char> buf;
    stream.toStream( buf );

    return DBNode( key, buf.str( ) );
}

void DBIO::remove( const DLString& key ) 
{
    if (!getEntryAsFile( key ).remove( ))
        throw ExceptionDBIO( "Unable to delete '" + key + "'" );
}

void DBIO::renameID( const DLString& oldKey, const DLString& newKey ) 
{
    DLFile oldEntry( table, oldKey, EXT_XML );
    DLFile backupTable( table, "backup" );
    DLFile newEntry( backupTable, newKey, EXT_XML );
    
    if (!oldEntry.rename( newEntry ))
        throw ExceptionDBIO( "Unable to rename id '" + oldKey + "' to '" + newKey + "'" );
}

DLFile DBIO::getEntryAsFile( const DLString &key )
{
    return DLFile( table, key, EXT_XML );
}

