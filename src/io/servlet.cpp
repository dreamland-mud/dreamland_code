
#include "servlet.h"
#include "httpsocket.h"
#include "logstream.h"

ServletManager *ServletManager::instance = NULL;

Servlet::~Servlet()
{
}


ServletPlugin::~ServletPlugin()
{
}

void
ServletPlugin::initialization()
{
    ServletManager::getThis()->add(getPrefix(), (Servlet*)this);
}

void
ServletPlugin::destruction()
{
    ServletManager::getThis()->remove(getPrefix());
}


ServletManager::ServletManager()
{
    checkDuplicate(instance);
    instance = this;
}

void ServletManager::open(int port)
{
    (new HttpServerSocket(port))->put();
}


ServletManager::~ServletManager()
{
    instance = NULL;
}


void 
ServletManager::handle(HttpRequest &req, HttpResponse &rsp)
{
    for(PrefixMap::iterator it=servlets.begin();it!=servlets.end();it++) {
        if(req.uri.size() >= it->first.size() && req.uri.substr(0, it->first.size()) == it->first) {
            // Last-resort boundary: no servlet exception may ever escape to std::terminate
            // and take the whole server down over one HTTP request. Turn it into a 500.
            try {
                it->second->handle(req, rsp);
            } catch (const std::exception &e) {
                LogStream::sendError() << "Servlet " << req.method << " " << req.uri
                                       << " threw: " << e.what() << endl;
                rsp.status = 500;
                rsp.message = "Command failed";
                rsp.body = e.what();
            }
            break;
        }
    }
}

void 
ServletManager::add(const DLString &prefix, Servlet::Pointer p)
{
    servlets[prefix] = p;
}

void 
ServletManager::remove(const DLString &prefix)
{
    servlets.erase(prefix);
}
