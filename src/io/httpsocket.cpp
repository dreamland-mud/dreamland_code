
#include <sstream>

#include "httpsocket.h"
#include "servlet.h"
#include "logstream.h"

using namespace std;

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

            handleRequest();

            response.headers["content-length"] = to_string(response.body.size());

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

    ServletManager::getThis()->handle(request, response);
}

HttpServerSocket::HttpServerSocket(unsigned short port) : ServerSocketTask(port) 
{
}

SocketTask::Pointer 
HttpServerSocket::createNewTask(int fd)
{
    LogStream::sendError() << "HttpServerSocket: Connected! fd=" << fd << endl;
    return (SocketTask*)new HttpSocketTask(fd);
}

