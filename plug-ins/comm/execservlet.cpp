/* execservlet -- run any in-game command HEADLESS, as a transient synthetic coder.
 *
 * /api/force needs the target character ONLINE: it captures output from that
 * char's live descriptor and 404s when the char is offline. So coder-gated
 * utility commands (vaultmigrate, stat, probes) can't run when no immortal is
 * logged in -- the "log Taiphoen in first" blocker.
 *
 * This servlet removes that blocker. It builds a REAL, fully-registered pooled
 * PCharacter (in char_list, resident in LIMBO) with a socketless descriptor + a
 * capture buffer, grants it coder rights, runs the command through interpret(),
 * captures the text, and then tears the actor down through the engine's own
 * extract_char() -- the same path do_quit uses. Because the actor is a proper
 * in-world char, commands that move/equip/engage/Fenia-expose it (at, goto, load,
 * cast, kill, ...) are cleaned up correctly by extract_char, instead of leaving a
 * freed pointer in a room's people list / object_list / the fight system. This is
 * the fix for the review's BLOCKER-1/2/3: reditservlet's detached synthetic char
 * only survived because OLC subcommands never re-attach the actor; a general
 * command runner must register and extract for real.
 *
 * Why this is safe to do inside a servlet: servlet handling runs in the single
 * main loop, between pulses (dreamland.cpp pulseEnd -> socketManager->run), same
 * thread as the game tick -- no concurrent update runs while the char exists, so
 * register->interpret->extract is atomic from the game's point of view. That is
 * also why /api/force can call interpret() directly.
 *
 * The actor is still SYNTHETIC ("ExecBot"), so commands keyed on a specific
 * immortal's identity (clan, followers, ownership-as-Taiphoen, wiznet persona)
 * won't behave as that immortal -- use /api/force with an online char for those.
 *
 * Auth: bottype=telegram|discord, token=<secret> -- the same god-level token as
 * /force and /reload (already "arbitrary code execution on live").
 * Args: cmd (required), room (optional room vnum to stand in, default LIMBO).
 *
 * Example:
 *   curl -s 'http://localhost:1235/api/exec' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"cmd":"vaultmigrate dry"}}'
 *   Response (200): the command's captured text output.
 */
#include <sstream>

#include "servlet.h"
#include "servlet_utils.h"
#include "bufferhandler.h"
#include "iconvmap.h"
#include "xmlattributecoder.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "descriptor.h"
#include "interp.h"
#include "room.h"
#include "fight_extract.h"
#include "loadsave.h"
#include "exception.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

static IconvMap koi2utf("koi8-u", "utf-8");

// LIMBO room vnum (loadsave/vnum.h ROOM_VNUM_LIMBO); hard-coded to avoid a
// cross-plugin include, same convention as vaultmigrate.cpp's bureau vnums.
// LIMBO is empty of players and mobs, so char_to_room fires no greet/aggro.
#define EXEC_ROOM_LIMBO 2

/* A BufferHandler that captures output to a string instead of a socket. A
 * self-contained copy of olc/CaptureBufferHandler (whose .cpp lives in the olc
 * plugin) so this plugin needs no link against olc. Header-only. */
class ExecCaptureBuffer : public BufferHandler {
public:
    virtual void write(Descriptor *, const char *txt) { captured << txt; }
    DLString getString() const { return captured.str(); }
private:
    std::ostringstream captured;
};

/* Commands that manage the player's OWN session/pfile are meaningless for a
 * transient synthetic actor and awkward to run headless: quit/save write a junk
 * ExecBot pfile, switch hijacks the descriptor onto a mob. The lifecycle below
 * would not CRASH on them (the char_list guard + extract_char handle it), but
 * refusing them keeps "no pfile is touched" true and is honest defense-in-depth.
 * First-token match only -- this is a footgun guard, not a security boundary (the
 * bot token is already god-level), so an obscure alias slipping past it still
 * lands in the safe lifecycle, not a crash. */
static bool exec_command_denied(const DLString &cmd)
{
    DLString rest = cmd;
    DLString first = rest.getOneArgument().toLower();
    return first == "quit" || first == "save" || first == "switch";
}

/* Is this char still linked into the in-world char_list? After a self-extracting
 * command (quit-class, refused above but checked anyway) extract_char has already
 * removed it and returned it to the pool -- we must not touch it again. Pointer
 * compare only, never a field deref, so it is safe even on a recycled pointer. */
static bool exec_char_in_list(Character *ch)
{
    for (Character *wch = char_list; wch != 0; wch = wch->next)
        if (wch == ch)
            return true;
    return false;
}

/* Build a fully-registered pooled coder char with a detached descriptor + capture
 * buffer. Returns the char; hands the descriptor back via `d` so teardown uses the
 * descriptor we made, not ch->desc (which a self-extracting command may have
 * cleared/recycled). */
static PCharacter * exec_setup_char(Room *room, ExecCaptureBuffer *&capture, Descriptor *&d)
{
    capture = new ExecCaptureBuffer();

    d = new Descriptor();
    d->descriptor = -1;                 // detached fd: any ::close/write on it is a harmless no-op
    d->buffer_handler.setPointer(capture);
    d->connected = CON_PLAYING;

    // Pooled, NOT `new PCharacter()`: extract_char ends in PCharacterManager::
    // extract(pc) which returns it to the pool -- a new'd char there would corrupt
    // the pool. getPCharacter() gives a clean, fresh-id'd blank.
    PCharacter *ch = PCharacterManager::getPCharacter();
    ch->setName("ExecBot");
    ch->setSecurity(110);
    ch->lines = 0;                      // disable pager -> page_to_char dumps ALL output to us
    ch->position = POS_STANDING;        // command dispatch allows standing-position commands
    // Grant the 'coder' attribute so isCoder() passes (idiom from
    // characterwrapper.cpp: getAttr<> creates it if absent). get_trust() then
    // reports 0xFFFF, so is_immortal() is true too.
    ch->getAttributes().getAttr<XMLAttributeCoder>("coder");
    d->associate(ch);                   // ch->desc = d; d->character = ch

    // Register for real so commands that move/scan the actor behave and, above all,
    // so extract_char can clean up everything they attach.
    char_to_room(ch, room);
    char_to_list(ch, &char_list);

    return ch;
}

/* Tear the actor down through the engine, then drop our descriptor. */
static void exec_teardown(PCharacter *ch, Descriptor *d)
{
    // If the command self-extracted the actor (quit-class), it is already gone from
    // char_list and returned to the pool -- do NOT extract again. Otherwise run the
    // full engine teardown: stop_fighting, extract carried objects, nuke pets,
    // follower_die, dismount, switch-return, char_from_room, char_from_list,
    // desc->character=0, and PCharacterManager::extract (pool return). extract_char
    // is itself idempotent (guards ch->extracted).
    // count=true so a temp `load obj` of a limited (limit>0, global-capped) item
    // frees its limit slot on teardown instead of consuming it until reboot -- the
    // synthetic actor's items evaporate, they don't persist in a pfile. This mirrors
    // the quit-DIE path (quit.cpp fCount=true), which is production-safe for a PC.
    if (exec_char_in_list(ch))
        extract_char(ch, true);

    // Our descriptor is detached (fd -1, never in descriptor_list); extract_char (or
    // a quit-class command's close()) already nulled its ->character. Plain delete
    // frees outbuf (null) and releases the capture buffer it owns. NEVER close()/
    // slay() it -- slay() would ::close(fd) and walk descriptor_list.
    if (d) {
        d->character = 0;
        delete d;
    }
    // ch is pool-owned now; never delete it here.
}

SERVLET_HANDLE(api_exec, "/exec")
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response))
        return;

    DLString cmd;
    if (!servlet_get_arg(params, response, "cmd", cmd))
        return;

    if (exec_command_denied(cmd)) {
        servlet_response_400(response,
            "api/exec refuses session/pfile commands (quit, save, switch) -- they are "
            "meaningless for a transient synthetic actor. Use /api/force with an online char.");
        return;
    }

    // Optional room to stand in; default LIMBO (empty of players, no triggers).
    int vnum = EXEC_ROOM_LIMBO;
    DLString roomStr;
    if (servlet_get_arg(params, "room", roomStr) && !roomStr.empty()) {
        if (!roomStr.isNumber()) {
            servlet_response_400(response, "Parameter room must be a vnum");
            return;
        }
        vnum = roomStr.toInt();
    }

    Room *room = get_room_instance(vnum);
    if (!room) {
        std::ostringstream em;
        em << "Room instance for vnum " << vnum << " not found";
        servlet_response_404(response, em.str().c_str());
        return;
    }

    ExecCaptureBuffer *capture;
    Descriptor *d;
    PCharacter *ch = exec_setup_char(room, capture, d);

    // Arbitrary commands run here, so a thrown Exception is likelier than on /force's
    // typical use; catch it so a bad command reports back instead of terminating the
    // server. Read the capture inside the try/catch, BEFORE teardown deletes it.
    DLString output;
    try {
        // interpret() returns false when no command matched; surface that instead
        // of a silent empty 200, so a typo reads as a typo (only when the command
        // produced no other output -- never mask real output).
        bool matched = interpret(ch, cmd.c_str());
        output = koi2utf(capture->getString());
        if (!matched && output.empty())
            output = "[exec: command not recognized]";
    } catch (const ::Exception &e) {
        LogStream::sendError() << "api/exec: exception running '" << cmd << "': " << e.what() << endl;
        output = koi2utf(capture->getString()) + "\n[exec exception: " + DLString(e.what()) + "]";
    } catch (const std::exception &e) {
        LogStream::sendError() << "api/exec: std exception running '" << cmd << "': " << e.what() << endl;
        output = koi2utf(capture->getString()) + "\n[exec exception: " + DLString(e.what()) + "]";
    }

    // Teardown ALWAYS runs (after capture is read), success or exception, so the
    // actor never leaks into char_list / the pool.
    exec_teardown(ch, d);

    servlet_response_200(response, output);
}
