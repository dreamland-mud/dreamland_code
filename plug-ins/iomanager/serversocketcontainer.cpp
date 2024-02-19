/* $Id$
 *
 * ruffina, 2004
 */
// serversocketcontainer.cpp: implementation of the ServerSocketContainer class.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <sstream>
#include <arpa/inet.h>
#include <fcntl.h>

#include "class.h"
#include "serversocketcontainer.h"
#include "serversocket.h"
#include "dreamland.h"
#include "xmlinteger.h"
#include "logstream.h"

void init_descriptor( int );

ServerSocketContainer* ServerSocketContainer::thisClass = 0;
const DLString ServerSocketContainer::TABLE_NAME = "ports";
const DLString ServerSocketContainer::NODE_NAME = "ServerSocket";

ServerSocketContainer::ServerSocketContainer( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

ServerSocketContainer::~ServerSocketContainer( )
{
    thisClass = 0;
}

DLString ServerSocketContainer::getTableName( ) const 
{
    return TABLE_NAME;
}

DLString ServerSocketContainer::getNodeName( ) const 
{
    return NODE_NAME;
}

ServerSocket * ServerSocketContainer::findSocket(int desc)
{
    for (auto &elem: elements) {
        ServerSocket *sock = elem.getStaticPointer<ServerSocket>();
        if (sock && sock->getFD() == desc)
            return sock;
    }

    return 0;
}

bool ServerSocketContainer::isWrapped( int d )
{
    ServerSocket *sock = thisClass->findSocket(d);
    return sock && (sock->getWrapped( ) > 0);
}

bool ServerSocketContainer::isBackdoor( int d )
{
    ServerSocket *sock = thisClass->findSocket(d);
    return sock && sock->isBackdoor();
}

bool ServerSocketContainer::isWebSock( int d )
{
    ServerSocket *sock = thisClass->findSocket(d);
    return sock && sock->isWebSock();
}

bool ServerSocketContainer::isNewNanny(int desc)
{
    ServerSocket *sock = thisClass->findSocket(desc);
    return sock && sock->isNewNanny();
}

bool ServerSocketContainer::isAllowed( int d, struct sockaddr_in &other_sock )
{
    ServerSocket *sock = thisClass->findSocket(d);

    if (sock) {
        if (!sock->isLocal( ))
            return true;

        if (other_sock.sin_addr.s_addr == inet_addr(sock->getAllowedIP( )))
            return true;

        return false;
    }

    return false;
}

void ServerSocketContainer::FD_SETBeforeSelect( fd_set* in_set )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );

        if (sock->isInitialized( ))
            FD_SET( sock->getFD( ), in_set );
    }
}

void ServerSocketContainer::checkNewConnaection( fd_set* in_set )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );

        if (sock->isInitialized( ) && FD_ISSET( sock->getFD( ), in_set ))
            init_descriptor( sock->getFD( ) );
    }
}
