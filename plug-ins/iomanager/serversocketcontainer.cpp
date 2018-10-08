/* $Id$
 *
 * ruffina, 2004
 */
// serversocketcontainer.cpp: implementation of the ServerSocketContainer class.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <sstream>
#ifndef __MINGW32__
#include <arpa/inet.h>
#endif
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

bool ServerSocketContainer::isWrapped( int d )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );

        if (sock->getFD( ) == d)
            return (sock->getWrapped( ) > 0);
    }

    return false;
}

bool ServerSocketContainer::isBackdoor( int d )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );

        if (sock->getFD( ) == d)
            return sock->isBackdoor( );
    }

    return false;
}

bool ServerSocketContainer::isWebSock( int d )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );

        if (sock->getFD( ) == d)
            return sock->isWebSock( );
    }

    return false;
}

bool ServerSocketContainer::isAllowed( int d, struct sockaddr_in &other_sock )
{
    iterator pos;

    for (pos = thisClass->elements.begin( ); pos != thisClass->elements.end( ); pos++) {
        ServerSocket *sock = pos->getStaticPointer<ServerSocket>( );
        
        if (sock->getFD( ) == d) {
           if (!sock->isLocal( ))
               return true;

           if (other_sock.sin_addr.s_addr == inet_addr(sock->getAllowedIP( )))
               return true;

           return false;
        }
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
