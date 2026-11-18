#include <time.h>
#include <set>
#include <vector>
#include <utility>

#include "olcops.h"
#include "olcservlet.h"
#include "capturebufferhandler.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "olc.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "behavior.h"
#include "globalbitvector.h"
#include "iconvmap.h"
#include "dlfileop.h"
#include "exceptiondbio.h"
#include "dreamland.h"
#include "wiznet.h"
#include "logstream.h"
#include "arg_utils.h"
#include "loadsave.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

static IconvMap utf2koi("utf-8", "koi8-u");
static IconvMap koi2utf("koi8-u", "utf-8");

/*--------------------------------------------------------------------------
 * Text conversion
 *-------------------------------------------------------------------------*/

// Game text is ASCII punctuation only (KOI8 engine): map the usual typographic
// characters instead of refusing them.
static const struct { const char *from; const char *to; } koiPunctuation[] = {
    { "\xE2\x80\xA6", "..." },  // U+2026 ellipsis
    { "\xE2\x80\x94", "--" },   // U+2014 em dash
    { "\xE2\x80\x93", "-" },    // U+2013 en dash
    { "\xCA\xBC", "'" },        // U+02BC modifier letter apostrophe
    { "\xE2\x80\x99", "'" },    // U+2019
    { "\xE2\x80\x98", "'" },    // U+2018
    { "\xE2\x80\x9C", "\"" },   // U+201C
    { "\xE2\x80\x9D", "\"" },   // U+201D
    { "\xC2\xAB", "\"" },       // U+00AB
    { "\xC2\xBB", "\"" },       // U+00BB
    { "\xC2\xA0", " " },        // U+00A0 no-break space
    { 0, 0 }
};

bool OlcOps::toKoi(const std::string &utf, DLString &koi, DLString &error)
{
    std::string mapped = utf;

    for (int i = 0; koiPunctuation[i].from; i++) {
        const std::string from = koiPunctuation[i].from;
        const std::string to = koiPunctuation[i].to;
        std::string::size_type pos = 0;

        while ((pos = mapped.find(from, pos)) != std::string::npos) {
            mapped.replace(pos, from.size(), to);
            pos += to.size();
        }
    }

    koi = utf2koi(mapped);

    std::string back = koi2utf(koi);
    if (back == mapped)
        return true;

    std::string::size_type bad = 0;
    while (bad < back.size() && bad < mapped.size() && back[bad] == mapped[bad])
        bad++;

    if (bad >= mapped.size()) {
        error = "Text does not convert to KOI8-U";
        return false;
    }

    // Step back to the lead byte of the character, count characters before it.
    while (bad > 0 && (mapped[bad] & 0xC0) == 0x80)
        bad--;

    int offset = 0;
    for (std::string::size_type i = 0; i < bad; i++)
        if ((mapped[i] & 0xC0) != 0x80)
            offset++;

    std::string::size_type end = bad + 1;
    while (end < mapped.size() && (mapped[end] & 0xC0) == 0x80)
        end++;

    // The character itself has no KOI8 form: name its code point and offset.
    unsigned int code;
    unsigned char lead = mapped[bad];
    int extra = lead >= 0xF0 ? 3 : lead >= 0xE0 ? 2 : lead >= 0xC0 ? 1 : 0;
    code = extra ? (lead & (0x3F >> extra)) : lead;
    for (std::string::size_type i = bad + 1; i < end; i++)
        code = (code << 6) | (mapped[i] & 0x3F);

    char hex[16];
    snprintf(hex, sizeof(hex), "U+%04X", code);

    error = DLString("Character ") + hex + " at offset " + DLString(offset)
            + " has no KOI8-U form, the game would cut the text there";
    return false;
}

/*--------------------------------------------------------------------------
 * Journal
 *-------------------------------------------------------------------------*/

// In the directory of the immortal command log, wherever the config puts it.
static DLString journal_file_name()
{
    const DLString &immLog = dreamland->getImmLogFile();
    DLString::size_type slash = immLog.rfind('/');

    if (slash == DLString::npos)
        return "olc-journal.log";

    return immLog.substr(0, slash + 1) + "olc-journal.log";
}

void olc_journal(const DLString &actor, const DLString &profile, const DLString &source,
                 const DLString &kind, int vnum, const DLString &cmd, const DLString &args)
{
    char stamp[32];
    time_t now = dreamland->getCurrentTime();
    struct tm tmNow;

    gmtime_r(&now, &tmNow);
    strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%SZ", &tmNow);

    // KOI8 is one byte per character, so a byte cut never splits a letter.
    DLString fullArgs = args.size() > 500 ? DLString(args.substr(0, 500) + "...") : args;

    Json::Value line;
    line["time"] = stamp;
    line["actor"] = koi2utf(actor);
    line["profile"] = profile;
    line["source"] = source;
    line["kind"] = kind;
    line["vnum"] = vnum;
    line["cmd"] = cmd;
    line["args"] = koi2utf(fullArgs);

    try {
        Json::FastWriter writer;
        DLFileAppend(dreamland->getBasePath(), journal_file_name()).write(writer.write(line));
    } catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << "OLC journal: " << ex.what() << endl;
    }

    DLString shortArgs = args.size() > 60 ? DLString(args.substr(0, 60) + "...") : args;
    wiznet(WIZ_SECURE, 0, 0, "OLC %s (%s %s): %s %d %s %s{x",
           actor.c_str(), profile.c_str(), source.c_str(),
           kind.c_str(), vnum, cmd.c_str(), shortArgs.c_str());
}

/*--------------------------------------------------------------------------
 * Policy
 *-------------------------------------------------------------------------*/

const char * OlcOps::policyName(OlcPolicy policy)
{
    switch (policy) {
    case OLC_POLICY_IMMORTAL: return "immortal";
    case OLC_POLICY_PLAYER:   return "player";
    default:                  return "agent";
    }
}

bool OlcOps::policyLookup(const DLString &name, OlcPolicy &policy)
{
    if (name == "immortal")
        policy = OLC_POLICY_IMMORTAL;
    else if (name == "agent")
        policy = OLC_POLICY_AGENT;
    else if (name == "player")
        policy = OLC_POLICY_PLAYER;
    else
        return false;

    return true;
}

/*--------------------------------------------------------------------------
 * Ops
 *-------------------------------------------------------------------------*/

struct OlcOpContext {
    PCharacter *ch;
    CaptureBufferHandler *capture;
    OlcPolicy policy;
    bool dryRun;
    bool verbose;
};

/** The parsed op, every string already KOI8. */
struct OlcOpLine {
    DLString cmdName;       // subcommand as typed
    DLString requiredName;  // 'set' only: the resolved subcommand must be exactly this
    DLString arguments;
    DLString text;
    bool hasText;
};

// The editors' 'behaviors X' replaces the whole list and drops the props of
// every behavior it removes. Over the API it is additive: 'X' adds,
// '-X' (or -'x y', '-x y') removes, '= X Y' replaces the list as typed. Props of the behaviors
// that stay are untouched by editBehaviors, so nothing needs restoring.
static bool additive_behaviors(const GlobalBitvector &current, DLString &arguments, DLString &error)
{
    DLString args = arguments;
    args.stripWhiteSpace();

    if (args.empty() || arg_is_clear(args))
        return true;

    if (args.at(0) == '=') {
        arguments = args.substr(1);
        arguments.stripWhiteSpace();
        return true;
    }

    std::set<int> final = current.toSet();
    DLString word;

    for (;;) {
        args.stripLeftWhiteSpace();
        if (args.empty())
            break;

        // Both -'random weapon' and '-random weapon' remove: getOneArgument
        // honours a quote only as the first character of a word.
        bool remove = args.at(0) == '-';
        if (remove)
            args.erase(0, 1);

        word = args.getOneArgument();
        if (!remove && !word.empty() && word.at(0) == '-') {
            remove = true;
            word.erase(0, 1);
        }

        if (word.empty()) {
            error = "Behavior name missing after '-'";
            return false;
        }

        Behavior *bhv = behaviorManager->findExisting(word);
        if (!bhv)
            bhv = behaviorManager->findUnstrict(word);

        if (!bhv || !bhv->isValid()) {
            error = "Behavior '" + word + "' not found";
            return false;
        }

        if (remove)
            final.erase(bhv->getIndex());
        else
            final.insert(bhv->getIndex());
    }

    if (final.empty()) {
        arguments = "clear";
        return true;
    }

    arguments.clear();
    for (auto b: final)
        arguments += "'" + behaviorManager->find(b)->getName() + "' ";

    arguments.stripWhiteSpace();
    return true;
}

/** Run one subcommand line on a fresh editor state, the way the servlets do:
 *  base attach (no char-side state to restore), handle, commit, base detach. */
template <typename State>
static bool run_on_state(::Pointer<State> state, OlcOpContext &ctx, OlcOpLine &line,
                         const GlobalBitvector &behaviors, Json::Value &res, DLString &error)
{
    CommandBase::Pointer cmd = state->findCommand(ctx.ch, line.cmdName);
    if (!cmd) {
        error = "Unknown " + state->getJournalKind() + " subcommand: " + line.cmdName;
        return false;
    }

    const DLString &name = cmd->getName();

    if (!line.requiredName.empty() && name != line.requiredName) {
        error = "No '" + line.requiredName + "' field on " + state->getJournalKind();
        return false;
    }

    error = olc_servlet_refusal(name, line.arguments, line.hasText);
    if (!error.empty())
        return false;

    if (name == "behaviors" && !additive_behaviors(behaviors, line.arguments, error))
        return false;

    if (ctx.dryRun)
        return true;

    state->journalProfile = OlcOps::policyName(ctx.policy);
    state->journalSource = "api";

    olc_servlet_seed_buffer(ctx.ch, line.text);

    DLString cmdLine = name;
    if (!line.arguments.empty())
        cmdLine += " " + line.arguments;

    std::vector<char> buf(cmdLine.begin(), cmdLine.end());
    buf.push_back(0);

    std::string::size_type mark = ctx.capture ? ctx.capture->getString().size() : 0;

    state->OLCState::attach(ctx.ch);
    try {
        state->handle(ctx.ch->desc, &buf[0]);
        if (name != "show")
            state->commit();
    } catch (...) {
        state->OLCState::detach(ctx.ch);
        throw;
    }
    state->OLCState::detach(ctx.ch);

    res["changed"] = state->lastChanged;

    DLString output;
    if (ctx.capture)
        output = DLString(ctx.capture->getString().substr(mark)).colourStrip();

    // Read-only and refused subcommands answer in the editor's own words.
    if (ctx.verbose || !state->lastChanged)
        res["output"] = output;

    // A 'set' the editor turned down is a failed op, not a no-op.
    if (!line.requiredName.empty() && !state->lastChanged) {
        error = line.requiredName + " not changed: " + output;
        return false;
    }

    return true;
}

static bool op_string(const Json::Value &op, const char *key, DLString &out, DLString &error)
{
    if (!op.isMember(key))
        return true;

    const Json::Value &v = op[key];

    if (v.isString()) {
        if (OlcOps::toKoi(v.asString(), out, error))
            return true;
        error = DLString(key) + ": " + error;
        return false;
    }

    if (v.isNumeric()) {
        out = v.asString();
        return true;
    }

    error = DLString(key) + " must be a string";
    return false;
}

static bool op_vnum(const Json::Value &op, int &vnum, DLString &error)
{
    const Json::Value &v = op["vnum"];

    if (v.isInt())
        vnum = v.asInt();
    else if (v.isString() && DLString(v.asString()).isNumber())
        vnum = DLString(v.asString()).toInt();
    else {
        error = "vnum is required and must be a number";
        return false;
    }

    // A negative vnum reaching the index lookups crashes the server.
    if (vnum <= 0) {
        error = "vnum must be positive";
        return false;
    }

    return true;
}

static const char *textFields[] = { "name", "short", "long", "desc", "sound", "smell", 0 };

static bool is_text_field(const DLString &field)
{
    for (int i = 0; textFields[i]; i++)
        if (field == textFields[i])
            return true;
    return false;
}

/** Turn a 'cmd' or 'set' op into a subcommand line. */
static bool op_line(const Json::Value &op, const DLString &action, OlcOpLine &line, DLString &error)
{
    line.hasText = false;

    if (action == "cmd") {
        DLString cmd;
        if (!op_string(op, "cmd", cmd, error) || !op_string(op, "text", line.text, error))
            return false;

        // getOneArgument trims only the left side: a desc set inline keeps
        // its trailing newline, as in game.
        line.arguments = cmd;
        line.cmdName = line.arguments.getOneArgument();
        line.hasText = !line.text.empty();

        if (line.cmdName.empty()) {
            error = "cmd is required for action cmd";
            return false;
        }
        return true;
    }

    DLString field, value, lang;
    if (!op_string(op, "field", field, error) || !op_string(op, "value", value, error)
        || !op_string(op, "lang", lang, error))
        return false;

    if (field.empty()) {
        error = "field is required for action set";
        return false;
    }

    // 'rudesc' / 'uaname' name the language in the field itself.
    DLString base = field;
    if (field.size() > 2 && (field.substr(0, 2) == "ru" || field.substr(0, 2) == "ua")
        && is_text_field(field.substr(2)))
    {
        DLString fieldLang = field.substr(0, 2);
        if (!lang.empty() && lang != fieldLang) {
            error = "field " + field + " and lang " + lang + " disagree";
            return false;
        }
        lang = fieldLang;
        base = field.substr(2);
    }

    bool clear = op.isMember("clear") && op["clear"].isBool() && op["clear"].asBool();

    if (is_text_field(base)) {
        DLString prefix;
        if (lang.empty() || lang == "en")
            prefix = "";
        else if (lang == "ru" || lang == "ua")
            prefix = lang;
        else {
            error = "lang must be en, ru or ua";
            return false;
        }

        if (value.empty() && !clear) {
            error = "value is empty: send clear:true to empty the field";
            return false;
        }

        if (!value.empty() && clear) {
            error = "clear:true takes no value";
            return false;
        }

        // Through paste, so a value like 'copy' or 'web' is never read as an
        // editor keyword. An empty buffer pasted on purpose clears the field.
        line.cmdName = prefix + base;
        line.requiredName = line.cmdName;
        line.arguments = "paste";
        line.text = value;
        line.hasText = true;
        return true;
    }

    if (!lang.empty()) {
        error = field + " has no language variants";
        return false;
    }

    if (clear) {
        error = "clear:true is for text fields; other fields take their own value, e.g. 'clear' or 'none'";
        return false;
    }

    if (value.empty()) {
        error = "value is required for action set";
        return false;
    }

    line.cmdName = field;
    line.requiredName = field;
    line.arguments = value;
    return true;
}

static bool apply_one(OlcOpContext &ctx, const Json::Value &op, Json::Value &res,
                      DLString &kind, int &vnum, DLString &error)
{
    if (!op.isObject()) {
        error = "op must be an object";
        return false;
    }

    DLString action;
    if (!op_string(op, "kind", kind, error) || !op_string(op, "action", action, error))
        return false;

    if (action.empty())
        action = "cmd";

    if (kind != "room" && kind != "mob" && kind != "obj") {
        error = "kind must be room, mob or obj";
        return false;
    }

    if (action != "get" && action != "cmd" && action != "set") {
        error = "action must be get, cmd or set";
        return false;
    }

    if (!op_vnum(op, vnum, error))
        return false;

    res["kind"] = kind;
    res["vnum"] = vnum;

    if (kind == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        Room *room = pRoom ? get_room_instance(vnum) : 0;
        if (!room) {
            error = "Room vnum " + DLString(vnum) + " not found";
            return false;
        }

        if (action == "get") {
            res["xml"] = olc_room_to_xml(pRoom);
            return true;
        }

        OlcOpLine line;
        if (!op_line(op, action, line, error))
            return false;

        // Room subcommands edit the prototype of the room the actor stands in.
        ctx.ch->in_room = room;

        OLCStateRoom::Pointer sr(NEW);
        sr->room.setValue(vnum);
        // No original room: a throwing subcommand's detach returns early
        // instead of moving the actor.
        sr->originalRoom.setValue(0);
        return run_on_state(sr, ctx, line, pRoom->behaviors, res, error);
    }

    if (action == "get") {
        error = "get is rooms only for now, use action cmd with 'show'";
        return false;
    }

    OlcOpLine line;
    if (!op_line(op, action, line, error))
        return false;

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        error = "Limbo room instance not found";
        return false;
    }
    ctx.ch->in_room = limbo;

    if (kind == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (!pMob || !pMob->area) {
            error = "Mobile vnum " + DLString(vnum) + " not found";
            return false;
        }

        OLCStateMobile::Pointer state(NEW, pMob);
        return run_on_state(state, ctx, line, pMob->behaviors, res, error);
    }

    OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
    if (!pObj || !pObj->area) {
        error = "Object vnum " + DLString(vnum) + " not found";
        return false;
    }

    OLCStateObject::Pointer state(NEW, pObj);
    return run_on_state(state, ctx, line, pObj->behaviors, res, error);
}

Json::Value OlcOps::apply(PCharacter *actor, const Json::Value &ops,
                          OlcPolicy policy, bool dryRun, bool verbose)
{
    Json::Value result;
    result["ok"] = true;
    result["dryRun"] = dryRun;
    result["profile"] = policyName(policy);
    result["changed"] = Json::Value(Json::arrayValue);
    result["errors"] = Json::Value(Json::arrayValue);
    // Validators (phase 2) report here.
    result["warnings"] = Json::Value(Json::arrayValue);
    result["results"] = Json::Value(Json::arrayValue);
    result["skipped"] = 0;

    if (!ops.isArray() || ops.empty()) {
        Json::Value err;
        err["op"] = -1;
        err["error"] = "ops must be a non-empty array";
        result["errors"].append(err);
        result["ok"] = false;
        return result;
    }

    // The player profile needs its field whitelist and validators first.
    if (policy == OLC_POLICY_PLAYER) {
        Json::Value err;
        err["op"] = -1;
        err["error"] = "the player profile is not available yet";
        result["errors"].append(err);
        result["ok"] = false;
        result["skipped"] = (int)ops.size();
        return result;
    }

    OlcOpContext ctx;
    ctx.ch = actor;
    ctx.capture = actor->desc
        ? dynamic_cast<CaptureBufferHandler *>(actor->desc->buffer_handler.getPointer()) : 0;
    ctx.policy = policy;
    ctx.dryRun = dryRun;
    ctx.verbose = verbose;

    std::set<std::pair<DLString, int> > changed;
    Json::Value::ArrayIndex i;

    for (i = 0; i < ops.size(); i++) {
        const Json::Value &op = ops[i];
        Json::Value res;
        DLString kind, error;
        int vnum = 0;

        res["op"] = (int)i;

        bool ok;
        // One bad op (an out of range number, a throwing handler) must not
        // lose the report of the ops already committed before it.
        try {
            ok = apply_one(ctx, op, res, kind, vnum, error);
        } catch (const std::exception &e) {
            ok = false;
            error = DLString("Internal error: ") + e.what();
        }
        res["ok"] = ok;
        result["results"].append(res);

        if (res.isMember("changed") && res["changed"].asBool()
            && changed.insert(std::make_pair(kind, vnum)).second)
        {
            Json::Value c;
            c["kind"] = kind;
            c["vnum"] = vnum;
            result["changed"].append(c);
        }

        if (ok)
            continue;

        Json::Value err;
        err["op"] = (int)i;
        err["error"] = error;
        result["errors"].append(err);
        result["ok"] = false;

        bool cont = op.isObject() && op.isMember("continueOnError") && op["continueOnError"].isBool()
                    && op["continueOnError"].asBool();
        if (!cont) {
            i++;
            break;
        }
    }

    result["skipped"] = (int)(ops.size() - i);
    return result;
}
