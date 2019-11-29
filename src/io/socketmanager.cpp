
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

ServerSocketTask::ServerSocketTask(int i) : SocketTask(i)
{
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

    handleConnection(d);   
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

HttpSocketTask::HttpSocketTask(int fd) : BufferedSocketTask(fd), state(INIT)
{
}

bool
HttpSocketTask::consumeLine(string &str)
{
    static const unsigned char CRLF[] = { '\r', '\n' };
    vector<unsigned char>::iterator it = search(in.begin(), in.end(), CRLF, CRLF+sizeof(CRLF));

    if(it == in.end())
        return false;

    str.assign(in.begin(), it);
    in.erase(in.begin(), it+2);

    return true;
}

void
HttpSocketTask::parse1stLine(const string &str)
{
    stringstream ss(str);
    getline(ss, request.method, ' ');
    getline(ss, request.uri, ' ');
    getline(ss, request.proto);
    request.headers.clear();
}

void
HttpSocketTask::parseHeaderLine(const string &str)
{
    stringstream ss(str);
    string name, value;

    getline(ss, name, ':');
    getline(ss, value);
    
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    string::iterator it = find_if(value.begin(), value.end(), [](unsigned char c) { return c != ' '; });
    
    if(it != value.end())
        value.erase(value.begin(), it);

    if(request.headers.find(name) != request.headers.end())
        value += ", ";
    
    request.headers[name] = value;
}

size_t
HttpSocketTask::getContentLength()
{
    const string &val = request.headers["content-length"];

    if(val.size() == 0)
        return 0;
    
    return atoi(val.c_str());
}

ostream &
operator << (ostream &os, const HttpResponse &response)
{
    os << response.proto << " " << response.status << " " << response.message << "\r\n";

    for(map<string, string>::const_iterator it=response.headers.begin();it!=response.headers.end();it++)
        os << it->first << ": " << it->second << "\r\n";
    
    os << "\r\n" << response.body;
    return os;
}

void
HttpSocketTask::handleIn()
{
    string line;

    switch(state) {
        case INIT:
            if(!consumeLine(line))
                break;

            parse1stLine(line);
            state = HEADER;
            /* fall through */
        case HEADER:
            for(;;) {
                if(!consumeLine(line))
                    return;

                if(line.size() == 0)
                    break;
                
                parseHeaderLine(line);
            }
            state = BODY;
            /* fall through */
        case BODY:
            if(in.size() < getContentLength())
                return;

            state = INIT;
            request.body.assign(in.begin(), in.begin()+getContentLength());
            in.erase(in.begin(), in.begin()+getContentLength());

            response.status = 404;
            response.message = "Not found";
            response.proto = request.proto;
            response.body = "Nothing's here, go away!\n";
            response.headers.clear();
            response.headers["content-type"] = "text/plain";
            response.headers["connection"] = "close";
            response.headers["content-length"] = to_string(response.body.size());

            handleRequest();

            stringstream ss;
            ss << response;
            ss.flush();
            string s = ss.str();
            out.assign(s.begin(), s.end());
    }
}

void
HttpSocketTask::handleRequest()
{
    LogStream::sendError() << "method=" << request.method << ", uri=" << request.uri << ", proto=" << request.proto << endl;

    for(map<string, string>::iterator it=request.headers.begin();it!=request.headers.end();it++)
        LogStream::sendError() << "  " << it->first << ": " << it->second << endl;

    LogStream::sendError() << "body='" << request.body << "'" << endl;
}

HttpServerSocket::HttpServerSocket(unsigned short port) : ServerSocketTask(createSocket(port)) 
{
    put();
}

void
HttpServerSocket::handleConnection(int fd)
{
    LogStream::sendError() << "HttpServerSocket: Connected! fd=" << fd << endl;
    (new HttpSocketTask(fd))->put();
}

int
HttpServerSocket::createSocket(unsigned short port)
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
