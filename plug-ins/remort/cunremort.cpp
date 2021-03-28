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
        ch->pecho("Usage: unremort <player_name> <#remort>\n\r"
                    "       - restore from remorts dir.");
        return;
    }

    buf.upperFirstCharacter( );

    for(c = char_list; c; c = c->next)
        if(!c->is_npc() && buf == c->getName()) {
            ch->pecho("Character is already in game.");
            return;
        }
    
    try {
        remort = num.toInt( );
    } catch (const ExceptionBadType &e) {
        ch->pecho("Number of remort must be a number.");
        return;
    }
        

    if(!PCharacterManager::pfRecover(buf, "remort", remort)) {
        ch->pecho("Oops. Failed to recover profile. Misspelled name?");
        return;
    }

    ch->pecho("Recovered.");
}


