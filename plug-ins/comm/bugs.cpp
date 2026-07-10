#include <jsoncpp/json/json.h>
#include "servlet.h"
#include "servlet_utils.h"
#include "commandtemplate.h"
#include "bugtracker.h"
#include "pcharacter.h"
#include "commonattributes.h"
#include "room.h"
#include "messengers.h"
#include "act.h"
#include "l10n.h"

CMDRUNP( nohelp )
{
    if (ch->is_npc())
        return;

    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho(_("Об отсутствии какого раздела справки ты хочешь сообщить?"));
        return;
    }

    bugTracker->reportMessage(getName(), ch->getPC(), txt);
    ch->pecho(_("Записано."));
}

CMDRUNP( bug )
{
    if (ch->is_npc())
        return;

    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho(_("О какой именно ошибке ты хочешь сообщить?"));
        return;
    }

    bugTracker->reportMessage(getName(), ch->getPC(), txt);
    ch->pecho( _("Ошибка записана."));
}

CMDRUNP( typo )
{
    if (ch->is_npc())
        return;

    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho(_("О какой именно опечатке ты хочешь сообщить?"));
        return;
    }

    bugTracker->reportMessage(getName(), ch->getPC(), txt);
    ch->pecho( _("Опечатка записана."));
}

CMDRUNP( idea )
{
    // Minimum idea length, to filter out accidental triggers and one-word noise.
    static const DLString::size_type IDEA_MIN_LENGTH = 15;

    if (ch->is_npc())
        return;

    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho(_("Какой именно идейкой ты хочешь поделиться?"));
        return;
    }

    if (txt.size( ) < IDEA_MIN_LENGTH) {
        ch->pecho(_("Идейка слишком коротка -- опиши ее подробнее (хотя бы %d символов)."),
                  (int)IDEA_MIN_LENGTH);
        return;
    }

    txt = fmt(0, _("от %1$C2]: %2$s"), ch, txt.c_str());
    // Let's experiment and see if this will be abused -- can always mute abusers
    send_to_discord_stream(":bulb: [**Идейка** " + txt);
    send_telegram("[Идейка " + txt);

    bugTracker->reportMessage("idea", ch->getPC(), txt);
    ch->pecho( _("Идейка записана."));

    // Players not linked to Telegram won't see any reply from the devs, so
    // point them at the public chat where ideas get discussed.
    if (get_string_attribute(ch->getPC(), "telegram").empty())
        ch->pecho(_("Ты не привязан к Telegram -- ответ на идейку ищи в нашем чате: "
                  "{y{hlhttps://t.me/dreamland_rocks{x"));
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

    if (!servlet_auth_bot(params, response)) 
        return;

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

