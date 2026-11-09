#include <string>
#include "iconvmap.h"
#include "servlet.h"
#include "servlet_utils.h"
#include "capturebufferhandler.h"
#include "olcservlet.h"
#include "medit.h"
#include "olc.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "loadsave.h"
#include "merc.h"
#include "vnum.h"

static IconvMap koi2utf("koi8-u", "utf-8");

/**
 * Servlet for /medit API endpoint: runs one medit subcommand on a mobile
 * prototype and returns the captured output. Each request is its own editing
 * session (copy prototype -> run subcommand -> commit).
 * Auth: bottype=telegram, token=<secret token>
 * Args: vnum (mobile vnum), cmd (subcommand name, 'show' is read-only),
 *       arguments (subcommand args), text (optional: editor buffer contents
 *       for a 'paste' subcommand, the way to set multi-line fields)
 *
 * Example: set the Russian description of mobile 3000
 *   curl -s 'http://localhost:1235/medit' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"vnum":"3000","cmd":"rudesc","arguments":"paste","text":"..."}}'
 *
 * The area is marked changed like an interactive edit; run 'asave changed'
 * to write it to disk.
 */
static void medit_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_ops(params, response))
        return;

    DLString vnumStr, subcmd, arguments, text;
    if (!servlet_get_arg(params, response, "vnum", vnumStr))
        return;
    if (!servlet_get_arg(params, response, "cmd", subcmd))
        return;
    servlet_get_arg(params, "arguments", arguments);
    // text is optional: seeds the editor buffer for a "paste" subcommand
    bool hasText = servlet_get_arg(params, "text", text);

    int vnum = vnumStr.toInt();
    MOB_INDEX_DATA *pMob = vnum > 0 ? get_mob_index(vnum) : 0;
    if (!pMob || !pMob->area) {
        servlet_response_404(response, "Mobile vnum " + vnumStr + " not found");
        return;
    }

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        servlet_response_404(response, "Limbo room instance not found");
        return;
    }

    CaptureBufferHandler *capture;
    PCharacter *ch = olc_servlet_setup_char(limbo, capture);
    if (hasText)
        olc_servlet_seed_buffer(ch, text);

    OLCStateMobile::Pointer oe(NEW, pMob);

    CommandBase::Pointer cmd = oe->findCommand(ch, subcmd);
    if (!cmd) {
        olc_servlet_cleanup_char(ch);
        servlet_response_400(response, "Unknown medit subcommand: " + subcmd);
        return;
    }

    DLString refusal = olc_servlet_refusal(cmd->getName(), arguments, hasText);
    if (!refusal.empty()) {
        olc_servlet_cleanup_char(ch);
        servlet_response_400(response, refusal);
        return;
    }

    DLString cmdLine = subcmd;
    if (!arguments.empty())
        cmdLine += " " + arguments;

    // Base attach/detach: OLCStateMobile has no char-side state to restore.
    oe->OLCState::attach(ch);
    oe->handle(ch->desc, const_cast<char *>(cmdLine.c_str()));
    if (subcmd != "show")
        oe->commit();
    oe->OLCState::detach(ch);

    DLString output = koi2utf(capture->getString());
    olc_servlet_cleanup_char(ch);

    servlet_response_200(response, output);
}

SERVLET_HANDLE(cmd_medit, "/medit")
{
    medit_servlet(request, response);
}
