#include <jsoncpp/json/json.h>
#include "servlet.h"
#include "servlet_utils.h"
#include "commandtemplate.h"
#include "bugtracker.h"
#include "pcharacter.h"
#include "room.h"
#include "messengers.h"
#include "act.h"

CMDRUNP( nohelp )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("Об отсутствии какого раздела справки ты хочешь сообщить?");
        return;
    }

    bugTracker->reportMessage(getName(), ch, txt);
    ch->pecho("Записано.");
}

CMDRUNP( bug )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("О какой именно ошибке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportMessage(getName(), ch, txt);
    ch->pecho( "Ошибка записана.");
}

CMDRUNP( typo )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("О какой именно опечатке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportMessage(getName(), ch, txt);
    ch->pecho( "Опечатка записана.");
}

CMDRUNP( idea )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("Какой именно идейкой ты хочешь поделиться?");
        return;
    }

    txt = fmt(0, "от %1$C2]: %2$s", ch, txt.c_str());
    // Let's experiment and see if this will be abused -- can always mute abusers
    send_to_discord_stream(":bulb: [**Идейка** " + txt);
    send_telegram("[Идейка " + txt);
    
    bugTracker->reportMessage("idea", ch, txt);
    ch->pecho( "Идейка записана.");
}

/** 
 * Common servlet for /typo etc API, does the same as 'typo' command 
 * on behalf of the author id.
 */
static void bugtracker_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    DLString message;

    if (!servlet_parse_params(request, response, params))
        return;

//    if (!servlet_auth_bot(params, response)) 
//        return;

    // Ensure the TG/Discord user is linked to a real player.
    PCMemoryInterface *player = servlet_find_player(params, response);
    if (!player)
        return;

    // Grab the message.
    if (!servlet_get_arg(params, response, "message", message))
        return;

    // Log bug/typo the usual way, using URI to determine message type.
    bugTracker->reportMessage(request.uri.substr(1), player->getName(), message);

    servlet_response_200(response, "Message logged");
}

/**
 * Telegram: /typo <message>, /bug <message>, /idea <message>, /nohelp <message>
 * Auth: bottype=telegram, token=<discord secret>
 * Args: id, message
 */
SERVLET_HANDLE(cmd_typo, "/typo")
{
    bugtracker_servlet(request, response);
}
SERVLET_HANDLE(cmd_bug, "/bug")
{
    bugtracker_servlet(request, response);
}
SERVLET_HANDLE(cmd_idea, "/idea")
{
    bugtracker_servlet(request, response);
}
SERVLET_HANDLE(cmd_nohelp, "/nohelp")
{
    bugtracker_servlet(request, response);
}

