
#ifndef __HTTP_H__
#define __HTTP_H__

#include <map>
#include <string>
#include <ostream>

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

std::ostream &operator << (std::ostream &os, const HttpResponse &resp);


#endif
