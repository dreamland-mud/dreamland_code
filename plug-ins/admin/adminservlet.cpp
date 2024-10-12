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
#include "dreamland.h"


void reboot_action(const DLString& arg, ostringstream& buf);

/**
 * Discord: /admin reboot|ban|deny args
 * Auth: bottype=discord, token=<discord secret>
 * Args: id, message
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

    }  else {    
        servlet_response_404(response, "Command not found");
        return;
    }

    Json::Value rc;
    rc["message"] = buf.str();
    servlet_response_200_json(response, rc);

}


