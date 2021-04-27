
#ifndef __SERVLET_H__
#define __SERVLET_H__

#include "plugin.h"
#include "plugininitializer.h"
#include "oneallocate.h"
#include "http.h"

class Servlet : public virtual DLObject {
public:
    typedef ::Pointer<Servlet> Pointer;

    virtual ~Servlet();

    virtual void handle(HttpRequest &req, HttpResponse &rsp) = 0;
};

class ServletPlugin : public Plugin, public Servlet {
public:
    virtual ~ServletPlugin();

    virtual DLString getPrefix() = 0;

protected:
    virtual void initialization( );
    virtual void destruction( );
};

class ServletManager : public OneAllocate {
public:
    ServletManager();
    virtual ~ServletManager();

    void handle(HttpRequest &req, HttpResponse &rsp);

    void open(int port);
    void add(const DLString &prefix, Servlet::Pointer p);
    void remove(const DLString &prefix);

    static ServletManager *getThis() { return instance; }
private:
    typedef std::map<DLString, Servlet::Pointer> PrefixMap;
    PrefixMap servlets;

    static ServletManager *instance;
};

template <const char *&tn>
class ServletTemplate : public ServletPlugin { //TODO ClassSelfRegistratorPlugin<tn>? We may want to register the class later
public:

    virtual void handle(HttpRequest &req, HttpResponse &rsp);
    virtual DLString getPrefix() { return prefix; }
private:
    static const char *prefix;
};

#define INITPRIO_SERVLETS 52

#define SERVLET_DUMMY(x) dummyServlet_ ##x## _TypeName
#define SERVLET(x) ServletTemplate<SERVLET_DUMMY(x)>

#define SERVLET_DECL(x, p) \
const char *SERVLET_DUMMY(x) = "SERVLET(" #x ")"; \
template<> const char *SERVLET(x)::prefix = p; \
PluginInitializer<SERVLET(x)> dummyServlet_ ##x## _init(INITPRIO_SERVLETS);

#define SERVLET_HANDLE(x, p) \
SERVLET_DECL(x, p) \
template<> void SERVLET(x)::handle(HttpRequest &request, HttpResponse &response)


#endif
