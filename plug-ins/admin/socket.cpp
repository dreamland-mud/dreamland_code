/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          socket.cpp  -  description
                             -------------------
    begin                : Fri Apr 13 2001
    copyright            : (C) 2001 by nofate
    email                : nofate@europe.com
 ***************************************************************************/

#include <string.h>
#include <arpa/inet.h>

#include "admincommand.h"
#include "serversocketcontainer.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "merc.h"
#include "descriptor.h"
#include "screenreader.h"
#include "websocketrpc.h"
#include "act.h"

const char *ttype_name( int ttype );

CMDADM( socket )
{
    DLString arg = constArguments;
    PCMemoryInterface *pcm;
    Descriptor *d;
    DLString name;
    int             count;

    ch->pecho("\n\r[Num Connected  Login Idl Client] Player Name  Host            ");
    ch->pecho("--------------------------------------------------------------------------");
    count = 0;

    for (d = descriptor_list; d; d = d->next) {
        const char *myHost, *myIP;
        const char * state;
        DLString logon;
        DLString idle;
        DLString client;
        DLString extraInfo;

        if (d->character) {
            PCharacter *player;

            player = d->character->getPC();
            name = player->getName();
            pcm = PCharacterManager::find( name );
            
            if (pcm) {
                if (!ch->can_see(player))
                    continue;

                // hide wizard login attempt
                if (pcm->get_trust( ) > ch->get_trust( ))
                    continue;
            }
        }
        else {
            name = "Unknown";
        }
        
        if (d->character && d->connected == CON_PLAYING) {
            logon = d->character->getPC( )->age.getLogon( ).getTimeAsString( "%H:%M" );
            idle = fmt(0, "%3d", d->character->timer);
        }
        else {
            logon = "-----";
            idle = "   ";
        }
        
        switch (d->connected) {
        case CON_PLAYING:   state = " PLAYING  ";        break;
        case CON_CODEPAGE:  state = " Codepage ";        break;
        case CON_NANNY:     state = "  Nanny   ";        break;
        default:            state = " UNKNOWN! ";        break;
        }
        
        count++;
        
        if (!d->via.empty( )) {
            myHost = d->via.back( ).second.c_str( );
            myIP = inet_ntoa(d->via.back( ).first);
        }
        else {
            myHost = d->host;
            myIP = d->realip;
        }
        
        if (is_websock(d))
            client = "MudJS";
        else if (d->telnet.ttype != TTYPE_NONE)
            client = ttype_name(d->telnet.ttype);
        else
            client = "Telnet";

        if (d->character && d->character->is_npc())
            extraInfo = "switched";
        else if (uses_screenreader(d))
            extraInfo = "sreader";
        
        ch->pecho( "[%3d %10s %-5s %3s %6s] %-12s %-15s %s",
                        d->descriptor,
                        state,
                        logon.c_str(),
                        idle.c_str(),
                        client.c_str(),
                        name.c_str( ),
                        (!*myHost) ? myIP : myHost,
                        extraInfo.c_str());
    }

    ch->pecho( "\n\r%d user%s", count, count == 1 ? "" : "s" );
}

