#ifndef SERVLET_UTILS_H
#define SERVLET_UTILS_H

class HttpRequest;
class HttpResponse;
class PCMemoryInterface;
class DLString;
namespace Json {
    class Value;
}

bool servlet_parse_params(HttpRequest &request, HttpResponse &response, Json::Value &params);
bool servlet_auth_bot(Json::Value &params, HttpResponse &response);
PCMemoryInterface * servlet_find_player(Json::Value &params, HttpResponse &response);
DLString servlet_find_username(Json::Value &params, HttpResponse &response);
bool servlet_get_arg(const Json::Value &params, const DLString &argName, DLString &argValue);
bool servlet_get_arg(const Json::Value &params, HttpResponse &response, const DLString &argName, DLString &argValue);
void servlet_response_200(HttpResponse &response, const DLString &text);
void servlet_response_404(HttpResponse &response, const DLString &text);
void servlet_response_200_json(HttpResponse &response, const Json::Value &payload);

#endif
