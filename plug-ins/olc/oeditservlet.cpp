#include <string>
#include "iconvmap.h"
#include "servlet.h"
#include "servlet_utils.h"
#include "capturebufferhandler.h"
#include "olcservlet.h"
#include "oedit.h"
#include "olc.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "loadsave.h"
#include "merc.h"
#include "vnum.h"

static IconvMap koi2utf("koi8-u", "utf-8");

/**
 * Servlet for /oedit API endpoint: runs one oedit subcommand on an object
 * prototype and returns the captured output. Each request is its own editing
 * session (copy prototype -> run subcommand -> commit), so commands that open
 * the interactive string editor have no effect: send the text in 'text' and
 * use the 'paste' form instead ('ed paste <keyword>', 'rued paste <keyword>').
 * Auth: bottype=telegram, token=<secret token>
 * Args: vnum (object vnum), cmd (subcommand name, 'show' is read-only),
 *       arguments (subcommand args), text (optional: editor buffer contents
 *       for a 'paste' subcommand)
 *
 * Example: drop affect #1 from object 30235
 *   curl -s 'http://localhost:1235/oedit' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"vnum":"30235","cmd":"delaffect","arguments":"1"}}'
 *   Response (200): Affect removed.
 *
 * The area is marked changed like an interactive edit; run 'asave changed'
 * to write it to disk.
 */
static void oedit_servlet(HttpRequest &request, HttpResponse &response)
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
    OBJ_INDEX_DATA *pObj = vnum > 0 ? get_obj_index(vnum) : 0;
    if (!pObj || !pObj->area) {
        servlet_response_404(response, "Object vnum " + vnumStr + " not found");
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

    OLCStateObject::Pointer oe(NEW, pObj);

    CommandBase::Pointer cmd = oe->findCommand(ch, subcmd);
    if (!cmd) {
        olc_servlet_cleanup_char(ch);
        servlet_response_400(response, "Unknown oedit subcommand: " + subcmd);
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

    // Base attach/detach: OLCStateObject has no char-side state to restore.
    oe->OLCState::attach(ch);
    oe->handle(ch->desc, const_cast<char *>(cmdLine.c_str()));
    if (subcmd != "show")
        oe->commit();
    oe->OLCState::detach(ch);

    DLString output = koi2utf(capture->getString());
    olc_servlet_cleanup_char(ch);

    servlet_response_200(response, output);
}

SERVLET_HANDLE(cmd_oedit, "/oedit")
{
    oedit_servlet(request, response);
}
