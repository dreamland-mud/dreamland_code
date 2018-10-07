/* $Id$
 *
 * ruffina, 2004
 */
#include "backdoorhandler.h"
#include "interprethandler.h"
#include "descriptor.h"
#include "descriptorstatemanager.h"
#include "defaultbufferhandler.h"
#include "comm.h"
#include "codepage.h"
#include "colour.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"

#include "loadsave.h"
#include "interp.h"
#include "wiznet.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

bool password_check( PCMemoryInterface *pci, const DLString &plainText );

void BackdoorHandler::init( Descriptor *d ) 
{
    d->handle_input.clear( );
    d->handle_input.push_front( new BackdoorHandler );

    d->send( ANSI_CLEARSCR ANSI_HOME ANSI_COLOR_RESET "\n\r" );
    do_help( d, "BACKDOORGREETING", false );
}

int BackdoorHandler::handle(Descriptor *d, char *arg) 
{
    int num, oldState;
    char *cp, *name, *pwd;
    PCMemoryInterface *pcm;
    PCharacter *pch;
    Descriptor *d0;
    
    cp   = strtok(arg," "); 
    name = strtok(NULL," ");
    pwd  = strtok(NULL," ");
    
    d->echoOn( );

    if (!cp || !cp[0] || (num = cp[0] - '1') < 0 || num >= NCODEPAGES) {
	d->send( "Wrong codepage.\r\n" );
	d->close( );
	return -1;
    }
    
    if (!name || !( pcm = PCharacterManager::find( name ) )) {
	d->send( "Wrong name.\r\n" );
	d->close( );
	return -1;
    }

    if (!pwd || !password_check( pcm, pwd )) {
	d->send( "Wrong password.\r\n" );
	d->close( );
	return -1;
    }
    
    d->buffer_handler = new DefaultBufferHandler( num );
    d->send( "Welcome to Dream Land!\r\n" );
    
    /* close other descs for this character, waiting in the nanny */
    while (( d0 = descriptor_find_named( d, pcm->getName( ), CON_NANNY ) )) 
	d0->close( );
    
    /* already playing - reconnect or reanimate */
    if (( pch = pcm->getPlayer( ) )) {
	oldState = CON_BREAK_CONNECT;
	if (pch->desc)
	    pch->desc->close( );
    }
    else { /* load and link to the world anew */
	oldState = CON_READ_MOTD;
	pch = PCharacterManager::create( pcm->getName( ) );
	PCharacterManager::update( pch );
	
	char_to_list( pch, &char_list );

        Room *start_room = get_room_index( pch->start_room  );
        if (!start_room)
            start_room = get_room_index( ROOM_VNUM_TEMPLE );
	char_to_room( pch, start_room );

	if (pch->pet) {
            // If room was already set in fread_pet, simply place the beast there.
            // Otherwise use master's room.
            if (pch->pet->in_room) 
                char_to_room( pch->pet, pch->pet->in_room);
            else 
                char_to_room( pch->pet, pch->in_room );
        }
    }

    d->associate( pch );
    InterpretHandler::init( d );

    DescriptorStateManager::getThis( )->handle( oldState, CON_PLAYING, d );
    wiznet( WIZ_LOGINS, 0, pch->get_trust( ), "%C1 входит в DreamLand через заднюю дверь.", pch );
    return 1;
}

void BackdoorHandler::prompt(Descriptor *d)
{
}


