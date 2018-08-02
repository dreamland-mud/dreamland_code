/* $Id: mutex.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifndef __MINGW32__
#include <pthread.h>
#else
#include <windows.h>
#endif


#include "dlobject.h"

class Mutex : public virtual DLObject
{
friend class Monitor;
public:
    Mutex();
    virtual ~Mutex();
    
    void lock();
    void unlock();

private:
#ifndef __MINGW32__
    pthread_mutex_t mux;
#else
    HANDLE mux;
#endif
};

#endif /* __MUTEX_H__ */

