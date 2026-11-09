#include "olcservlet.h"
#include "capturebufferhandler.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "merc.h"
#include "sedit.h"
#include "arg_utils.h"

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

static bool is_direction(const DLString &cmdName)
{
    static const char *dirNames[] = { "north", "east", "south", "west", "up", "down", 0 };

    for (int i = 0; dirNames[i]; i++)
        if (cmdName == dirNames[i])
            return true;

    return false;
}

DLString olc_servlet_refusal(const DLString &cmdName, const DLString &arguments, bool hasText)
{
    if (cmdName == "done" || cmdName == "cancel")
        return "Every call commits on its own, no " + cmdName + " needed";

    // create attaches a second editing state that nobody commits.
    if (cmdName == "create")
        return "create is not supported over the API, use it in game";

    DLString args = arguments;
    DLString word = args.getOneArgument();

    if (is_direction(cmdName)) {
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
