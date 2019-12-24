#include "jsonservlet.h"
#include "json/json.h"
#include "iconvmap.h"

static IconvMap koi2utf("koi8-r", "utf-8");

void JsonServletBase::handleRequest(HttpRequest &request, HttpResponse &response)
{
    try {
        Json::Value params;
        if (request.method == "POST") {
            Json::Reader reader;
            if (!reader.parse(request.body, params))
                throw ::Exception("Cannot parse JSON body");
        }

        Json::Value body;
        jsonBody(params, body);

        Json::FastWriter writer;
        response.body = koi2utf(writer.write(body));
        
        response.status = 200;
        response.message = "OK";
        response.headers["content-type"] = "application/json";
    
    } catch (const ::Exception &e) {
        response.status = 500;
        response.message = "Command failed";
        response.body = e.what();
    }
}

