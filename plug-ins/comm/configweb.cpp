/* Settings dialog support for the web client: the plumbing, and nothing else.
 *
 * The dialog draws itself from what the server tells it, so a setting added to
 * the world shows up in it without touching the client. Three answers carry
 * that, and none of them is built here:
 *
 *   config_schema   what settings exist, on which page, called what in the
 *                   player's language -- assembled in Fenia out of
 *                   config/settings.json, so a wording fix is a world edit;
 *   config_result   what came of one change, always, including the six paths
 *                   that used to print nothing at all;
 *   config_changed  one setting moved by some other hand -- a typed command, an
 *                   alt on the same account -- so an open dialog follows it.
 *
 * What stays here is what cannot live anywhere else: the option-to-bit mapping
 * (it is C++ and belongs to the 'config' command), the apply step, and the
 * socket. The words, the pages, the order and the limits are world data.
 *
 * An older client asks for none of this and is none the wiser; a newer client
 * asking an older server gets no answer at all, which is exactly how it learns
 * that this server has no settings to offer.
 */
#include <algorithm>
#include <map>
#include <set>
#include <ctime>

#include "jsoncpp/json/json.h"

#include "configs.h"
#include "configweb.h"

#include "fenia/exceptions.h"
#include "fenia/register-impl.h"
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "fenia/object.h"
#include "regcontainer.h"
#include "json_utils_ext.h"

#include "configurable.h"
#include "rpccommandmanager.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "accountmanager.h"
#include "mudtags.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

/** config/settings.json: pages, labels, types and limits, in three languages.
 *  Registered here and nowhere read by C++ -- Fenia gets it through
 *  .config("config/settings") and the boot check compares it with the live
 *  option table. */
CONFIGURABLE_DECL(config, settings)

/*-------------------------------------------------------------------------
 * Who asked, and how recently
 *------------------------------------------------------------------------*/

/** Deltas go only to a client that has asked for the schema. A player who never
 *  opens the dialog costs the server nothing on the wire, which is the whole
 *  reason the values do not ride on the prompt any more.
 *
 *  Keyed by character name rather than by descriptor: a Descriptor is gone the
 *  moment the link drops, and a stale pointer here would be written to. */
static std::set<DLString> config_web_asked;

/** Two options write the pfile on every change (telegram, discord), and
 *  wsHandleFrame drains a whole read at once -- twenty frames in one read would
 *  be twenty saves in one pulse.
 *
 *  A flat one-per-second refused the third switch of a hand going down a page,
 *  which is an ordinary thing to do and not an attack. So: a bucket that starts
 *  full, pays one for a switch and four for an option that saves the pfile, and
 *  fills back up at two a second. A player clicking their way down the page
 *  never notices it; a script hammering the socket is held to two changes a
 *  second, and to one identity write every two. */
struct ConfigWebBucket {
    time_t at;
    int tokens;
};

static std::map<DLString, ConfigWebBucket> config_web_bucket;

static const int CONFIG_WEB_BURST  = 8;  // clicks in a row, from full
static const int CONFIG_WEB_REFILL = 2;  // tokens gained per second
static const int CONFIG_WEB_SAVES  = 4;  // what an option that writes the pfile costs

/** What one change of this option costs. */
static int config_web_cost(const DLString &key)
{
    return (key == "telegram" || key == "discord") ? CONFIG_WEB_SAVES : 1;
}

/** Take the price of this change out of the character's bucket, or refuse. */
static bool config_web_afford(const DLString &name, const DLString &key)
{
    time_t now = time(0);
    ConfigWebBucket &bucket = config_web_bucket[name];

    if (bucket.at == 0) {
        bucket.at = now;
        bucket.tokens = CONFIG_WEB_BURST;
    }

    if (now > bucket.at) {
        long gained = (long)(now - bucket.at) * CONFIG_WEB_REFILL;
        bucket.tokens = (int)std::min((long)CONFIG_WEB_BURST, bucket.tokens + gained);
        bucket.at = now;
    }

    int price = config_web_cost(key);
    if (bucket.tokens < price)
        return false;

    bucket.tokens -= price;
    return true;
}

/** Is this character actually in the world? Between the greeting and the first
 *  step there is a character already -- the one the entry script is filling in
 *  -- and its settings are nobody's yet. */
static bool config_web_playing(Character *ch)
{
    return ch && ch->desc && ch->desc->connected == CON_PLAYING;
}

/** A key comes from the far end of a socket and ends up in a log line. Anything
 *  that is not a plain option name is cut down before it gets there: a newline
 *  would forge a log record, and a long one would bury the line it matters to. */
static DLString config_web_safe_key(const DLString &key)
{
    static const unsigned int MAX = 32;
    DLString result;

    for (unsigned int i = 0; i < key.size() && result.size() < MAX; i++) {
        char c = key.at(i);
        if (isalnum(c) || c == '_' || c == '-')
            result += c;
        else
            result += '?';
    }

    return result;
}

/*-------------------------------------------------------------------------
 * Frames
 *------------------------------------------------------------------------*/
static void config_web_send(Character *ch, const DLString &command, const Json::Value &body)
{
    if (!ch || !ch->desc)
        return;

    Json::Value frame;
    frame["command"] = command;
    frame["args"][0] = body;
    ch->desc->writeWSCommand(frame);
}

/** The answer to one change. Sent on every path, refusals included: a dialog
 *  that is told nothing has to guess, and a guessed switch position is a lie
 *  about the world. */
static void config_web_result(Character *ch, const DLString &key, bool ok,
                              const Json::Value &value, const DLString &error,
                              const DLString &reason)
{
    Json::Value body;
    body["key"] = key;
    body["ok"] = ok ? 1 : 0;

    if (ok)
        body["value"] = value;
    else {
        body["error"] = error;
        if (!reason.empty())
            body["reason"] = reason;
    }

    config_web_send(ch, "config_result", body);
}

void web_config_changed( PCharacter *ch, const DLString &key, const Json::Value &value )
{
    if (!ch || !config_web_playing(ch))
        return;

    if (config_web_asked.count(ch->getName()) == 0)
        return;

    Json::Value body;
    body["key"] = key;
    // A switch travels as 1 or 0, never as true/false: spelling it out costs
    // four bytes an option and the client reads a number either way. The
    // account store keeps these as booleans, so this is where they become
    // numbers again.
    body["value"] = value.isBool() ? Json::Value(value.asBool() ? 1 : 0) : value;
    config_web_send(ch, "config_changed", body);
}

void web_config_changed_account( const DLString &id, const DLString &key,
                                 const Json::Value &value, PCharacter *except )
{
    if (id.empty())
        return;

    for (Character *wch = char_list; wch != 0; wch = wch->next) {
        if (wch->is_npc())
            continue;

        PCharacter *pch = wch->getPC();
        if (pch == 0 || pch == except)
            continue;

        if (AccountManager::accountOf(pch->getName()) == id)
            web_config_changed(pch, key, value);
    }
}

/*-------------------------------------------------------------------------
 * Fenia: everything that is a matter of words, pages and limits
 *------------------------------------------------------------------------*/

/** Call .tmp.webconfig.<name>(args). A world without the script answers nothing
 *  at all, and to the client that is indistinguishable from a server too old to
 *  know about settings -- which is the right thing for it to see. */
static bool config_web_fenia(const char *name, const RegisterList &args, Register &result)
{
    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP("tmp"), ID_WEBCONFIG("webconfig");
    IdRef ID_FUNC(name);

    try {
        Register tmp = *Context::root[ID_TMP];
        Register webconfig = *tmp[ID_WEBCONFIG];
        Register function = *webconfig[ID_FUNC];

        if (function.type != Register::FUNCTION)
            return false;

        result = function.toFunction()->invoke(webconfig, args);
        return true;

    } catch (const ::Exception &e) {
        FeniaManager::getThis()->croak(0, Register(DLString("webconfig.") + name), e);
        return false;
    }
}

/*-------------------------------------------------------------------------
 * Colour, which only the terminal renderer knows how to paint
 *------------------------------------------------------------------------*/

/** A sample of the game's own output on its way to a browser. The world writes
 *  it with the game's colour codes, exactly as the terminal shows it, and the
 *  renderer turns those into the tags the web client paints -- the same path
 *  every console line takes. The dialog has nowhere to go for a help link, so
 *  those are resolved away rather than left as '{hh96'. */
static DLString config_web_paint(const DLString &source, Character *ch)
{
    if (source.empty())
        return source;

    ostringstream buf;
    // The same flags every console line takes (output/character.cpp), and for
    // the same reason: the sample is meant to show what THIS player's terminal
    // shows. So the renderer decides -- web tags for a web client, and no
    // colour at all for somebody who turned colour off, who would otherwise be
    // shown a coloured sample of their colourless output. ENFORCE_WEB was
    // exactly that bug. Without CONVERT_COLOR the codes travel as the literal
    // '{r' the world wrote.
    mudtags_convert(source.c_str(), buf, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR, ch);
    return buf.str();
}

/** The one place in the answer where colour lives: the samples under a setting's
 *  question mark. Walked by hand rather than painted wholesale -- a label, a
 *  hint or a player's Telegram handle is text, and a brace in it is a brace. */
static void config_web_paint_examples(Json::Value &schema, Character *ch)
{
    for (Json::Value &section: schema["sections"])
        for (Json::Value &page: section["pages"])
            for (Json::Value &option: page["options"])
                for (Json::Value &sample: option["examples"])
                    if (sample.isMember("text"))
                        sample["text"] = config_web_paint(sample["text"].asString(), ch);
}

/** Does the world file still describe the options this server actually has?
 *  Asked once per boot, the first time anybody opens the dialog: the option
 *  table is built by then, and a mismatch is a setting the dialog shows and
 *  cannot change, or one it never shows at all. Both are silent otherwise, and
 *  both are cheaper to find here than in a bug report. */
static void config_web_check(PCharacter *ch)
{
    static bool checked = false;
    if (checked)
        return;
    checked = true;

    ConfigCommand *config = ConfigCommand::getThis();
    if (!config)
        return;

    Json::Value values;
    config->webValues(ch, values);

    RegList::Pointer live(NEW);
    for (auto &name: values.getMemberNames())
        live->push_back(Register(DLString(name)));

    Scripting::Object *listObj = &Scripting::Object::manager->allocate();
    listObj->setHandler(live);

    RegisterList args1;
    args1.push_back(Register(listObj));

    Register answer;
    if (!config_web_fenia("check", args1, answer))
        return;

    Json::Value problems = JsonUtils::fromRegister(answer);
    for (auto &problem: problems)
        LogStream::sendWarning() << "config/settings.json: " << problem.asString() << endl;
}

/*-------------------------------------------------------------------------
 * The two calls the dialog makes
 *------------------------------------------------------------------------*/

/** Everything the dialog needs to draw itself, in the language this player
 *  reads. The pages and the words come from the world; the values come from
 *  here, because the option-to-bit mapping is here. */
RPCRUN(config_schema)
{
    PCharacter *pch = ch->getPC();
    if (!pch || !config_web_playing(ch))
        return;

    RegisterList args1;
    args1.push_back(FeniaManager::wrapperManager->getWrapper((Character *)ch));

    Register answer;
    if (!config_web_fenia("schema", args1, answer))
        return;

    Json::Value schema = JsonUtils::fromRegister(answer);
    if (!schema.isObject()) {
        LogStream::sendWarning() << "config_schema: script returned no structure" << endl;
        return;
    }

    ConfigCommand *config = ConfigCommand::getThis();
    if (config) {
        Json::Value values;
        config->webValues(pch, values);
        schema["values"] = values;
    }

    config_web_paint_examples(schema, ch);
    config_web_check(pch);

    // Identity is the server's word, not the script's: the dialog throws
    // everything away when this changes, and it must change on a character
    // change even if the script forgot to say so.
    schema["who"] = pch->getName();

    config_web_asked.insert(pch->getName());
    config_web_send(ch, "config_schema", schema);
}

/** Change one setting. Whether it was clicked or typed, the same handler runs
 *  and the same line appears in the terminal. */
RPCRUN(config_set)
{
    PCharacter *pch = ch->getPC();
    if (!pch || !config_web_playing(ch))
        return;

    if (args.size() < 2) {
        LogStream::sendWarning() << "config_set: " << args.size() << " argument(s)" << endl;
        config_web_result(ch, "", false, Json::Value::null, "args", DLString::emptyString);
        return;
    }

    const DLString &key = args[0];
    const DLString &value = args[1];

    if (!config_web_afford(pch->getName(), key)) {
        config_web_result(ch, config_web_safe_key(key), false, Json::Value::null,
                          "throttled", DLString::emptyString);
        return;
    }

    // What this value is allowed to be is a matter of world data: the type, the
    // length, the range and the words all live in config/settings.json.
    RegisterList args3;
    args3.push_back(FeniaManager::wrapperManager->getWrapper((Character *)ch));
    args3.push_back(Register(key));
    args3.push_back(Register(value));

    Register answer;
    if (!config_web_fenia("set", args3, answer)) {
        config_web_result(ch, config_web_safe_key(key), false, Json::Value::null,
                          "server", DLString::emptyString);
        return;
    }

    Json::Value verdict = JsonUtils::fromRegister(answer);
    if (!verdict.isObject() || !verdict["ok"].asBool()) {
        DLString error = "server", reason;
        if (verdict.isObject()) {
            error = verdict["error"].asString();
            reason = verdict["reason"].asString();
        }

        config_web_result(ch, config_web_safe_key(key), false, Json::Value::null,
                          error.empty() ? DLString("refused") : error, reason);
        return;
    }

    ConfigCommand *config = ConfigCommand::getThis();
    if (!config)
        return;

    Json::Value stored;
    if (!config->webApply(pch, key, verdict["value"].asString(), stored)) {
        // Described in the world but unknown to the code, or not for this
        // player: either way the dialog must not be left thinking it worked.
        LogStream::sendWarning() << "config_set: no such option '"
                                 << config_web_safe_key(key) << "'" << endl;
        config_web_result(ch, config_web_safe_key(key), false, Json::Value::null,
                          "unknown", DLString::emptyString);
        return;
    }

    config_web_result(ch, key, true, stored, DLString::emptyString, DLString::emptyString);
}

/*-------------------------------------------------------------------------
 * Entering and leaving the world
 *------------------------------------------------------------------------*/
void ConfigWebStateListener::run( int oldState, int newState, Descriptor *d )
{
    if (d == 0)
        return;

    PCharacter *pch = d->character ? d->character->getPC() : 0;

    if (newState == CON_PLAYING) {
        Json::Value body;
        body["playing"] = 1;
        if (pch)
            body["who"] = pch->getName();

        config_web_send(d->character, "config_state", body);
        return;
    }

    if (oldState != CON_PLAYING)
        return;

    // Out of the world: a hotkey script that quits and logs in another character
    // leaves the dialog holding somebody else's settings otherwise.
    if (pch) {
        config_web_asked.erase(pch->getName());
        config_web_bucket.erase(pch->getName());
    }

    Json::Value body;
    body["playing"] = 0;

    Json::Value frame;
    frame["command"] = "config_state";
    frame["args"][0] = body;
    d->writeWSCommand(frame);
}
