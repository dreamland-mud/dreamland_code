/* $Id: cbackup.cpp,v 1.1.2.2.10.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "merc.h"
#include "def.h"

CMDADM( backup )
{
    Character *c;
    DLString arguments = constArguments; 
    DLString buf = arguments.getOneArgument( );
    
    if(buf.empty( )) {
        ch->pecho("Usage: backup <player_name>\n\r"
                    "       - backup player.");
        return;
    }

    buf.upperFirstCharacter( );

    for(c = char_list; c; c = c->next)
        if(!c->is_npc() && buf == c->getName( )) {
            ch->pecho("Character in game. Force saving.");
            c->getPC( )->save();
            break;
        }

    if (!PCharacterManager::pfBackup(buf)) {
        ch->pecho("Oops. Failed to backup player profile.");
        return;
    }

    ch->pecho("Backup done.");
}


