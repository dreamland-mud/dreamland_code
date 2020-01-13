#ifndef JSONSERVLET_H
#define JSONSERVLET_H

#include "servlet.h"

namespace Json {
    class Value;
}

class JsonServletBase {
public:
    virtual void jsonBody(const Json::Value &params, Json::Value &body) = 0;
    void handleRequest(HttpRequest &req, HttpResponse &rsp);
};

template <const char *&tn>
class JsonServletTemplate : public ServletPlugin, public JsonServletBase {
public:
    
    virtual void handle(HttpRequest &req, HttpResponse &rsp)
    {
        handleRequest(req, rsp);
    }    

    virtual void jsonBody(const Json::Value &params, Json::Value &body);
    virtual DLString getPrefix() { return prefix; }

private:
    static const char *prefix;
};

// JSON Servlet macros
#define JSONSERVLET(x) JsonServletTemplate<SERVLET_DUMMY(x)>

#define JSONSERVLET_DECL(x, p) \
const char *SERVLET_DUMMY(x) = "JSONSERVLET(" #x ")"; \
template<> const char *JSONSERVLET(x)::prefix = p; \
PluginInitializer<JSONSERVLET(x)> dummyServlet_ ##x## _init(INITPRIO_SERVLETS);

#define JSONSERVLET_HANDLE(x, p) \
JSONSERVLET_DECL(x, p) \
template<> void JSONSERVLET(x)::jsonBody(const Json::Value &params, Json::Value &body)


#endif