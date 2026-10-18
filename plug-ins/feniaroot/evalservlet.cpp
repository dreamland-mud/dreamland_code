#include <jsoncpp/json/json.h>
#include "servlet.h"
#include "servlet_utils.h"
#include "codesourcerepo.h"
#include "codesource.h"
#include "fenia/register-impl.h"
#include "json_utils_ext.h"
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

    if (!servlet_auth_ops(params, response))
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

/**
 * Servlet for /fenia API endpoint.
 * Evaluates ONE inline Fenia expression and returns its value as JSON, so the
 * tooling reads live state with nobody online. Same path as the in-game 'eval'
 * command: a throwaway CodeSource that refcount-collects unless the expression
 * stores a function somewhere persistent.
 * Auth: ops token (servlet_auth_ops).
 * Args: code (one expression; wrap statements in .apply(function(){...}))
 * Reply: {ok:true, value:<json>, repr:"..."} or {ok:false, error:"..."}.
 * value is null for wrappers and functions, repr names them.
 */
static void fenia_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    DLString code;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_ops(params, response))
        return;

    if (!servlet_get_arg(params, response, "code", code))
        return;

    Json::Value reply;

    try {
        Scripting::CodeSource &cs = Scripting::CodeSource::manager->allocate();
        cs.author = "<api>";
        cs.name = "<api fenia>";
        cs.content = code;

        Scripting::Register result = cs.eval(Scripting::Register());
        reply["ok"] = true;
        reply["value"] = JsonUtils::fromRegister(result);
        reply["repr"] = result.repr().c_str();

    } catch (const ::Exception &e) {
        reply["ok"] = false;
        reply["error"] = e.what();
    }

    servlet_response_200_json(response, reply);
}

SERVLET_HANDLE(cmd_fenia, "/fenia")
{
    fenia_servlet(request, response);
}
