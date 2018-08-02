/* $Id: cunremort.cpp,v 1.1.2.3.10.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "merc.h"
#include "def.h"

CMDADM( unremort )
{
    Character *c;
    int remort = -1;
    DLString arguments = constArguments; 
    DLString buf = arguments.getOneArgument( );
    DLString num = arguments.getOneArgument( );
    
    if(buf.empty( ) || num.empty( )) {
	ch->send_to("Usage: unremort <player_name> <#remort>\n\r"
		    "       - restore from remorts dir.\n\r");
	return;
    }

    buf.upperFirstCharacter( );

    for(c = char_list; c; c = c->next)
	if(!c->is_npc() && buf == c->getName()) {
	    ch->send_to("Character is already in game.\n\r");
	    return;
	}
    
    try {
	remort = num.toInt( );
    } catch (const ExceptionBadType &e) {
	ch->send_to("Number of remort must be a number.\n\r");
	return;
    }
	

    if(!PCharacterManager::pfRecover(buf, "remort", remort)) {
	ch->send_to("Oops. Failed to recover profile. Misspelled name?\n\r");
	return;
    }

    ch->send_to("Recovered.\n\r");
}


