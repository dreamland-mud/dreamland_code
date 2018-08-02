/* $Id: monitor.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include <string.h>
#include "logstream.h"

#include "monitor.h"

Monitor::Monitor(Mutex *mx) : mux(mx)
{
#ifndef __MINGW32__
    pthread_cond_init(&cnd, 0);
#else
    evt = CreateEvent( 
        NULL,         // default security attributes
        FALSE,        // manual-reset event
        FALSE,        // initial state is signaled
        NULL          // object name
        ); 
#endif
}

Monitor::~Monitor()
{
#ifndef __MINGW32__
    pthread_cond_destroy(&cnd);
#else
    CloseHandle(evt);
#endif
}

void
Monitor::wait()
{
#ifndef __MINGW32__
    int rc;
    if((rc=pthread_cond_wait(&cnd, &mux->mux)))
	LogStream::sendError( ) << "pthread_cond_wait failed! " << strerror(rc) << endl;
#else
    /*XXX - this is a bit dangerous*/
    mux->unlock( );
    WaitForSingleObject(evt, INFINITE);
    mux->lock( );
#endif
}

#if notyet
void
Monitor::wait(int t)
{
#ifndef __MINGW32__
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (ts.tv_nsec + t*1000) / 1000000000;
    ts.tv_nsec += (ts.tv_nsec + t*1000) % 1000000000;
    
    pthread_cond_timedwait(&cnd, &mux->mux, &ts);
#else
    WaitForSingleObject(evt, t);
#endif
}
#endif

void
Monitor::notify()
{
#ifndef __MINGW32__
    pthread_cond_signal(&cnd);
#else
    SetEvent(evt);
#endif
}

void
Monitor::notifyAll()
{
#ifndef __MINGW32__
    pthread_cond_broadcast(&cnd);
#else
    while(SetEvent(evt))
	;
#endif
}

