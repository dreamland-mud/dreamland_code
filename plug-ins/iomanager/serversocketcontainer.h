/* $Id$
 *
 * ruffina, 2004
 */
// serversocketcontainer.h: interface for the ServerDescriptorContainer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SERVERSOCKETCONTAINER_H
#define SERVERSOCKETCONTAINER_H

#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#include "oneallocate.h"
#include "dlxmlloader.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "serversocket.h"
#include "xmltableloaderplugin.h"

class Descriptor;

class ServerSocketContainer : 
            public OneAllocate, 
            public DLXMLTableLoader,
            public XMLTableLoaderPlugin
{
    friend class ServerSocket;

public:        
    typedef ::Pointer<ServerSocketContainer> Pointer;
    typedef LoadedList::iterator iterator;

public:
    ServerSocketContainer( );
    virtual ~ServerSocketContainer( );

    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;

    static bool isWrapped( int );
    static bool isBackdoor( int );
    static bool isWebSock( int );
    static bool isAllowed( int, struct sockaddr_in & );
    static bool isNewNanny(int desc);

    // temporary methods
    static void FD_SETBeforeSelect( fd_set* );
    static void checkNewConnaection( fd_set* );

    static inline ServerSocketContainer* getThis( ) {
        return thisClass;
    }

    /** Find socket description matching this descriptor number. */
    ServerSocket * findSocket(int desc);

private:
    static ServerSocketContainer* thisClass;
    static const DLString TABLE_NAME, NODE_NAME;
};


#endif
