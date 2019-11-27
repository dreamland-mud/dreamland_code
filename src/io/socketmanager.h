
#ifndef __SOCKETMANAGER_H__
#define __SOCKETMANAGER_H__

#include <list>
#include <algorithm>
#include <functional>

#include "oneallocate.h"

struct timeval;

class SocketTask : public virtual DLObject
{
    friend class SocketManager;
public:
    SocketTask(int descriptor, int flags);

    typedef ::Pointer<SocketTask> Pointer;

    virtual void handleRead();
    virtual void handleWrite();
        
protected:
    int descriptor;
    int flags;
};

class SocketManager : std::list<SocketTask::Pointer>, public OneAllocate 
{
public:
    SocketManager();
    virtual ~SocketManager();

    void run(int ms);
    void put(SocketTask::Pointer task);
    void slay(SocketTask::Pointer task);
};


#endif
