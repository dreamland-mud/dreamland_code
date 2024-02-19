/* $Id: mutex.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include <errno.h>
#include <string.h>

#include "mutex.h"
#include "logstream.h"

Mutex::Mutex()
{
    int rc;
    if((rc=pthread_mutex_init(&mux, 0)))
        LogStream::sendError( ) << "pthread_mutex_init: failed! " << strerror(rc) << endl;

}

Mutex::~Mutex()
{
    int rc;
    if((rc=pthread_mutex_destroy(&mux)))
        LogStream::sendError( ) << "pthread_mutex_destroy: failed! " << strerror(rc) << endl;
}

void
Mutex::lock()
{
    int rc;
    if( (rc=pthread_mutex_lock(&mux)) )
        LogStream::sendError( ) << "pthread_mutex_lock: failed! "<< strerror(rc) << endl;
    
}

void
Mutex::unlock()
{
    int rc;
    if( (rc=pthread_mutex_unlock(&mux)) )
        LogStream::sendError( ) << "pthread_mutex_unlock: failed! " << strerror(rc) << endl;
        
}

