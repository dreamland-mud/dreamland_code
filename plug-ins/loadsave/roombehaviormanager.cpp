/* $Id: roombehaviormanager.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#include "roombehaviormanager.h"
#include "roombehavior.h"

#include "logstream.h"
#include "room.h"

#include "fread_utils.h"
#include "mercdb.h"

void RoomBehaviorManager::setAll( )
{
    LogStream::sendNotice( ) << "Assigning room behaviors..." << endl;

    for (Room *r = room_list; r; r = r->rnext)
        if (r->behavior)
            r->behavior->setRoom( r );
}

void RoomBehaviorManager::parse( Room * pRoom, FILE *fp ) 
{
    char letter;
    char *word;
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
        
    word = fread_string( fp );

    try {
        std::basic_istringstream<char> istr( word );
        
        pRoom->behavior.fromStream( istr );

    } catch (Exception e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
        
    free_string( word );
}

void RoomBehaviorManager::save( const Room *pRoom, FILE *fp ) 
{
    std::basic_ostringstream<char> ostr;
     
    if (!pRoom->behavior)
        return;

    try {
        pRoom->behavior.toStream( ostr );

        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (ExceptionXMLError e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

