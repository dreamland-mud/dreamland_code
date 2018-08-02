/* $Id: monitor.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "mutex.h"

class Monitor : public virtual DLObject
{
public:
    Monitor(Mutex *mux);
    virtual ~Monitor();

    void wait();

    void notify();
    void notifyAll();
    
#ifndef __MINGW32__
    pthread_cond_t cnd;
#else
    HANDLE evt;
#endif
    Mutex *mux;
};

#endif
