
#ifndef __SOCKETMANAGER_H__
#define __SOCKETMANAGER_H__

#include <list>
#include <vector>
#include <algorithm>
#include <functional>

#include "oneallocate.h"

struct timeval;

class SocketTask : public virtual DLObject
{
    friend class SocketManager;
public:
    SocketTask(int descriptor);
    virtual ~SocketTask();

    typedef ::Pointer<SocketTask> Pointer;

    virtual bool expectRead() const = 0;
    virtual bool expectWrite() const = 0;
    virtual void handleRead() = 0;
    virtual void handleWrite() = 0;

protected:
    struct pollfd getPfd() const;

    int fd;
};

class ServerSocketTask : public SocketTask
{
public:
    ServerSocketTask(int);

    virtual bool expectRead() const;
    virtual bool expectWrite() const;
    
    virtual void handleRead();
    virtual void handleWrite();

protected:
    virtual void handleConnection(int fd) = 0;
    virtual void handleError(int) = 0;
};

class BufferedSocketTask : public SocketTask
{
public:
    BufferedSocketTask(int);

    virtual bool expectRead() const;
    virtual bool expectWrite() const;
    
    virtual void handleRead();
    virtual void handleWrite();

    virtual void handleIn() = 0;
    virtual void handleEOF() = 0;
    virtual void handleError(int) = 0;
protected:
    std::vector<unsigned char> in, out;
};

class SocketManager : std::list<SocketTask::Pointer>, public OneAllocate 
{
public:
    SocketManager();
    virtual ~SocketManager();

    void run(int ms);
    void put(SocketTask::Pointer task);
    void slay(int fd);

private:
    static bool sameFd(SocketTask::Pointer task1, int fd);
};


#endif
