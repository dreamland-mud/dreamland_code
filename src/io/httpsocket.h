
#ifndef __HTTPSOCKET_H__
#define __HTTPSOCKET_H__

#include "socketmanager.h"
#include "http.h"

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
    bool consumeLine(std::string &str);
    void parseHeaderLine(const std::string &str);
    void parse1stLine(const std::string &str);

    enum {
        INIT,
        HEADER,
        BODY
    } state;
};

class HttpServerSocket : public ServerSocketTask {
public:
    HttpServerSocket(unsigned short port);

protected:
    virtual Pointer createNewTask(int fd);
};

#endif
