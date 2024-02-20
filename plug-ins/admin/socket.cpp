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

CMDADM( socket )
{
    DLString arg = constArguments;
    PCMemoryInterface *pcm;
    Descriptor *d;
    DLString name;
    int             count;
    bool            ip;

    ip = (arg == "ip");

    if (!ip && !arg.empty())
        for (d = descriptor_list; d; d = d->next) {
            if (d->character) {
                name = d->character->getName( );
                pcm = PCharacterManager::find( name );
                
                if (pcm) {
                    if (!ch->can_see( d->character ))
                        continue;

                    // hide wizard login attempt
                    if (pcm->get_trust( ) > ch->get_trust( ))
                        continue;
                }
            }
            else {
                name = "Unknown";
            }
            
            if(arg.strPrefix(name) || arg == DLString(d->descriptor)) {
                ch->pecho( "Connected from: %s(%s)", d->realip, d->host );

                if(d->via.empty())
                    ch->pecho("No via records for this descriptor.");
                else {
                    ViaVector::iterator it;

                    for(it = d->via.begin(); it != d->via.end(); it++)
                        ch->pecho("Via: %s(%s)", 
                                it->second.c_str(), 
                                inet_ntoa(it->first));

                }
                return;
            }
        }

    ch->pecho("\n\r[Num Connected  Login Idl C P] Player Name  Host            ");
    ch->pecho("--------------------------------------------------------------------------");
    count = 0;

    for (d = descriptor_list; d; d = d->next) {
        const char *myHost, *myIP;
        DLString p;
        const char * state;
        char logon[100];
        char idle[10];

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
            strncpy( logon, d->character->getPC( )->age.getLogon( ).getTimeAsString( "%H:%M" ).c_str( ), 100 );
            sprintf( idle, "%3d", d->character->timer );
        }
        else {
            sprintf( logon, "-----" );
            sprintf( idle, "   " );
        }
        
        switch (d->connected) {
        case CON_PLAYING:   state = " PLAYING  ";        break;
        case CON_CODEPAGE:  state = " Codepage ";        break;
        case CON_NANNY:            state = "  Nanny   ";        break;
        default:            state = " UNKNOWN! ";         break;
        }
        
        count++;
        
        if (!d->via.empty( )) {
            myHost = d->via.back( ).second.c_str( );
            myIP = inet_ntoa(d->via.back( ).first);
            p = d->via.size( );
        }
        else {
            myHost = d->host;
            myIP = d->realip;
            p = (ServerSocketContainer::isWrapped(d->control) ? "*" : " "); 
        }
        
        ch->pecho( "[%3d %10s %-5s %s %c %s] %-12s %-15s %s",
                        d->descriptor,
                        state,
                        logon,
                        idle,
                        d->out_compress ? '*' : ' ',
                        p.c_str( ),
                        name.c_str( ),
                        (ip || !*myHost) ? myIP : myHost,
                        (d->character && d->character->is_npc() ? "switched" : "") );
    }

    ch->pecho( "\n\r%d user%s", count, count == 1 ? "" : "s" );
}

