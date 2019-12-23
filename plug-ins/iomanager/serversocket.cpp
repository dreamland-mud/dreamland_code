/* $Id$
 *
 * ruffina, 2004
 */
// serversocket.cpp: implementation of the ServerSocket class.
//
//////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstring>

#include <unistd.h>

#ifndef __MINGW32__
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#include "logstream.h"

#include "class.h"
#include "serversocket.h"
#include "serversocketcontainer.h"
#include "exceptionserversocket.h"

ServerSocket::ServerSocket( )
        : listen( DEFAULT_LISTEN ),
                wrapped( false ),
                local( false ),
                backdoor( false ),
                websock( false ),
                fd( -1 )
{
}

ServerSocket::~ServerSocket( )
{
    destroy( );
}

void ServerSocket::initialize( ) 
{
    sockaddr_in sa;
    int x = 1;

    LogStream::sendNotice( ) << "port::trying::" << getPort( ) << endl;
    
    if( ( fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 ) {
        fd = -1;
        throw ExceptionServerSocket( "socket", strerror( errno ), getPort( ), getListen( ) );
    }

#ifndef __MINGW32__
    if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof( x ) ) < 0 ) {
        destroy( );
        throw ExceptionServerSocket( "SO_REUSEADDR", strerror( errno ), getPort( ), getListen( ) );
    }
#endif

    memset( &sa, 0, sizeof( sa ) );
    sa.sin_family = AF_INET;
    sa.sin_port = htons( getPort( ) );

    if(bind(fd, reinterpret_cast<struct sockaddr*>(&sa ), sizeof(sa) ) < 0 ) {
        destroy( );
        throw ExceptionServerSocket( "bind", strerror( errno ), getPort( ), getListen( ) );
    }

    if( ::listen( fd, getListen( ) ) < 0 ) {
        destroy( );
        throw ExceptionServerSocket( "listen", strerror( errno ), getPort( ), getListen( ) );
    }
}

void ServerSocket::destroy( )
{
    if(fd >= 0) {
#ifndef __MINGW32__
        close(fd);
#else
        closesocket(fd);
#endif
    }
    fd = -1;
}

void ServerSocket::loaded( )
{
    try {
        initialize( );
    } catch( const ExceptionServerSocket& ex ) {
        LogStream::sendError( ) << ex << endl;
    }
}

void ServerSocket::unloaded( )
{
    destroy( );
}

const DLString & ServerSocket::getName( ) const
{
    return name;
}

void ServerSocket::setName( const DLString &name )
{
    this->name = name;
    port = name.toInt( );
}

