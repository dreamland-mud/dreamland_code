#include <jsoncpp/json/json.h>
#include "servlet.h"
#include "servlet_utils.h"
#include "capturebufferhandler.h"
#include "redit.h"
#include "olc.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "loadsave.h"
#include "logstream.h"

/**
 * Servlet for /redit API endpoint.
 * Executes redit subcommands on a given room and returns captured output.
 * Auth: bottype=telegram, token=<secret token>
 * Args: vnum (room vnum), cmd (subcommand name), arguments (subcommand args)
 *
 * Interactive editor modes (sedit, webedit) are safe to invoke but will
 * have no effect — the temporary OLCState is destroyed after each request.
 */

/** Set up a temporary PCharacter + Descriptor with a capture buffer for collecting output. */
static PCharacter * servlet_setup_char(Room *room, CaptureBufferHandler *&capture)
{
    capture = new CaptureBufferHandler();

    Descriptor *d = new Descriptor();
    d->buffer_handler.setPointer(capture);
    d->connected = CON_PLAYING;

    PCharacter *ch = new PCharacter();
    ch->setName("ServletBot");
    ch->setSecurity(110);
    ch->desc = d;
    d->character = ch;
    ch->in_room = room;

    return ch;
}

/** Tear down the temporary PCharacter + Descriptor created by servlet_setup_char. */
static void servlet_cleanup_char(PCharacter *ch)
{
    Descriptor *d = ch->desc;
    ch->desc = 0;
    ch->in_room = 0;
    if (d) {
        d->character = 0;
        delete d;
    }
    delete ch;
}

static void redit_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response))
        return;

    // Extract parameters.
    DLString vnumStr, subcmd, arguments;
    if (!servlet_get_arg(params, response, "vnum", vnumStr))
        return;
    if (!servlet_get_arg(params, response, "cmd", subcmd))
        return;
    // arguments are optional
    servlet_get_arg(params, "arguments", arguments);

    int vnum = vnumStr.toInt();
    RoomIndexData *pRoom = get_room_index(vnum);
    if (!pRoom) {
        response.status = 404;
        response.message = "Not found";
        response.body = "Room vnum " + vnumStr + " not found";
        return;
    }

    Room *roomInstance = get_room_instance(vnum);
    if (!roomInstance) {
        response.status = 404;
        response.message = "Not found";
        response.body = "Room instance for vnum " + vnumStr + " not found";
        return;
    }

    CaptureBufferHandler *capture;
    PCharacter *ch = servlet_setup_char(roomInstance, capture);

    // Handle 'show' as a direct static call — doesn't need OLCState.
    if (subcmd == "show") {
        OLCStateRoom::show(ch, pRoom, false);
    } else {
        // Create an OLCStateRoom and dispatch the subcommand via handle(),
        // which sets up 'owner' and internal state needed by edit methods.
        OLCStateRoom::Pointer sr(NEW);
        sr->room.setValue(pRoom->vnum);
        sr->originalRoom.setValue(pRoom->vnum);

        // Build full command line: "subcmd arguments"
        DLString cmdLine = subcmd;
        if (!arguments.empty())
            cmdLine += " " + arguments;

        // Verify the subcommand exists before dispatching.
        CommandBase::Pointer cmd = sr->findCommand(ch, subcmd);
        if (!cmd) {
            servlet_cleanup_char(ch);
            response.status = 400;
            response.message = "Bad request";
            response.body = "Unknown redit subcommand: " + subcmd;
            return;
        }

        sr->handle(ch->desc, const_cast<char *>(cmdLine.c_str()));
        sr->commit();
    }

    // Collect output before cleanup, as capture is owned by the descriptor.
    DLString output = capture->getString();
    servlet_cleanup_char(ch);

    servlet_response_200(response, output);
}

SERVLET_HANDLE(cmd_redit, "/redit")
{
    redit_servlet(request, response);
}
