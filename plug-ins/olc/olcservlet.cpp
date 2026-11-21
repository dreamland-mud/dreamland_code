#include "olcservlet.h"
#include "capturebufferhandler.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "merc.h"
#include "sedit.h"
#include "arg_utils.h"
#include "xmlroom.h"
#include "room.h"
#include "servlet.h"
#include "servlet_utils.h"
#include "olcops.h"
#include "vnum.h"
#include "directions.h"

PCharacter * olc_servlet_setup_char(Room *room, CaptureBufferHandler *&capture)
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

void olc_servlet_cleanup_char(PCharacter *ch)
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

void olc_servlet_seed_buffer(PCharacter *ch, const DLString &text)
{
    ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].split(text);
}

bool olc_is_direction(const DLString &name)
{
    for (int door = 0; door < DIR_SOMEWHERE; door++)
        if (name == dirs[door].name)
            return true;

    return false;
}

DLString olc_servlet_refusal(const DLString &cmdName, const DLString &arguments, bool hasText)
{
    // OLCCommand::entryPoint and the handlers copy the line into fixed char
    // buffers; in game the descriptor caps a line, over the API nothing does.
    if (cmdName.size() + 1 + arguments.size() >= MAX_INPUT_LENGTH)
        return "Too long for a command line, send the body in text and use the paste form";

    if (cmdName == "done" || cmdName == "cancel")
        return "Every call commits on its own, no " + cmdName + " needed";

    // The editors' create attaches a second editing state that nobody commits.
    if (cmdName == "create")
        return "create is an action of its own over the API: {action: create}";

    DLString args = arguments;
    DLString word = args.getOneArgument();

    // copy fenia evaluates Fenia code and needs a Fenia right the API actor lacks:
    // refuse it here as an error instead of a refusal hidden in the editor echo.
    if (cmdName == "copy") {
        DLString rest = args;
        DLString second = rest.getOneArgument();
        if (arg_is(word, "fenia") || arg_is(second, "fenia"))
            return "copy fenia is in-game only (it needs Fenia security)";
    }

    if (olc_is_direction(cmdName)) {
        // A bare direction moves the actor through the exit.
        if (word.empty())
            return cmdName + " needs arguments: a bare direction would move the actor";
        // '<dir> rudesc paste': the paste word comes second.
        word = args.getOneArgument();
    }
    else if (cmdName == "props") {
        // 'props <behavior> <prop> paste': the paste word comes third.
        args.getOneArgument();
        word = args.getOneArgument();
    }

    // pastedone runs 'done' inside the request: a second commit, and redit's
    // detach would move the actor into the room.
    if (arg_is_strict(word, "pastedone"))
        return "pastedone is not supported over the API, use paste";

    // The editors take paste from the first word with engine prefix/synonym
    // matching; without text that pastes an empty buffer and wipes the field.
    if (!hasText && arg_is_paste(word))
        return "A paste subcommand needs args.text";

    return DLString::emptyString;
}

DLString olc_room_to_xml(RoomIndexData *pRoom)
{
    XMLStreamableBase<XMLRoom> it("room");
    ostringstream os;

    it.init(pRoom);
    it.toStream(os);

    return os.str();
}

/**
 * Servlet for /olc: a batch of typed OLC ops (see OlcOps in olcops.h).
 * Auth: bottype=telegram, token=<ops token>
 * Args: ops (array), dryRun (bool), verbose (bool: editor echo for every op,
 *       info findings), save (bool: 'asave changed' after the batch),
 *       strict (bool: agent profile, validator warnings become errors and every
 *       finding is listed),
 *       profile (immortal|agent, default agent; player is refused until phase 5),
 *       actor (name for the journal)
 *
 * Example: Russian room name and an object short description in one batch
 *   curl -s 'http://localhost:1235/olc' -d '{"token":"<secret>","bottype":"telegram","args":{"ops":[
 *     {"kind":"room","action":"set","vnum":3001,"field":"name","lang":"ru","value":"..."},
 *     {"kind":"obj","action":"cmd","vnum":3010,"cmd":"level 5"}]}}'
 *   Response (200, JSON): {"ok":true,"changed":[{"kind":"room","vnum":3001},...],
 *     "errors":[],"warnings":[],"findings":[],"findingsSummary":{},
 *     "results":[{"op":0,"ok":true,"changed":true,...}],"skipped":0}
 *
 * Areas are marked changed like an interactive edit; save, 'asave changed' or
 * the minute autosave writes them to disk.
 */
static void olc_ops_servlet(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_ops(params, response))
        return;

    const Json::Value &args = params["args"];
    if (!args.isObject() || !args.isMember("ops")) {
        servlet_response_400(response, "Required parameter args.ops not found");
        return;
    }

    DLString profile = "agent";
    servlet_get_arg(params, "profile", profile);

    OlcPolicy policy;
    if (!OlcOps::policyLookup(profile, policy)) {
        servlet_response_400(response, "args.profile must be immortal, agent or player");
        return;
    }

    bool dryRun = args.isMember("dryRun") && args["dryRun"].isBool() && args["dryRun"].asBool();
    bool verbose = args.isMember("verbose") && args["verbose"].isBool() && args["verbose"].asBool();
    bool save = args.isMember("save") && args["save"].isBool() && args["save"].asBool();
    bool strict = args.isMember("strict") && args["strict"].isBool() && args["strict"].asBool();

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        servlet_response_404(response, "Limbo room instance not found");
        return;
    }

    CaptureBufferHandler *capture;
    PCharacter *ch = olc_servlet_setup_char(limbo, capture);

    DLString actor;
    if (servlet_get_arg(params, "actor", actor))
        ch->setName(actor);

    Json::Value result;
    try {
        result = OlcOps::apply(ch, args["ops"], policy, dryRun, verbose, save, strict);
    } catch (...) {
        olc_servlet_cleanup_char(ch);
        throw;
    }

    olc_servlet_cleanup_char(ch);
    servlet_response_200_json(response, result);
}

SERVLET_HANDLE(cmd_olc, "/olc")
{
    olc_ops_servlet(request, response);
}
