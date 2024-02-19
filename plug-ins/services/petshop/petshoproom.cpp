/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "petshoproom.h"
#include "petshopstorage.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"



/*----------------------------------------------------------------------
 * PetShopRoom
 *---------------------------------------------------------------------*/
PetShopRoom::PetShopRoom( ) 
{
}

PetShopStorage::Pointer PetShopRoom::getStorage( )
{
    Room *storageRoom;
    PetShopStorage::Pointer storage;

    if (( storageRoom = get_room_instance( storageVnum ) ) == NULL) {
        LogStream::sendError( ) << getType( ) << ": zero room for storage " << storageVnum << endl;
        return storage;
    }

    if (!storageRoom->behavior 
        || !( storage = storageRoom->behavior.getDynamicPointer<PetShopStorage>( ) )) 
    {
        LogStream::sendError( ) << getType( ) << ": zero behavior for storage " << storageVnum << endl;
        return storage;
    }

    return storage;
}

bool PetShopRoom::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    PetShopStorage::Pointer storage = getStorage( );

    if (!storage)
        return false;

    if (ch->is_npc( ))
        return false;
    
    if (cmdName == "list") {
        storage->doList( ch->getPC( ) );
        return true;
    }

    if (cmdName == "buy" ) {
        storage->doBuy( ch->getPC( ), cmdArgs );
        return true;
    }

    return false;
}

