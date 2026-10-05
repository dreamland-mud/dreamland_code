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
#include "iomanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "merc.h"
#include "descriptor.h"
#include "screenreader.h"
#include "websocketrpc.h"
#include "act.h"

const char *ttype_name( int ttype );

/*
 * socket kick <num>   -- close one descriptor (not your own)
 * socket kick idle    -- close login screens silent past the idle timeout
 */
static void socket_kick( Character *ch, DLString args )
{
    IOManager *io = IOManager::getThis( );
    DLString what = args.getOneArgument( );

    if (what.empty( )) {
        ch->pecho( "Usage: socket kick <num> | socket kick idle" );
        return;
    }

    if (what == "idle") {
        int cnt = io->kickIdleLogins( IOManager::LOGIN_IDLE_TIMEOUT );
        ch->pecho( "%d idle login descriptor%s closed.", cnt, cnt == 1 ? "" : "s" );
        return;
    }

    if (!what.isNumber( )) {
        ch->pecho( "Usage: socket kick <num> | socket kick idle" );
        return;
    }

    int num = what.toInt( );
    for (Descriptor *d = descriptor_list; d; d = d->next) {
        if (d->descriptor != num || d->connected == CON_CLOSED)
            continue;

        if (d == ch->desc) {
            ch->pecho( "That is your own connection." );
            return;
        }

        if (d->character && d->character->get_trust( ) > ch->get_trust( )) {
            ch->pecho( "You can't." );
            return;
        }

        io->kickDescriptor( d );
        ch->pecho( "Descriptor %d closed.", num );
        return;
    }

    ch->pecho( "No open descriptor %d.", num );
}

CMDADM( socket )
{
    DLString arg = constArguments;
    DLString cmd = arg.getOneArgument( );

    if (cmd == "kick") {
        socket_kick( ch, arg );
        return;
    }

    PCMemoryInterface *pcm;
    Descriptor *d;
    DLString name;
    int             count;

    ch->pecho("\n\r[Num Connected  Login Idl Client] Player Name  Host            ");
    ch->pecho("--------------------------------------------------------------------------");
    count = 0;

    for (d = descriptor_list; d; d = d->next) {
        DLString myHost, myIP;
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
            // A login screen's idle is minutes since its last input, so a dead
            // line (see socket kick idle) shows up as a big number.
            logon = "-----";
            idle = fmt(0, "%3d", IOManager::getThis( )->idleSeconds( d ) / 60);
        }
        
        switch (d->connected) {
        case CON_PLAYING:   state = " PLAYING  ";        break;
        case CON_CODEPAGE:  state = " Codepage ";        break;
        case CON_NANNY:     state = "  Nanny   ";        break;
        default:            state = " UNKNOWN! ";        break;
        }
        
        count++;
        
        if (!d->via.empty( )) {
            myHost = d->via.back( ).second;
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
                        myHost.empty() ? myIP.c_str() : myHost.c_str(),
                        extraInfo.c_str());
    }

    ch->pecho( "\n\r%d user%s", count, count == 1 ? "" : "s" );
}

