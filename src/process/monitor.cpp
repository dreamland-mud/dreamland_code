/* $Id: monitor.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include <string.h>
#include "logstream.h"

#include "monitor.h"

Monitor::Monitor(Mutex *mx) : mux(mx)
{
    pthread_cond_init(&cnd, 0);
}

Monitor::~Monitor()
{
    pthread_cond_destroy(&cnd);
}

void
Monitor::wait()
{
    int rc;
    if((rc=pthread_cond_wait(&cnd, &mux->mux)))
        LogStream::sendError( ) << "pthread_cond_wait failed! " << strerror(rc) << endl;
}

#if notyet
void
Monitor::wait(int t)
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (ts.tv_nsec + t*1000) / 1000000000;
    ts.tv_nsec += (ts.tv_nsec + t*1000) % 1000000000;
    
    pthread_cond_timedwait(&cnd, &mux->mux, &ts);
}
#endif

void
Monitor::notify()
{
    pthread_cond_signal(&cnd);
}

void
Monitor::notifyAll()
{
    pthread_cond_broadcast(&cnd);
}

