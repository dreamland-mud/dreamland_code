#include <string>
#include "iconvmap.h"
#include "servlet.h"
#include "servlet_utils.h"
#include "capturebufferhandler.h"
#include "xmlroom.h"
#include "redit.h"
#include "olc.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "loadsave.h"
#include "logstream.h"
#include "merc.h"

static IconvMap koi2utf("koi8-u", "utf-8");

/**
 * Servlet for /redit API endpoint.
 * Executes redit subcommands on a given room and returns captured output.
 * Auth: bottype=telegram, token=<secret token>
 * Args: vnum (room vnum), cmd (subcommand name), arguments (subcommand args)
 *
 * Interactive editor modes (sedit, webedit) are safe to invoke but will
 * have no effect — the temporary OLCState is destroyed after each request.
 *
 * Example: get room data as XML
 *   curl -s 'http://localhost:1235/redit' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"vnum":"3001","cmd":"get"}}'
 *   Response (200):
 *     <room><vnum>3001</vnum><name><en>The Temple of Mota</en><ua>...</ua><ru>...</ru></name>
 *     <description><en>You are in the southern end of the temple...</en>...</description>
 *     <sector>inside</sector><flags>indoors</flags><healRate>100</healRate>
 *     <manaRate>100</manaRate>...</room>
 *
 * Example: set room name
 *   curl -s 'http://localhost:1235/redit' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"vnum":"3001","cmd":"name","arguments":"The Temple of Mota"}}'
 *   Response (200):
 *     Name changed.
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

/** Build an XML representation of room fields using the built-in XMLRoom serializer. */
static DLString room_to_xml(RoomIndexData *pRoom)
{
    XMLStreamableBase<XMLRoom> it("room");
    ostringstream os;

    it.init(pRoom);
    it.toStream(os);

    return os.str();
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

    // Handle 'get' — return room fields as XML.
    if (subcmd == "get") {
        servlet_cleanup_char(ch);
        servlet_response_200(response, room_to_xml(pRoom));
        return;
    }

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

        // Push OLCState onto handle_input so OLCInterpretLayer can find it
        // during command dispatch, then remove it after.
        // Use base OLCState::detach to avoid OLCStateRoom::detach's transfer_char.
        sr->OLCState::attach(ch);
        sr->handle(ch->desc, const_cast<char *>(cmdLine.c_str()));
        sr->commit();
        sr->OLCState::detach(ch);
    }

    // Collect output before cleanup, as capture is owned by the descriptor.
    DLString output = koi2utf(capture->getString());
    servlet_cleanup_char(ch);

    servlet_response_200(response, output);
}

SERVLET_HANDLE(cmd_redit, "/redit")
{
    redit_servlet(request, response);
}
