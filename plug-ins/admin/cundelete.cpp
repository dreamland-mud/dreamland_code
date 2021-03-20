/* $Id: cundelete.cpp,v 1.1.2.2.10.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "merc.h"
#include "def.h"

CMDADM( undelete )
{
    Character *c;
    DLString arguments = constArguments; 
    DLString buf = arguments.getOneArgument( );
    
    if(buf.empty( )) {
        ch->pecho("Usage: undelete <player_name>");
        return;
    }

    buf.upperFirstCharacter( );

    for(c = char_list; c; c = c->next)
        if(!c->is_npc() && buf == c->getName( )) {
            ch->pecho("Character is already in game.");
            return;
        }

    if(!PCharacterManager::pfRecover(buf, "delete", 0)) {
        ch->pecho("Не могу восстановить профайл. Возможно, забыли убрать случайное расширение в конце имени файла.");
        return;
    }

    ch->pecho("Recovered.");
}


