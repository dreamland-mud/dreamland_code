/* $Id$
 *
 * ruffina, 2004
 */
// serversocket.h: interface for the ServerSocket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#ifndef __MINGW32__
#include <sys/socket.h>
#endif

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"

#include "exceptionserversocket.h"
#include "dlxmlloader.h"


class ServerSocket : public XMLVariableContainer, public XMLTableElement
{
XML_OBJECT;
public:        
        typedef ::Pointer<ServerSocket> Pointer;
        
public:        
        static const int DEFAULT_LISTEN = 5;

public:
        ServerSocket( );
        virtual ~ServerSocket( );

        virtual const DLString & getName( ) const;
        virtual void setName( const DLString & );

        inline int getPort( ) const
        {
                return port.getValue( );
        }

        inline int getListen( ) const
        {
                return listen.getValue( );
        }

        inline int getWrapped( ) const
        {
                return wrapped.getValue( );
        }
        
        inline int getFD( ) const
        {
                return fd;
        }

        inline bool isLocal( ) const
        {
            return local.getValue( );
        }

        inline bool isBackdoor( ) const
        {
            return backdoor.getValue( );
        }

        inline bool isWebSock( ) const
        {
            return websock.getValue( );
        }

        inline bool isInitialized( ) const
        {
            return fd != -1;
        }

        inline const char * getAllowedIP( ) const
        {
                return allowedIP.getValue( ).c_str( );
        }

        void initialize( ) throw ( ExceptionServerSocket );
        void destroy( );
        
        virtual void loaded( );
        virtual void unloaded( );

private:
        DLString name;

        XML_VARIABLE XMLInteger        port;
        XML_VARIABLE XMLInteger        listen;
        XML_VARIABLE XMLInteger wrapped;
        XML_VARIABLE XMLBoolean local;
        XML_VARIABLE XMLString  allowedIP;
        XML_VARIABLE XMLBoolean backdoor;
        XML_VARIABLE XMLBoolean websock;
            
        int fd;
};


#endif
