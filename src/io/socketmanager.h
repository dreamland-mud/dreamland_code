
#ifndef __SOCKETMANAGER_H__
#define __SOCKETMANAGER_H__

#include <list>
#include <vector>
#include <map>
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
    virtual void handleError(string);
    void close();
    void put();

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
    virtual void handleEOF();
protected:
    std::vector<unsigned char> in, out;
};

class HttpEntity
{
public:
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpRequest : public HttpEntity
{
public:
    std::string method;
    std::string uri;
    std::string proto;
};

class HttpResponse : public HttpEntity
{
public:
    std::string proto;
    int status;
    std::string message;
};

class HttpSocketTask : public BufferedSocketTask
{
public:
    HttpSocketTask(int);

protected:
    virtual void handleIn();

    virtual void handleRequest();

    HttpRequest request;
    HttpResponse response;

private:
    size_t getContentLength();
    bool consumeLine(string &str);
    void parseHeaderLine(const string &str);
    void parse1stLine(const string &str);

    enum {
        INIT,
        HEADER,
        BODY
    } state;
};

class SocketManager : std::list<SocketTask::Pointer>, public OneAllocate 
{
public:
    SocketManager();
    virtual ~SocketManager();

    void run(int ms);
    void put(SocketTask::Pointer task);
    void slay(int fd);

    static SocketManager *getThis() { return instance; }

private:
    static bool sameFd(SocketTask::Pointer task1, int fd);
    static SocketManager *instance;
};

ostream &operator << (ostream &os, const HttpResponse &resp);

class HttpServerSocket : public ServerSocketTask {
public:
    HttpServerSocket(unsigned short port);

    int createSocket(unsigned short port);

protected:
    void handleConnection(int fd);
};

#endif
