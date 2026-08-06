#include "admincommand.h"
#include "logstream.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "arg_utils.h"
#include "act.h"
#include "servlet.h"
#include "servlet_utils.h"
#include "cban.h"
#include "deny.h"
#include "reward.h"
#include "dreamland.h"


void reboot_action(const DLString& arg, ostringstream& buf);

/**
 * Discord or Telegram: /admin reboot|ban|deny|reward args
 * Auth: bottype=discord|telegram, token=<bot secret>
 * Args: id, command
 *
 * The caller is resolved from their player record rather than an online character,
 * so these work with no immortal in the game -- which is the point of 'reward':
 * a queued reward is stored on the target's memory record and handed over on
 * their next login, so neither side has to be connected.
 */
SERVLET_HANDLE(cmd_admin, "/admin")
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response)) 
        return;

    PCMemoryInterface *player = servlet_find_player(params, response);
    if (!player)
        return;

    if (player->get_trust() < 110) {
        servlet_response_404(response, "Command not found");
        return;
    }

    DLString message;
    if (!servlet_get_arg(params, response, "command", message))
        return;

    ostringstream buf;
    DLString cmd = message.getOneArgument();
    DLString cmdArgs = message;

    if (cmd == "reboot") {
        reboot_action(cmdArgs, buf);

    } else if (cmd == "deny") {
        Deny::action(cmdArgs, buf);

    } else if (cmd == "ban") {
        CBan::action(cmdArgs, buf);

    } else if (cmd == "reward") {
        reward_action(cmdArgs, buf);

    }  else {
        servlet_response_404(response, "Command not found");
        return;
    }

    Json::Value rc;
    rc["message"] = buf.str();
    servlet_response_200_json(response, rc);

}


