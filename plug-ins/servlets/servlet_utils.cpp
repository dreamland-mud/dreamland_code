#include <sstream>
#include "servlet_utils.h"
#include "jsoncpp/json/json.h"
#include "logstream.h"
#include "iconvmap.h"
#include "dlfilestream.h"
#include "servlet.h"
#include "dreamland.h"
#include "commonattributes.h"

static IconvMap utf2koi("utf-8", "koi8-r");
static IconvMap koi2utf("koi8-r", "utf-8");

bool servlet_parse_params(HttpRequest &request, HttpResponse &response, Json::Value &params)
{
    try {
        if (request.method != "POST")
            throw ::Exception("Expecting POST method");

        Json::Reader reader;
        if (!reader.parse(request.body, params))
            throw Exception("Cannot parse JSON body");

        return true;

    } catch (const Exception &e) {
        LogStream::sendError() << "Servlet " << request.uri << ":" << e.what() << endl;
        response.status = 500;
        response.message = "Command failed";
        response.body = e.what();
        return false;
    }
}

static DLString read_token()
{
    DLString token;

    try {
        ostringstream buf;
        DLFileStream tokenFile(dreamland->getMiscDir(), "dreamland_bot.token");
        tokenFile.toStream(buf);
        token = buf.str();
        token.replaces("\n", "");
        
    } catch (const Exception &ex) {
        LogStream::sendError() << "Reading bot token: " << ex.what() << endl;
    }

    return token;
}

bool servlet_auth_bot(Json::Value &params, HttpResponse &response)
{
    DLString myBotType = params["bottype"].asString();  
    myBotType.toLower();

    if (myBotType != "discord" && myBotType != "telegram") {
        response.status = 401;
        response.message = "Unathorized";
        response.body = "Invalid bot type " + myBotType +", expecting Discord or Telegram";
        return false;
    }

    DLString myToken = params["token"].asString();
    DLString token = read_token();

    if (myToken.empty() || token.empty() || myToken != token) {
        LogStream::sendError() << "Servlet token " << myToken << ", expected " << token << endl;
        response.status = 403;
        response.message = "Unathorized";
        response.body = "Invalid token";
        return false;
    }

    return true;
}

/**
 * Find player referred by the args.id payload. For Discord, analyze 'discord' json attribute
 * looking for 'id' field; for Telegram, analyze 'telegram' string attribute containing the id directly.
 */
PCMemoryInterface * servlet_find_player(Json::Value &params, HttpResponse &response)
{
    DLString myId = params["args"]["id"].asString();
    if (myId.empty()) {
        response.status = 400;
        response.message = "Bad request";
        response.body = "Required parameter args.id not found";
        return 0;
    }

    DLString botType = params["bottype"].asString();
    botType.toLower();

    PCMemoryInterface *player = find_player_by_attribute(botType, myId);
    if (player)
        return player;

    list<PCMemoryInterface *> players = find_players_by_json_attribute(botType, "id", myId);
    if (players.empty()) {
        response.status = 404;
        response.message = "Not found";
        response.body = "Player with this " + botType + " ID not found";
        return 0;
    }        

    return players.front();
}   

bool servlet_get_arg(Json::Value &params, HttpResponse &response, const DLString &argName, DLString &argValue)
{
    argValue = utf2koi(params["args"][argName].asString());
    if (argValue.empty()) {
        response.status = 400;
        response.message = "Bad request";
        response.body = "Required parameter args." + argName + " not found";
        return false;
    }

    return true;
}

void servlet_response_200(HttpResponse &response, const DLString &text)
{
    response.status = 200;
    response.message = "OK"; 
    response.body = text;
}

void servlet_response_404(HttpResponse &response, const DLString &text)
{
    response.status = 404;
    response.message = "Not found"; 
    response.body = text;
}

void servlet_response_200_json(HttpResponse &response, const Json::Value &payload)
{
    Json::FastWriter writer;
    response.body = koi2utf(writer.write(payload));        
    response.status = 200;
    response.message = "OK";
    response.headers["content-type"] = "application/json";
}