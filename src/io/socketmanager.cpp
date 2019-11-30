
#include <sstream>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>

#include "exception.h"
#include "logstream.h"
#include "socketmanager.h"


using namespace std;

SocketManager *SocketManager::instance = NULL;

SocketManager::SocketManager()
{
    checkDuplicate(instance);
    instance = this;
}

SocketManager::~SocketManager()
{
    instance = NULL;
}

void 
SocketManager::put(SocketTask::Pointer task)
{
    slay(task);
    push_back(task);
}

bool 
SocketManager::sameFd(SocketTask::Pointer task1, int fd)
{
    return task1->fd == fd;
}

void 
SocketManager::slay(int fd)
{
    remove_if(bind2nd(ptr_fun(sameFd), fd));
}

void 
SocketManager::run(int ms)
{
    int sz = size();
    struct pollfd fds[sz];
    SocketTask::Pointer tasks[sz];

    copy(begin(), end(), tasks);
    transform(begin(), end(), fds, [](SocketTask::Pointer p) -> struct pollfd { return p->getPfd(); });

    int rc = poll(fds, sz, ms);

    if(rc == 0) {
        return;
    }

    if(rc < 0) {
        LogStream::sendError() << "poll: " << strerror(errno) << endl;
        return;
    }
    
    for(int i=0;i<sz;i++) {
        if(fds[i].revents & POLLIN)
            tasks[i]->handleRead();
        
        if(fds[i].revents & POLLOUT)
            tasks[i]->handleWrite();
    }
}

SocketTask::SocketTask(int i) : fd(i)
{
}

SocketTask::~SocketTask()
{
}

void
SocketTask::close()
{
    SocketManager::getThis()->slay(fd);
    ::close(fd);
}

void
SocketTask::put()
{
    SocketManager::getThis()->put(this);
}

void
SocketTask::handleError(string msg)
{
    LogStream::sendError() << "SocketTask: error=" << msg << endl;
    close();
}

struct pollfd
SocketTask::getPfd() const
{
    struct pollfd rc;
    
    rc.fd = fd;
    rc.events = 0;
    rc.revents = 0;

    if(expectRead())
        rc.events |= POLLIN;

    if(expectWrite())
        rc.events |= POLLOUT;

    return rc;
}

BufferedSocketTask::BufferedSocketTask(int fd) : SocketTask(fd)
{
}

bool
BufferedSocketTask::expectRead() const
{
    return true;
}

void
BufferedSocketTask::handleRead()
{
    unsigned char buf[1024];
    int rc = read(fd, buf, 1024);

    if(rc < 0) {
        handleError(strerror(errno));
        return;
    }

    if(rc == 0) {
        handleEOF();
        return;
    }

    in.reserve(in.size() + rc);
    in.insert(in.end(), buf, buf+rc);
    handleIn();
}

void
BufferedSocketTask::handleEOF()
{
    LogStream::sendError() << "BufferedSocketTask: EOF" << endl;
    close();
}

bool
BufferedSocketTask::expectWrite() const
{
    return out.size() > 0;
}

void
BufferedSocketTask::handleWrite()
{
    int rc = write(fd, &out[0], out.size());

    if(rc < 0) {
        handleError(strerror(errno));
        return;
    }

    out.erase(out.begin(), out.begin()+rc);
}

ServerSocketTask::ServerSocketTask(unsigned short port) : SocketTask(createSocket(port))
{
}

int
ServerSocketTask::createSocket(unsigned short port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 6);
    struct sockaddr_in sin = { AF_INET };
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    if(bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        ostringstream os;
        os << "HttpServerSocket: Oops: " << strerror(errno);
        throw Exception(os.str());
    }

    if(listen(sock, 7) < 0) {
        ostringstream os;
        os << "HttpServerSocket: Oops: " << strerror(errno);
        throw Exception(os.str());
    }

    return sock;
}

bool
ServerSocketTask::expectRead() const
{
    return true;
}

void
ServerSocketTask::handleRead()
{
    struct sockaddr_in sin;
    socklen_t sin_size = sizeof(sin);
    int d = accept(fd, (struct sockaddr *)&sin, &sin_size);

    if(d < 0) {
        handleError(strerror(errno));
        return;
    }

    createNewTask(d)->put();
}

bool
ServerSocketTask::expectWrite() const
{
    return false;
}

void
ServerSocketTask::handleWrite()
{
    // noop
}

