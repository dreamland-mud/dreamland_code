#include "jsoncpp/json/json.h"
#include "servlet.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcharactermemory.h"
#include "interp.h"
#include "servlet_utils.h"


/**
 * Discord: /ooc <message>
 * Auth: bottype=discord, token=<discord secret>
 * Args: id, message
 */
SERVLET_HANDLE(cmd_ooc, "/ooc")
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response)) 
        return;

    PCMemoryInterface *player = servlet_find_player(params, response);
    if (!player)
        return;

    DLString message;
    if (!servlet_get_arg(params, response, "message", message))
        return;

    if (player->isOnline()) {
        interpret_raw(player->getPlayer(), "ooc", message.c_str());
    } else {
        PCharacterMemory *memory = dynamic_cast<PCharacterMemory *>(player);
        PCharacter dummy;
        dummy.setMemory(memory);
        interpret_raw(&dummy, "ooc", message.c_str());
    }

    servlet_response_200(response, "Message sent");
}


