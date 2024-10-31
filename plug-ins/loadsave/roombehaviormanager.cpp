/* $Id: roombehaviormanager.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#include "roombehaviormanager.h"
#include "roombehavior.h"

#include "logstream.h"
#include "room.h"

#include "fread_utils.h"
#include "merc.h"
#include "def.h"

void RoomBehaviorManager::parse( RoomIndexData * pRoom, FILE *fp ) 
{
    char letter;
    DLString word;
    istringstream istr;

    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
        
    word = fread_dlstring( fp );

    try {
        istr.str( word );
        pRoom->behavior.construct();
        pRoom->behavior->load(istr);

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void RoomBehaviorManager::save( const RoomIndexData *pRoom, FILE *fp ) 
{
    std::basic_ostringstream<char> ostr;
     
    if (!pRoom->behavior)
        return;

    try {
        pRoom->behavior->save( ostr );
        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (const ExceptionXMLError &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void RoomBehaviorManager::assign(Room *room)
{
    if (!room->pIndexData->behavior)
        return;
    
    try {
        if (room->behavior) {
            room->behavior->unsetRoom( );
            room->behavior.clear( );
        }

        const XMLNode::Pointer &rootNode = room->pIndexData->behavior->getFirstNode();                
        room->behavior.fromXML( rootNode );
        room->behavior->setRoom(room);

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }

}
