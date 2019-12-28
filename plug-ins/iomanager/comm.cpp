/* $Id$
 *
 * ruffina, 2004
 */

#include <errno.h>
#include <fcntl.h>

#ifndef __MINGW32__
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <winsock.h>

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#endif

#include "serversocketcontainer.h"
#include "wrapperhandler.h"
#include "nannyhandler.h"
#include "defaultbufferhandler.h"
#include "interprethandler.h"
#include "backdoorhandler.h"
#include "pagerhandler.h"
#include "codepage.h"
#include "comm.h"

#include "logstream.h"
#include "char.h"
#include "dreamland.h"
#include "ban.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "objectbehavior.h"
#include "room.h"
#include "helpmanager.h"
#include "object.h"
#include "mercdb.h"
#include "loadsave.h"
#include "interp.h"
#include "telnet.h"
#include "mudtags.h"
#include "wiznet.h"
#include "act.h"
#include "vnum.h"
#include "def.h"


const unsigned char eor_on_str[] = { IAC, WILL, TELOPT_EOR };
#ifdef MCCP
const unsigned char compress_on_str[] = { IAC, WILL, TELOPT_COMPRESS };
const unsigned char compress2_on_str[] = { IAC, WILL, TELOPT_COMPRESS2 };
#endif
const unsigned char via_qry_str[]  = { IAC, WILL, TELOPT_VIA };
const unsigned char ttype_do_str[] = { IAC, DO, TELOPT_TTYPE };
const unsigned char gmcp_on_str[] = { IAC, WILL, GMCP };

bool process_output( Descriptor *d, bool fPrompt )
{
    /*
     * Bust a prompt.
     */

    if (!dreamland->isShutdown( ) && fPrompt)
        if(!d->handle_input.empty( ) && d->handle_input.front( ))
            d->handle_input.front( )->prompt(d);

    /*
     * Short-circuit if nothing to write.
     */

    if (!d->outtop)
        return true;

    /*
     * OS-dependent output.
     */
    bool rc = d->writeRaw((const unsigned char*)d->outbuf, d->outtop);
    
    d->outtop = 0;

    return rc;
}

void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    Descriptor *dnew;
    struct sockaddr_in sock;
    int desc, x;
#ifdef __MINGW32__
    int size;
#else
    socklen_t size;
#endif

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    desc = accept( control, (struct sockaddr *) &sock, &size);
    if ( desc < 0 ) {
        LogStream::sendError( ) <<"New_descriptor: accept::" << strerror( errno ) << endl;
        return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

#ifndef __MINGW32__
    if ( fcntl( desc, F_SETFL, FNDELAY ) < 0 ) {
        LogStream::sendError( ) << "New_descriptor: fcntl: FNDELAY::" << strerror( errno ) << endl;
        return;
    }
    
    x = 1;
    if(setsockopt(desc, SOL_SOCKET, SO_KEEPALIVE, &x, sizeof(x)) < 0) {
        LogStream::sendError( ) << "setsockopt: SO_KEEPALIVE" << endl;
        close( desc );
        return;
    }
#endif

    if (!ServerSocketContainer::isAllowed(control, sock)) {
        close( desc );
        return;
    }
    
    /*
     * Cons a new descriptor.
     */
    dnew = new Descriptor;

    dnew->control        = control;
    dnew->descriptor        = desc;
    dnew->connected        = CON_CODEPAGE;
    dnew->outsize        = 2000;
    dnew->outbuf        = ( char* )malloc( dnew->outsize );
    dnew->buffer_handler= new DefaultBufferHandler;
    
    dnew->inptr = 0;
    dnew->telnet.sn_ptr = 0;
    dnew->telnet.state = TNS_NORMAL;
    dnew->telnet.ttype = 0;
    dnew->oob_proto = 0;

    const char *realip = inet_ntoa(sock.sin_addr);
    dnew->realip        = str_dup(realip); 
    dnew->host          = str_dup(realip); 
    LogStream::sendNotice( ) << "New descriptor " << dnew->realip << endl;

    if (banManager->checkVerbose( dnew, BAN_ALL )) {
        LogStream::sendWarning( ) << "Closing banned descriptor  " << buf << endl;
        close( desc );
        delete dnew;
        return;
    }
    
    /*
     * Init descriptor data.
     */
    dnew->next                        = descriptor_list;
    descriptor_list                = dnew;

    if (ServerSocketContainer::isWrapped( control )) 
        WrapperHandler::init( dnew );
    else if (ServerSocketContainer::isBackdoor( control ))
        BackdoorHandler::init( dnew );
    else if (ServerSocketContainer::isWebSock( control ))
        dnew->websock.state = WS_NEGOTIATING;
    else
        NannyHandler::init( dnew );

    if(dnew->websock.state != WS_NEGOTIATING) {
        dnew->writeFd(eor_on_str, sizeof(eor_on_str));
#ifdef MCCP
        dnew->writeFd(compress2_on_str, sizeof(compress2_on_str));
        dnew->writeFd(compress_on_str, sizeof(compress_on_str));
#endif
        dnew->writeFd(via_qry_str, sizeof(via_qry_str));
        dnew->writeFd(ttype_do_str, sizeof(ttype_do_str));
        dnew->writeFd(gmcp_on_str, sizeof(gmcp_on_str));
    }
}


void page_to_char( const char *txt, Character *ch )
{
    ostringstream out;
    Descriptor *d = ch->desc; 

    if (!d)
        return;

    if (!txt || !*txt)
        return;
    
    mudtags_convert( txt, out, ch );
    
    d->handle_input.push_front(new PagerHandler(out.str( ).c_str( )));
    d->handle_input.front()->handle(d, str_empty);
}

char *get_multi_command(Descriptor *d, char *argument)
{
    int counter;
    static char command[MAX_STRING_LENGTH];

    command[0] = '\0';

    for(counter = 0; argument[counter]; counter++) {
        if(argument[counter] == '|') {
            if(argument[counter + 1] != '|') {
                command[counter] = '\0';
                counter++;
                memmove(d->incomm, argument + counter, strlen(argument + counter) + 1);
                return(command);
            } else {
                memmove(argument + counter, argument + counter + 1, strlen(argument + counter));
            }
        }

        command[counter] = argument[counter];
    }

    d->incomm[0] = '\0';
    command[counter] = '\0';

    return(command);
}

/*
 * find a descriptor with given name
 */
Descriptor * descriptor_find_named( Descriptor *myD, const DLString &myName, int state )
{
    Descriptor *d;
    DLString name;

    for (d = descriptor_list; d; d = d->next) {
        if (d == myD)
            continue;
        
        if (!d->character)
            continue;
        
        if (state != -1 && d->connected != state)
            continue;

        name = d->character->getPC( )->getName( );

        if (name ^ myName)
            return d;
    } 

    return NULL;
}

/*
 *  write help article 
 */
void do_help( Character *ch, const char *topic )
{
    do_help( ch->desc, topic, ch->getConfig( )->color );
}

void do_help( Descriptor *d, const char *topic, bool fColor )
{
    HelpArticles::const_iterator a;
    Character *ch = d->character;

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
        if (is_name( topic, (*a)->getAllKeywordsString( ).c_str( ) )) {
            ostringstream buf;

            if (!fColor)
                mudtags_convert_nocolor( (*a)->getText( ch ).c_str( ), buf, ch );
            else
                mudtags_convert( (*a)->getText( ch ).c_str( ), buf, ch );

            d->send( buf.str( ).c_str( ));
            return;
        }
    }

    LogStream::sendError( ) << "nanny: no help for '" << topic << "'" << endl;
}

/** Generate random alnum string of given length. */
string create_nonce(int len)
{
    ostringstream buf;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        buf << alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return buf.str();
}

