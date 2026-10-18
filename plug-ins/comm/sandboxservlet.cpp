/* sandboxservlet -- /api/sandbox: a MORTAL synthetic test actor that lives across
 * requests, so fights, skills and mob reactions can run for real pulses with no
 * player character and no telnet session.
 *
 * One session at a time, always in room 1 (it keeps the test mobs). Actions:
 *   start {level, race, class, sex?} -- build TestBot, raise it like 'advance',
 *       train stats to the race/class cap, full heal. Remembers what room 1 held.
 *   cmd {commands: [..]}  -- run each through interpret(), return the output.
 *   read                  -- output gathered since the last call (fight rounds).
 *   stop                  -- extract the actor and its gear, and purge every mob
 *       and object in room 1 that was not there at start (corpses included).
 * The actor carries 'nosave' (PCharacter::save skips it) and 'sandbox' (raw_kill
 * parks it in LIMBO instead of a real death and marks 'sandbox_died'; the next
 * call reports it and ends the session). Idle 10 minutes = stopped by the reaper.
 *
 * Lifecycle and the main-loop safety argument are the same as execservlet.cpp.
 * Auth: ops token.
 */
#include <set>
#include <time.h>

#include "servlet.h"
#include "servlet_utils.h"
#include "sandboxservlet.h"
#include "execcapture.h"
#include "commonattributes.h"
#include "dlscheduler.h"
#include "iconvmap.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "core/object.h"
#include "race.h"
#include "pcrace.h"
#include "profession.h"
#include "descriptor.h"
#include "interp.h"
#include "room.h"
#include "fight_extract.h"
#include "loadsave.h"
#include "player_exp.h"
#include "exception.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

static IconvMap utf2koi("utf-8", "koi8-u");

// Room 1: no exits, no_mob, test mobs. Its two aggressive blobs (vnum 190) leave
// the actor alone ONLY because a fresh PC's hometown is 'none' with recall 0, and
// 0 falls inside Limbo's vnum range (ai/aggression.cpp co-citizen rule). Give the
// actor a hometown or move the room and it dies on the first aggression pulse.
#define SANDBOX_ROOM 1
#define SANDBOX_IDLE 600

struct SandboxSession {
    PCharacter *ch;
    long long id;                      // pooled chars are recycled: match pointer AND id
    Descriptor *d;
    ExecCaptureBuffer *capture;
    time_t last;
    std::set<long long> mobs, objs;    // room 1 contents at start
};

static SandboxSession *session = 0;

static bool sandbox_command_denied(const DLString &cmd)
{
    DLString rest = cmd;
    DLString first = rest.getOneArgument().toLower();
    return first == "quit" || first == "save" || first == "switch"
        || first == "delete" || first == "remort" || first == "password";
}

static bool sandbox_actor_alive(SandboxSession *s)
{
    for (Character *wch = char_list; wch != 0; wch = wch->next)
        if (wch == s->ch && wch->getID() == s->id)
            return true;
    return false;
}

/* The session is over: the actor "died", or a command extracted it outright.
 * Only an actor found in char_list (pointer and id) is ever dereferenced. */
static bool sandbox_ended()
{
    return session && (!sandbox_actor_alive(session)
        || session->ch->getAttributes().isAvailable("sandbox_died"));
}

/* Tear everything down. Returns the actor's last output. Detaches the session
 * first, so a throw from an extract hook cannot leave it half torn down. */
static DLString sandbox_stop()
{
    if (!session)
        return DLString::emptyString;

    SandboxSession *s = session;
    session = 0;

    DLString out = s->capture->take();

    if (sandbox_actor_alive(s))
        extract_char(s->ch, true);

    s->d->character = 0;
    delete s->d;                 // owns the capture buffer

    Room *room = get_room_instance(SANDBOX_ROOM);
    if (room) {
        Character *rch, *rch_next;
        for (rch = room->people; rch; rch = rch_next) {
            rch_next = rch->next_in_room;
            if (rch->is_npc() && s->mobs.count(rch->getID()) == 0)
                extract_char(rch, true);
        }

        Object *obj, *obj_next;
        for (obj = room->contents; obj; obj = obj_next) {
            obj_next = obj->next_content;
            if (s->objs.count(obj->getID()) == 0)
                extract_obj(obj);
        }
    }

    delete s;
    return out;
}

static DLString sandbox_start(Room *room, int level, const DLString &race,
                              const DLString &prof, int sex)
{
    session = new SandboxSession();
    session->capture = new ExecCaptureBuffer();
    session->last = time(0);

    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        if (rch->is_npc())
            session->mobs.insert(rch->getID());
    for (Object *obj = room->contents; obj; obj = obj->next_content)
        session->objs.insert(obj->getID());

    Descriptor *d = new Descriptor();
    d->descriptor = -1;
    d->buffer_handler.setPointer(session->capture);
    d->connected = CON_PLAYING;
    session->d = d;

    PCharacter *ch = PCharacterManager::getPCharacter();
    session->ch = ch;
    session->id = ch->getID();
    ch->setName("TestBot");
    ch->getAttributes().getAttr<XMLEmptyAttribute>("nosave");
    ch->getAttributes().getAttr<XMLEmptyAttribute>("sandbox");
    // No exp: a level-up would announce TestBot on infonet, Discord and Telegram.
    ch->getAttributes().getAttr<XMLEmptyAttribute>("noexp");
    ch->lines = 0;
    d->associate(ch);           // early, so the raise below is captured too
    ch->setRace(race);
    ch->setProfession(prof);
    ch->setSex(sex);

    // The race-derived body, as PCharacter::load applies it after reading a pfile.
    ch->size        = ch->getRace()->getSize();
    ch->detection   = ch->getRace()->getDet();
    ch->affected_by = ch->getRace()->getAff();
    ch->imm_flags   = ch->getRace()->getImm();
    ch->res_flags   = ch->getRace()->getRes();
    ch->vuln_flags  = ch->getRace()->getVuln();
    ch->form        = ch->getRace()->getForm();
    ch->parts       = ch->getRace()->getParts();
    ch->wearloc.set(ch->getRace()->getWearloc());

    // Same raise as 'advance' from level 1.
    ch->setLevel(1);
    ch->exp = ch->getExpToLevel();
    ch->max_hit = 20;
    ch->max_mana = 100;
    ch->max_move = 100;
    Player::advanceLevel(ch);
    for (int lev = 1; lev < level; lev++) {
        ch->exp += ch->getExpToLevel();
        ch->setLevel(lev + 1);
        Player::advanceLevel(ch);
    }

    for (int i = 0; i < stat_table.size; i++)
        ch->perm_stat[i] = ch->getMaxTrain(i);

    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    ch->position = POS_STANDING;

    char_to_room(ch, room);
    char_to_list(ch, &char_list);

    return session->capture->take();
}

static int sandbox_int_arg(const Json::Value &params, const char *name, int def)
{
    const Json::Value &args = params["args"];
    if (!args.isObject() || !args.isMember(name))
        return def;
    const Json::Value &v = args[name];
    if (v.isInt())
        return v.asInt();
    if (v.isString() && DLString(v.asString()).isNumber())
        return DLString(v.asString()).toInt();
    return def;
}

static void sandbox_reply(HttpResponse &response, const DLString &output, bool ended)
{
    Json::Value reply;
    reply["output"] = output.c_str();       // koi8: servlet_response_200_json converts the body
    reply["active"] = session != 0;
    if (ended)
        reply["ended"] = true;
    servlet_response_200_json(response, reply);
}

SERVLET_HANDLE(api_sandbox, "/sandbox")
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_ops(params, response))
        return;

    DLString action;
    if (!servlet_get_arg(params, response, "action", action))
        return;

    try {
        if (action == "start") {
            if (session) {
                servlet_response_400(response, "a sandbox session is already running, stop it first");
                return;
            }

            int level = sandbox_int_arg(params, "level", 0);
            if (level < 1 || level > LEVEL_HERO) {
                servlet_response_400(response, "level must be 1.." + DLString(LEVEL_HERO));
                return;
            }

            DLString raceName, profName;
            servlet_get_arg(params, "race", raceName);
            servlet_get_arg(params, "class", profName);
            Race *race = raceManager->findExisting(raceName);
            if (!race || !race->isValid() || !race->getPC() || !race->getPC()->isPC()) {
                servlet_response_400(response, "race must be a player race");
                return;
            }
            Profession *prof = professionManager->findExisting(profName);
            if (!prof || !prof->isValid() || !prof->isPlayed()) {
                servlet_response_400(response, "class must be a player class");
                return;
            }

            int sex = sandbox_int_arg(params, "sex", SEX_MALE);
            if (sex != SEX_MALE && sex != SEX_FEMALE) {
                servlet_response_400(response, "sex must be 1 (male) or 2 (female)");
                return;
            }

            Room *room = get_room_instance(SANDBOX_ROOM);
            if (!room) {
                servlet_response_404(response, "sandbox room not found");
                return;
            }

            DLString out = sandbox_start(room, level, race->getName(), prof->getName(), sex);
            sandbox_reply(response, out, false);
            return;
        }

        if (!session) {
            servlet_response_400(response, "no sandbox session, start one first");
            return;
        }
        session->last = time(0);

        if (action == "cmd") {
            const Json::Value &cmds = params["args"]["commands"];
            if (!cmds.isArray() || cmds.empty()) {
                servlet_response_400(response, "commands must be a non-empty array of strings");
                return;
            }

            DLString out;
            for (const auto &c: cmds) {
                if (!c.isString())
                    continue;
                DLString cmd = utf2koi(c.asString());
                out += "> " + cmd + "\n";
                if (sandbox_ended())
                    break;
                if (sandbox_command_denied(cmd)) {
                    out += "[sandbox: refused]\n";
                    continue;
                }
                if (!interpret(session->ch, cmd.c_str()))
                    out += "[sandbox: command not recognized]\n";
                out += session->capture->take();
                if (sandbox_ended())
                    break;
            }

            bool ended = sandbox_ended();
            if (ended)
                out += sandbox_stop();
            sandbox_reply(response, out, ended);
            return;
        }

        if (action == "read") {
            DLString out = session->capture->take();
            bool ended = sandbox_ended();
            if (ended)
                out += sandbox_stop();
            sandbox_reply(response, out, ended);
            return;
        }

        if (action == "stop") {
            DLString out = sandbox_stop();
            sandbox_reply(response, out, true);
            return;
        }

        servlet_response_400(response, "action must be start | cmd | read | stop");

    } catch (const ::Exception &e) {
        LogStream::sendError() << "api/sandbox: " << action << ": " << e.what() << endl;
        servlet_response_400(response, DLString("sandbox exception: ") + e.what());
    }
}

void SandboxReaper::run( )
{
    if (session && time(0) - session->last > SANDBOX_IDLE) {
        LogStream::sendNotice() << "api/sandbox: idle session stopped" << endl;
        try {
            sandbox_stop();
        } catch (const std::exception &e) {
            LogStream::sendError() << "api/sandbox: idle stop: " << e.what() << endl;
        }
    }

    DLScheduler::getThis()->putTaskInSecond(60, Pointer(this));
}

int SandboxReaper::getPriority( ) const
{
    return SCDP_AUTO;
}

void SandboxReaper::destruction( )
{
    try {
        sandbox_stop();
    } catch (const std::exception &e) {
        LogStream::sendError() << "api/sandbox: stop on unload: " << e.what() << endl;
    }
    SchedulerTaskRoundPlugin::destruction();
}
