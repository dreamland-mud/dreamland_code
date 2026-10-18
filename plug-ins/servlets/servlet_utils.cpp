#include <sstream>
#include "servlet_utils.h"
#include "jsoncpp/json/json.h"
#include "logstream.h"
#include "iconvmap.h"
#include "dlfilestream.h"
#include "dlfile.h"
#include "servlet.h"
#include "pcmemoryinterface.h"
#include "dreamland.h"
#include "commonattributes.h"

static IconvMap utf2koi("utf-8", "koi8-u");
static IconvMap koi2utf("koi8-u", "utf-8");

bool servlet_parse_params(HttpRequest &request, HttpResponse &response, Json::Value &params)
{
    try {
        if (request.method != "POST")
            throw ::Exception("Expecting POST method");

        Json::Reader reader;
        if (!reader.parse(request.body, params))
            throw Exception("Cannot parse JSON body");

        // Reject a non-object JSON root ([], "x", 5, true). Every servlet indexes params as
        // an object (params["bottype"], params["args"]...), and jsoncpp's operator[] on a
        // non-object root throws Json::LogicError. Member-level type errors (e.g. a non-string
        // "bottype") can still throw downstream; those are caught at the ServletManager boundary.
        if (!params.isObject())
            throw Exception("JSON body must be an object");

        // Same for args: helpers index params["args"] by key. /update/all takes an array.
        if (params.isMember("args") && !params["args"].isObject() && !params["args"].isArray()) {
            servlet_response_400(response, "args must be an object or an array");
            return false;
        }

        return true;

    } catch (const std::exception &e) {
        LogStream::sendError() << "Servlet " << request.uri << ":" << e.what() << endl;
        response.status = 500;
        response.message = "Command failed";
        response.body = e.what();
        return false;
    }
}

static DLString read_token(const char *fileName, bool optional = false)
{
    DLString token;

    try {
        ostringstream buf;
        DLFileStream tokenFile(dreamland->getMiscDir(), fileName);
        tokenFile.toStream(buf);
        token = buf.str();
        token.replaces("\n", "");
        token.replaces("\r", "");
        
    } catch (const std::exception &ex) {
        if (!optional)
            LogStream::sendError() << "Reading " << fileName << ": " << ex.what() << endl;
    }

    return token;
}

// Accepts `token`, or `alt` when it is set: an ops token also opens bot endpoints.
static bool servlet_check_token(Json::Value &params, HttpResponse &response,
                                const DLString &token, const DLString &alt)
{
    // Type-check before asString: a non-string bottype/token used to throw pre-auth.
    if (!params["bottype"].isString() || !params["token"].isString()) {
        servlet_response_400(response, "bottype and token must be strings");
        return false;
    }

    DLString myBotType = params["bottype"].asString();  
    myBotType.toLower();

    if (myBotType != "discord" && myBotType != "telegram") {
        response.status = 401;
        response.message = "Unathorized";
        response.body = "Invalid bot type " + myBotType +", expecting Discord or Telegram";
        return false;
    }

    DLString myToken = params["token"].asString();
    bool match = !myToken.empty()
                 && ((!token.empty() && myToken == token) || (!alt.empty() && myToken == alt));

    if (!match) {
        LogStream::sendError() << "Servlet auth failed: invalid or missing token" << endl;
        response.status = 403;
        response.message = "Unathorized";
        response.body = "Invalid token";
        return false;
    }

    return true;
}

// The bot token: community endpoints and /admin. The Discord and Telegram bots
// hold it, so it must never reach code execution. The ops token opens these too.
bool servlet_auth_bot(Json::Value &params, HttpResponse &response)
{
    return servlet_check_token(params, response,
                               read_token("dreamland_bot.token"),
                               read_token("dreamland_ops.token", true));
}

// The ops token: endpoints that run code or commands (/eval, /exec, /force,
// /snoop, /reload, /redit). Only the maintainer tooling holds it. While
// var/misc/dreamland_ops.token does not exist the bot token is accepted here, so
// the switch-over needs no downtime. Once the file exists the bot token never
// passes: an empty or unreadable file locks these endpoints instead of failing open.
bool servlet_auth_ops(Json::Value &params, HttpResponse &response)
{
    DLString token;

    if (!DLFile(dreamland->getMiscDir(), "dreamland_ops.token").exist())
        token = read_token("dreamland_bot.token");
    else {
        token = read_token("dreamland_ops.token");
        if (token.empty())
            LogStream::sendError() << "dreamland_ops.token is empty or unreadable, ops endpoints locked" << endl;
    }

    return servlet_check_token(params, response, token, DLString::emptyString);
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

/**
 * Like servlet_find_player, but when several characters share the same bot id
 * (account linking puts one telegram/discord id on all of a person's characters)
 * return the HIGHEST-TRUST match rather than the first by iteration. The admin
 * servlet uses this so a shared telegram id resolves to the caller's immortal, not
 * whichever mortal happens to sort first. No response side effects -- the caller
 * decides how to reject; returns 0 when nothing matches.
 */
PCMemoryInterface * servlet_find_player_best_trust(Json::Value &params)
{
    DLString myId = params["args"]["id"].asString();
    if (myId.empty())
        return 0;

    DLString botType = params["bottype"].asString();
    botType.toLower();

    list<PCMemoryInterface *> matches = find_players_by_attribute(botType, myId);
    list<PCMemoryInterface *> jsonMatches = find_players_by_json_attribute(botType, "id", myId);
    matches.insert(matches.end(), jsonMatches.begin(), jsonMatches.end());

    PCMemoryInterface *best = 0;
    for (PCMemoryInterface *player: matches) {
        if (!best || player->get_trust() > best->get_trust())
            best = player;
    }

    return best;
}

// Grab player name or just use the provided Telegram id.
DLString servlet_find_username(Json::Value &params, HttpResponse &response)
{
    PCMemoryInterface *player = servlet_find_player(params, response);

    if (player)
        return player->getName();
    else if (response.status == 404)
        return params["args"]["id"].asString();
    else
        return DLString::emptyString;
}

bool servlet_get_arg(const Json::Value &params, const DLString &argName, DLString &argValue)
{
    if (!params.isMember("args") || !params["args"].isMember(argName))
        return false;

    if (!params["args"][argName].isConvertibleTo(Json::stringValue))
        return false;

    argValue = utf2koi(params["args"][argName].asString());
    return !argValue.empty();
}

bool servlet_get_arg(const Json::Value &params, HttpResponse &response, const DLString &argName, DLString &argValue)
{
    if (!servlet_get_arg(params, argName, argValue)) {
        servlet_response_400(response, "Required parameter args." + argName + " not found");
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

void servlet_response_400(HttpResponse &response, const DLString &text)
{
    response.status = 400;
    response.message = "Bad request";
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
