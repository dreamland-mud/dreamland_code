#include <jsoncpp/json/json.h>
#include "servlet.h"
#include "servlet_utils.h"
#include "codesourcerepo.h"
#include "logstream.h"

/**
 * Servlet for /eval API endpoint.
 * Reads and evaluates a Fenia codesource file by path, similar to 'cs load'.
 * Auth: bottype=telegram, token=<secret token>
 * Args: path (file path relative to fenia script dir)
 */
static void eval_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    DLString path;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response))
        return;

    if (!servlet_get_arg(params, response, "path", path))
        return;

    if (CodeSourceRepo::getThis() == 0) {
        response.status = 500;
        response.message = "Internal error";
        response.body = "CodeSourceRepo is not available";
        return;
    }

    bool result = CodeSourceRepo::getThis()->read(path);

    if (result) {
        servlet_response_200(response, "Evaluated " + path);
    } else {
        response.status = 500;
        response.message = "Eval failed";
        response.body = "Failed to evaluate " + path;
    }
}

SERVLET_HANDLE(cmd_eval, "/eval")
{
    eval_servlet(request, response);
}
