#include <time.h>
#include <algorithm>
#include <ctype.h>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <utility>
#include <functional>

#include "olcops.h"
#include "olcvalidate.h"
#include "olcservlet.h"
#include "capturebufferhandler.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "aedit.h"
#include "qedit.h"
#include "hedit.h"
#include "areahelp.h"
#include "helpmanager.h"
#include "resetops.h"
#include "wearlocation.h"
#include "olc.h"
#include "xmlroom.h"
#include "xmlarea.h"
#include "xmlmobilefactory.h"
#include "xmlobjectfactory.h"
#include "xmldocument.h"
#include "exceptionxmlerror.h"
#include "areautils.h"
#include "areaquest.h"
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

extern void help_save_ids();

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
    int opIndex;
    // Vnums of this batch's creates by their 'ref' name, for "$name" in later ops.
    std::map<DLString, int> refs;
    // Dry run: the vnums this batch's creates would take, per kind.
    std::map<DLString, std::set<int> > planned;
    // Every room the batch puts, collected up front: exits may lead to a
    // room put later in the same batch.
    std::set<int> batchRooms;
    // Quests whose steps this batch touched, checked for step pairing at the end.
    std::set<int> quests;
    // Prototypes a 'validate' op named; area vnums mean every prototype of the area.
    std::set<std::pair<DLString, int> > validate;
    Json::Value *warnings;
};

/** The parsed op, every string already KOI8. */
struct OlcOpLine {
    DLString cmdName;       // subcommand as typed
    DLString requiredName;  // 'set' only: the resolved subcommand must be exactly this
    DLString arguments;
    DLString text;
    bool hasText;

    OlcOpLine() : hasText(false) { }
};

static void op_warning(OlcOpContext &ctx, const DLString &text)
{
    ctx.warnings->append("#" + DLString(ctx.opIndex) + ": " + text);
}

static void op_journal(OlcOpContext &ctx, const DLString &kind, int vnum,
                       const DLString &cmd, const DLString &args)
{
    olc_journal(ctx.ch->getName(), OlcOps::policyName(ctx.policy), "api", kind, vnum, cmd, args);
}

static std::string::size_type capture_mark(OlcOpContext &ctx)
{
    return ctx.capture ? ctx.capture->getString().size() : 0;
}

static DLString captured(OlcOpContext &ctx, std::string::size_type mark)
{
    if (!ctx.capture)
        return DLString::emptyString;

    DLString out = DLString(ctx.capture->getString().substr(mark)).colourStrip();
    out.stripWhiteSpace();
    return out;
}

/** Result of an editor function that talks to the actor: false is a failed op
 *  in the editor's own words. */
static bool direct_result(OlcOpContext &ctx, bool ok, std::string::size_type mark,
                          Json::Value &res, DLString &error)
{
    DLString out = captured(ctx, mark);

    res["changed"] = ok;
    if (ctx.verbose && !out.empty())
        res["output"] = out;

    if (!ok)
        error = out.empty() ? DLString("Refused") : out;

    return ok;
}

static bool is_planned(OlcOpContext &ctx, const DLString &kind, int vnum)
{
    return ctx.dryRun && ctx.planned[kind].count(vnum) > 0;
}

// A whole-node put or a delete under an open editor would be undone by the
// editor's 'done' (medit/oedit commit their copy) or crash it (a deleted
// help). Editors journal as the same kind and vnum the ops use.
static bool editor_refusal(const DLString &kind, int vnum, DLString &error)
{
    for (Descriptor *d = descriptor_list; d; d = d->next)
        for (auto &h: d->handle_input) {
            OLCState::Pointer state = h.getDynamicPointer<OLCState>();
            if (!state || state->getJournalKind() != kind || state->getJournalVnum() != vnum)
                continue;

            DLString name = d->character && d->character->getPC() ? d->character->getPC()->getName() : DLString("somebody");
            error = kind + " " + DLString(vnum) + " is open in " + name + "'s editor";
            return false;
        }

    return true;
}

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
 *  base attach (no char-side state to restore), handle, commit, base detach.
 *  Room, mob and object states commit after every subcommand but 'show' (their
 *  handlers may edit without reporting it); area and quest states only after a
 *  change, since their commit has side effects of its own. */
template <typename State>
static bool run_on_state(::Pointer<State> state, OlcOpContext &ctx, OlcOpLine &line,
                         const GlobalBitvector &behaviors, Json::Value &res, DLString &error,
                         bool commitAlways = true,
                         std::function<DLString()> commitRefusal = nullptr)
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

    if (ctx.policy == OLC_POLICY_PLAYER && olc_power_field(name)) {
        error = "The player profile can't set " + name + ": numbers, flags, power and code stay with builders";
        return false;
    }

    // 'quest' attaches a quest editor that nobody would commit.
    if (name == "quest" && state->getJournalKind() == "area") {
        error = "Area quests are kind quest ops: create, set, step, delete";
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

    std::string::size_type mark = capture_mark(ctx);

    DLString refusal;

    state->OLCState::attach(ctx.ch);
    try {
        state->handle(ctx.ch->desc, &buf[0]);
        if (name != "show" && (commitAlways || state->lastChanged)) {
            if (commitRefusal)
                refusal = commitRefusal();
            if (refusal.empty())
                state->commit();
        }
    } catch (...) {
        state->OLCState::detach(ctx.ch);
        throw;
    }
    state->OLCState::detach(ctx.ch);

    // The state is a copy: dropping it uncommitted changes nothing.
    if (!refusal.empty()) {
        error = refusal;
        return false;
    }

    res["changed"] = res.isMember("changed") && res["changed"].asBool() ? true : state->lastChanged;

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

/** "$name" of an earlier create's ref becomes its vnum. */
static void expand_refs(OlcOpContext &ctx, DLString &s)
{
    if (ctx.refs.empty() || s.find('$') == DLString::npos)
        return;

    DLString out;
    DLString::size_type i = 0;

    while (i < s.size()) {
        if (s[i] == '$') {
            DLString::size_type j = i + 1;
            while (j < s.size() && (isalnum((unsigned char)s[j]) || s[j] == '_'))
                j++;

            auto r = ctx.refs.find(s.substr(i + 1, j - i - 1));
            if (j > i + 1 && r != ctx.refs.end()) {
                out += DLString(r->second);
                i = j;
                continue;
            }
        }

        out += s[i];
        i++;
    }

    s = out;
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

static bool op_bool(const Json::Value &op, const char *key)
{
    return op.isMember(key) && op[key].isBool() && op[key].asBool();
}

/** A number, a numeric string or "$ref". minValue guards the index lookups:
 *  a negative vnum reaching them crashes the server. */
static bool op_number(OlcOpContext &ctx, const Json::Value &op, const char *key, int &out,
                      DLString &error, int minValue = 1)
{
    const Json::Value &v = op[key];

    if (v.isInt())
        out = v.asInt();
    else if (v.isString() && !v.asString().empty() && v.asString()[0] == '$') {
        DLString ref = v.asString().substr(1);
        auto r = ctx.refs.find(ref);
        if (r == ctx.refs.end()) {
            error = DLString(key) + ": no create with ref '" + ref + "' earlier in this batch";
            return false;
        }
        out = r->second;
    }
    else if (v.isString() && DLString(v.asString()).isNumber())
        out = DLString(v.asString()).toInt();
    else {
        error = DLString(key) + " is required and must be a number";
        return false;
    }

    if (out < minValue) {
        error = DLString(key) + " must be at least " + DLString(minValue);
        return false;
    }

    return true;
}

static bool op_number_or(OlcOpContext &ctx, const Json::Value &op, const char *key, int &out,
                         int dflt, DLString &error, int minValue = 1)
{
    if (!op.isMember(key)) {
        out = dflt;
        return true;
    }

    return op_number(ctx, op, key, out, error, minValue);
}

static const char *textFields[] = {
    "name", "short", "long", "desc", "sound", "smell",
    "message", "speedwalk", "preposition", "title", 0
};

static bool is_text_field(const DLString &field)
{
    for (int i = 0; textFields[i]; i++)
        if (field == textFields[i])
            return true;
    return false;
}

/** Turn a 'cmd' or 'set' op into a subcommand line. */
static bool op_line(OlcOpContext &ctx, const Json::Value &op, const DLString &action,
                    OlcOpLine &line, DLString &error)
{
    line.hasText = false;

    if (action == "cmd") {
        DLString cmd;
        if (!op_string(op, "cmd", cmd, error) || !op_string(op, "text", line.text, error))
            return false;

        expand_refs(ctx, cmd);

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

    bool clear = op_bool(op, "clear");

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

    expand_refs(ctx, value);

    line.cmdName = field;
    line.requiredName = field;
    line.arguments = value;
    return true;
}

/*--------------------------------------------------------------------------
 * Reads: the save serializers' XML tree as compact JSON
 *-------------------------------------------------------------------------*/

static bool is_element(const XMLNode::Pointer &n)
{
    return n->getType() == XMLNode::XML_NODE || n->getType() == XMLNode::XML_LEAF;
}

static bool has_elements(const XMLNode::Pointer &n)
{
    for (auto &k: n->getNodeList())
        if (is_element(k))
            return true;
    return false;
}

static DLString node_text(const XMLNode::Pointer &n)
{
    DLString text;

    for (auto &k: n->getNodeList())
        if (!is_element(k))
            text += k->getCData();

    return text;
}

/** Text-only element: a string. Otherwise an object of attributes and child
 *  elements; a repeated child becomes an array, and the per-language siblings
 *  of a multi-string (<name l="ru">) fold into one {en, ru, ua} object. */
static Json::Value xml_node_json(const XMLNode::Pointer &node)
{
    const XMLNode::AttributeListType &attrs = node->getAttributes();

    if (!has_elements(node) && attrs.empty())
        return Json::Value(node_text(node));

    Json::Value obj(Json::objectValue);

    for (auto &a: attrs)
        obj[a.first] = a.second;

    DLString text = node_text(node);
    if (!text.empty())
        obj["text"] = text;

    std::set<DLString> arrays;

    for (auto &k: node->getNodeList()) {
        if (!is_element(k))
            continue;

        const DLString &name = k->getName();

        if (k->getAttributes().size() == 1 && k->hasAttribute("l") && !has_elements(k)
            && (!obj.isMember(name) || (obj[name].isObject() && !arrays.count(name))))
        {
            obj[name][k->getAttribute("l")] = node_text(k);
            continue;
        }

        Json::Value v = xml_node_json(k);

        if (!obj.isMember(name)) {
            obj[name] = v;
            continue;
        }

        if (!arrays.count(name)) {
            Json::Value first = obj[name];
            obj[name] = Json::Value(Json::arrayValue);
            obj[name].append(first);
            arrays.insert(name);
        }

        obj[name].append(v);
    }

    return obj;
}

template <typename Serializer>
static XMLNode::Pointer to_node(const Serializer &x, const char *name)
{
    XMLNode::Pointer node(NEW);
    node->setName(name);
    x.toXML(node);
    return node;
}

/** The node as an XML document, in KOI8 like every other result string:
 *  XMLDocument::save writes UTF-8 and the response converts the whole JSON. */
static DLString node_xml(XMLNode::Pointer node)
{
    XMLDocument::Pointer root(NEW);
    ostringstream os;

    root->appendChild(node);
    root->save(os);
    return utf2koi(os.str());
}

static bool get_result(OlcOpContext &ctx, const Json::Value &op, XMLNode::Pointer node,
                       const Json::Value &extra, Json::Value &res, DLString &error,
                       const char *defaultFormat = "json")
{
    DLString format = defaultFormat;
    if (!op_string(op, "format", format, error))
        return false;

    if (format == "xml") {
        res["xml"] = node_xml(node);
        return true;
    }

    if (format != "json") {
        error = "format must be json or xml";
        return false;
    }

    Json::Value data = xml_node_json(node);
    if (!data.isObject())
        data = Json::Value(Json::objectValue);

    for (auto &key: extra.getMemberNames())
        data[key] = extra[key];

    if (op.isMember("fields")) {
        const Json::Value &fields = op["fields"];
        if (!fields.isArray()) {
            error = "fields must be an array of top-level field names";
            return false;
        }

        Json::Value picked(Json::objectValue);
        for (auto &f: fields) {
            if (!f.isString()) {
                error = "fields must be an array of top-level field names";
                return false;
            }
            if (data.isMember(f.asString()))
                picked[f.asString()] = data[f.asString()];
        }
        data = picked;
    }

    res["data"] = data;
    return true;
}

/*--------------------------------------------------------------------------
 * Whole-node writes
 *-------------------------------------------------------------------------*/

/** The root element of op's xml. doc owns the tree: nodes point back at
 *  their parent, so it lives as long as the caller uses the node. */
static XMLNode::Pointer parse_xml(const Json::Value &op, XMLDocument::Pointer &doc, DLString &error)
{
    if (!op["xml"].isString()) {
        error = "xml is required for action put";
        return XMLNode::Pointer();
    }

    // toKoi checks every character has a KOI8 form; the parser itself reads
    // UTF-8 (the default prolog encoding) and decodes to KOI8.
    DLString koi;
    if (!OlcOps::toKoi(op["xml"].asString(), koi, error)) {
        error = "xml: " + error;
        return XMLNode::Pointer();
    }

    std::istringstream is(koi2utf(koi));
    doc = XMLDocument::Pointer(NEW);

    try {
        doc->load(is);
    } catch (const ExceptionXMLError &e) {
        error = DLString("xml: ") + e.what();
        return XMLNode::Pointer();
    }

    for (auto &n: doc->getNodeList())
        if (is_element(n))
            return n;

    error = "xml has no root element";
    return XMLNode::Pointer();
}

static int ndx_type(const DLString &kind)
{
    if (kind == "mob")
        return OLC_NDX_MOB;
    if (kind == "obj")
        return OLC_NDX_OBJ;
    return OLC_NDX_ROOM;
}

static bool index_exists(const DLString &kind, int vnum)
{
    if (kind == "mob")
        return get_mob_index(vnum) != 0;
    if (kind == "obj")
        return get_obj_index(vnum) != 0;
    if (kind == "room")
        return get_room_index(vnum) != 0;
    return false;
}

/** The checks of 'create <vnum>|next' in the editor's own words. */
static int create_vnum(OlcOpContext &ctx, const DLString &kind, AreaIndexData *nextArea,
                       const DLString &arg, DLString &error)
{
    std::string::size_type mark = capture_mark(ctx);
    int vnum = olc_create_vnum(ctx.ch, nextArea, arg.c_str(), ndx_type(kind),
                               ctx.dryRun ? &ctx.planned[kind] : 0);

    if (vnum < 0) {
        error = captured(ctx, mark);
        if (error.empty())
            error = "Refused";
    }

    return vnum;
}

static bool room_target_known(OlcOpContext &ctx, int target)
{
    return target > 0
           && (get_room_index(target) || is_planned(ctx, "room", target) || ctx.batchRooms.count(target));
}

/** A room from XML may only point at rooms, mobs and objects that exist or
 *  that this batch puts. */
static bool room_xml_refusal(OlcOpContext &ctx, XMLRoom &xr, int vnum, DLString &error)
{
    for (auto &e: xr.exits) {
        int target = e.second.target.getValue();
        if (target != vnum && !room_target_known(ctx, target)) {
            error = "Exit " + e.first + " leads to room " + DLString(target) + ", which doesn't exist";
            return false;
        }
    }

    for (auto &e: xr.extraExits) {
        int target = e.second.target.getValue();
        if (target != vnum && !room_target_known(ctx, target)) {
            error = "Extra exit " + e.first + " leads to room " + DLString(target) + ", which doesn't exist";
            return false;
        }
    }

    for (auto &r: xr.resets) {
        bool mob = r.command == 'M';
        bool obj = r.command == 'O' || r.command == 'P' || r.command == 'G' || r.command == 'E';

        if ((mob || obj) && (r.arg1 <= 0 || !index_exists(mob ? "mob" : "obj", r.arg1))) {
            error = DLString("A reset loads ") + (mob ? "mob " : "obj ") + DLString(r.arg1) + ", which doesn't exist";
            return false;
        }

        if (r.command == 'P' && (r.arg3 <= 0 || !get_obj_index(r.arg3))) {
            error = "A reset puts an item into obj " + DLString(r.arg3) + ", which doesn't exist";
            return false;
        }
    }

    return true;
}

static bool op_put(OlcOpContext &ctx, const DLString &kind, int vnum, const Json::Value &op,
                   Json::Value &res, DLString &error)
{
    XMLDocument::Pointer doc;
    XMLNode::Pointer node = parse_xml(op, doc, error);
    if (!node)
        return false;

    // The op's vnum wins over whatever the XML says.
    node->insertAttribute("vnum", DLString(vnum));

    bool exists = index_exists(kind, vnum);

    // A new vnum gets its rights check from create_vnum below.
    if (exists && !OLCState::can_edit(ctx.ch, vnum)) {
        error = "You can't build at vnum " + DLString(vnum);
        return false;
    }

    if (!exists && !is_planned(ctx, kind, vnum)) {
        if (create_vnum(ctx, kind, 0, DLString(vnum), error) < 0)
            return false;

        // The rest of a dry run sees the prototype this put would create.
        if (ctx.dryRun)
            ctx.planned[kind].insert(vnum);
    }

    if (kind == "mob") {
        OLCStateMobile::Pointer state(NEW, vnum);
        state->mob.fromXML(node);
        if (ctx.dryRun)
            return true;
        state->commit();
    }
    else if (kind == "obj") {
        OLCStateObject::Pointer state(NEW, vnum);
        state->obj.fromXML(node);
        if (ctx.dryRun)
            return true;
        // A runtime counter, not in the XML: commit would zero it.
        if (exists)
            state->obj.reset_num = get_obj_index(vnum)->reset_num;
        state->commit();
    }
    else {
        XMLRoom xr;
        xr.fromXML(node);

        if (!room_xml_refusal(ctx, xr, vnum, error))
            return false;

        if (ctx.dryRun)
            return true;

        if (exists) {
            // In place: characters, objects, other rooms' exits and saved
            // rooms all hold this prototype and its instance.
            OLCStateRoom::replaceRoom(get_room_index(vnum), xr.compat(vnum));
        } else {
            RoomIndexData *pRoom = xr.compat(vnum);
            OLCStateRoom::registerRoom(pRoom);
            // Boot resolves every exit after the whole world loads; this room
            // joins a world that is already there. Rooms later in the batch
            // are resolved again when it ends.
            OLCStateRoom::resolveInstance(pRoom);
        }
    }

    OLCState::get_vnum_area(vnum)->changed = true;
    op_journal(ctx, kind, vnum, "put", exists ? "replace" : "new");
    res["changed"] = true;
    return true;
}

/*--------------------------------------------------------------------------
 * Prototype kinds: room, mob, obj
 *-------------------------------------------------------------------------*/

/** 'area': an area vnum, for 'next' and for a quest. 0 when absent. */
static bool op_area(OlcOpContext &ctx, const Json::Value &op, AreaIndexData *&pArea, DLString &error)
{
    pArea = 0;

    if (!op.isMember("area"))
        return true;

    int avnum;
    if (!op_number(ctx, op, "area", avnum, error, 0))
        return false;

    pArea = AreaUtils::getByVnum(avnum);
    if (!pArea) {
        error = "Area vnum " + DLString(avnum) + " not found";
        return false;
    }

    if (!OLCState::can_edit(ctx.ch, pArea)) {
        error = "You can't build in area " + DLString(avnum);
        return false;
    }

    return true;
}

/** 'vnum' of a create: a number or "next" with 'area'. Fills arg for olc_create_vnum. */
static bool op_create_arg(OlcOpContext &ctx, const Json::Value &op, DLString &arg,
                          AreaIndexData *&pArea, DLString &error)
{
    if (!op_area(ctx, op, pArea, error))
        return false;

    if (op["vnum"].isString() && op["vnum"].asString() == "next") {
        if (!pArea) {
            error = "vnum next needs an area (the area's vnum)";
            return false;
        }
        arg = "next";
        return true;
    }

    int vnum;
    if (!op_number(ctx, op, "vnum", vnum, error))
        return false;

    if (pArea && (vnum < pArea->min_vnum || vnum > pArea->max_vnum)) {
        error = "vnum " + DLString(vnum) + " is outside area " + DLString(pArea->vnum)
                + " (" + DLString(pArea->min_vnum) + "-" + DLString(pArea->max_vnum) + ")";
        return false;
    }

    arg = DLString(vnum);
    return true;
}

static bool op_create(OlcOpContext &ctx, const DLString &kind, const Json::Value &op,
                      Json::Value &res, int &vnum, DLString &error)
{
    DLString arg, ref;
    AreaIndexData *pArea;

    if (!op_create_arg(ctx, op, arg, pArea, error) || !op_string(op, "ref", ref, error))
        return false;

    if (!ref.empty() && ctx.refs.count(ref)) {
        error = "ref '" + ref + "' is already used in this batch";
        return false;
    }

    vnum = create_vnum(ctx, kind, pArea, arg, error);
    if (vnum < 0)
        return false;

    res["vnum"] = vnum;
    if (!ref.empty())
        ctx.refs[ref] = vnum;

    if (ctx.dryRun) {
        ctx.planned[kind].insert(vnum);
        return true;
    }

    if (kind == "room") {
        OLCStateRoom::createRoom(vnum);
    }
    else if (kind == "mob") {
        OLCStateMobile::Pointer state(NEW, vnum);
        state->commit();
    }
    else {
        OLCStateObject::Pointer state(NEW, vnum);
        state->commit();
    }

    OLCState::get_vnum_area(vnum)->changed = true;
    op_journal(ctx, kind, vnum, "create", arg);
    res["changed"] = true;
    return true;
}

/** How many resets load this mob or object, for a delete warning. */
static int count_resets_of(const DLString &kind, int vnum)
{
    int cnt = 0;

    for (auto &r: roomIndexMap)
        for (auto &reset: r.second->resets) {
            if (kind == "mob" && reset->command == 'M' && reset->arg1 == vnum)
                cnt++;
            else if (kind == "obj" && reset->command != 'M' && reset->command != 'R'
                     && (reset->arg1 == vnum || (reset->command == 'P' && reset->arg3 == vnum)))
                cnt++;
        }

    return cnt;
}

static int count_quest_steps_on(const DLString &type, int vnum)
{
    DLString v(vnum);
    int cnt = 0;

    for (auto &q: areaQuests)
        for (auto &s: q.second->steps)
            if ((s->beginType == type && s->beginValue == v) || (s->endType == type && s->endValue == v))
                cnt++;

    return cnt;
}

/** delete with 'exit', 'eexit' or 'ed': the editor's own delete subcommand. */
static bool op_delete_part(const DLString &kind, const Json::Value &op, OlcOpLine &line, DLString &error)
{
    DLString exitDir, eexit, ed;

    if (!op_string(op, "exit", exitDir, error) || !op_string(op, "eexit", eexit, error)
        || !op_string(op, "ed", ed, error))
        return false;

    int parts = !exitDir.empty() + !eexit.empty() + !ed.empty();
    if (parts > 1) {
        error = "delete takes one of exit, eexit, ed";
        return false;
    }

    if (!exitDir.empty()) {
        if (kind != "room") {
            error = "exit is for rooms";
            return false;
        }

        if (!olc_is_direction(exitDir)) {
            error = "exit must be north, east, south, west, up or down";
            return false;
        }

        // 'delete' takes the exit away on both sides, 'unlink' on this side only.
        line.cmdName = exitDir;
        line.requiredName = exitDir;
        line.arguments = op_bool(op, "unlink") ? "unlink" : "delete";
        return true;
    }

    if (!eexit.empty()) {
        if (kind != "room") {
            error = "eexit is for rooms";
            return false;
        }
        line.cmdName = line.requiredName = "eexit";
        line.arguments = "delete " + eexit;
        return true;
    }

    if (!ed.empty()) {
        if (kind == "mob") {
            error = "mobiles have no extra descriptions";
            return false;
        }
        line.cmdName = line.requiredName = "ed";
        line.arguments = "delete " + ed;
        return true;
    }

    return true;
}

static bool op_delete(OlcOpContext &ctx, const DLString &kind, int vnum, const Json::Value &op,
                      Json::Value &res, DLString &error)
{
    std::string::size_type mark = capture_mark(ctx);

    if (kind == "room") {
        if (!OLCState::can_edit(ctx.ch, vnum)) {
            error = "You can't build in room " + DLString(vnum);
            return false;
        }

        RoomIndexData *pRoom = get_room_index(vnum);
        error = OLCStateRoom::deleteRefusal(pRoom);
        if (!error.empty())
            return false;

        int steps = count_quest_steps_on("room", vnum);
        if (steps > 0)
            op_warning(ctx, "room " + DLString(vnum) + " is a target of " + DLString(steps) + " area quest step(s)");

        if ((pRoom->room && pRoom->room->wrapper) || !pRoom->behaviors.empty() || pRoom->behavior)
            op_warning(ctx, "room " + DLString(vnum) + " has Fenia triggers or behaviors: scripts that name it lose it");

        if (ctx.dryRun)
            return true;

        OLCStateRoom::unregisterRoom(pRoom);
        op_journal(ctx, kind, vnum, "delete", "");
        res["changed"] = true;
        return true;
    }

    int resets = count_resets_of(kind, vnum);
    if (resets > 0)
        op_warning(ctx, kind + " " + DLString(vnum) + " is still loaded by " + DLString(resets) + " reset(s)");

    int steps = count_quest_steps_on(kind, vnum);
    if (steps > 0)
        op_warning(ctx, kind + " " + DLString(vnum) + " is a target of " + DLString(steps) + " area quest step(s)");

    if (ctx.dryRun)
        return true;

    DLString arg(vnum);
    bool ok;
    if (kind == "mob")
        ok = OLCStateMobile::markDeleted(ctx.ch, arg.c_str()) != 0;
    else
        ok = OLCStateObject::markDeleted(ctx.ch, arg.c_str()) != 0;

    if (ok)
        op_journal(ctx, kind, vnum, "delete", "");

    return direct_result(ctx, ok, mark, res, error);
}

static bool prototype_op(OlcOpContext &ctx, const DLString &kind, const DLString &action,
                         int vnum, const Json::Value &op, Json::Value &res, DLString &error)
{
    if ((action == "put" || action == "delete") && !editor_refusal(kind, vnum, error))
        return false;

    if (action == "put")
        return op_put(ctx, kind, vnum, op, res, error);

    OlcOpLine line;
    bool part = false;

    if (action == "delete") {
        if (!op_delete_part(kind, op, line, error))
            return false;
        part = !line.cmdName.empty();
    }
    else if (action == "cmd" || action == "set") {
        if (!op_line(ctx, op, action, line, error))
            return false;
    }

    // Ops on a vnum this dry run would create: parsed, nothing to resolve yet.
    if (is_planned(ctx, kind, vnum))
        return true;

    if (!index_exists(kind, vnum)) {
        error = kind + " vnum " + DLString(vnum) + " not found";
        return false;
    }

    if (action == "get") {
        Json::Value extra(Json::objectValue);
        extra["vnum"] = vnum;

        if (kind == "room") {
            RoomIndexData *pRoom = get_room_index(vnum);
            XMLRoom xr;
            xr.init(pRoom);
            extra["area"] = pRoom->areaIndex->vnum;
            // Phase 0 answered a room get with xml, and older dl_redit
            // clients read only that: json is opt-in via fields or format.
            const char *defaultFormat = op.isMember("fields") ? "json" : "xml";
            return get_result(ctx, op, to_node(xr, "room"), extra, res, error, defaultFormat);
        }

        if (kind == "mob") {
            MOB_INDEX_DATA *pMob = get_mob_index(vnum);
            XMLMobileFactory f;
            f.init(pMob);
            if (pMob->area)
                extra["area"] = pMob->area->vnum;
            return get_result(ctx, op, to_node(f, "mobile"), extra, res, error);
        }

        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        XMLObjectFactory f;
        f.init(pObj);
        if (pObj->area)
            extra["area"] = pObj->area->vnum;
        return get_result(ctx, op, to_node(f, "object"), extra, res, error);
    }

    if (action == "delete" && !part)
        return op_delete(ctx, kind, vnum, op, res, error);

    if (kind == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        Room *room = get_room_instance(vnum);
        if (!room) {
            error = "Room vnum " + DLString(vnum) + " has no instance";
            return false;
        }

        // Room subcommands edit the prototype of the room the actor stands in.
        ctx.ch->in_room = room;

        OLCStateRoom::Pointer sr(NEW);
        sr->room.setValue(vnum);
        // No original room: a throwing subcommand's detach returns early
        // instead of moving the actor.
        sr->originalRoom.setValue(0);
        return run_on_state(sr, ctx, line, pRoom->behaviors, res, error);
    }

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        error = "Limbo room instance not found";
        return false;
    }
    ctx.ch->in_room = limbo;

    if (kind == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (!pMob->area) {
            error = "Mobile vnum " + DLString(vnum) + " has no area";
            return false;
        }

        OLCStateMobile::Pointer state(NEW, pMob);
        return run_on_state(state, ctx, line, pMob->behaviors, res, error);
    }

    OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
    if (!pObj->area) {
        error = "Object vnum " + DLString(vnum) + " has no area";
        return false;
    }

    OLCStateObject::Pointer state(NEW, pObj);
    return run_on_state(state, ctx, line, pObj->behaviors, res, error);
}

/*--------------------------------------------------------------------------
 * Resets, addressed by room vnum
 *-------------------------------------------------------------------------*/

static bool reset_op(OlcOpContext &ctx, const DLString &action, int vnum, const Json::Value &op,
                     Json::Value &res, DLString &error)
{
    if (action == "get") {
        if (is_planned(ctx, "room", vnum))
            return true;

        RoomIndexData *pRoom = get_room_index(vnum);
        if (!pRoom) {
            error = "Room vnum " + DLString(vnum) + " not found";
            return false;
        }

        XMLRoom xr;
        xr.init(pRoom);
        Json::Value fields(Json::arrayValue);
        fields.append("resets");
        Json::Value getOp = op;
        if (!getOp.isMember("fields"))
            getOp["fields"] = fields;

        Json::Value extra(Json::objectValue);
        extra["vnum"] = vnum;
        return get_result(ctx, getOp, to_node(xr, "room"), extra, res, error);
    }

    int number = 0, after = -1;
    DLString journalCmd, journalArgs;

    if (action == "delete" || action == "set") {
        if (!op_number(ctx, op, "number", number, error))
            return false;
    }

    if (action == "add" && !op_number_or(ctx, op, "after", after, -1, error, 0))
        return false;

    int mobVnum = 0, objVnum = 0, maxArea = 1, maxRoom = 1, container = 0, limit = 1, count = 1;
    DLString to = "room", field, value;

    if (action == "add") {
        bool hasMob = op.isMember("mob"), hasObj = op.isMember("obj");

        if (hasMob == hasObj) {
            error = "add takes mob or obj";
            return false;
        }

        if (hasMob) {
            if (!op_number(ctx, op, "mob", mobVnum, error)
                || !op_number_or(ctx, op, "max", maxArea, 1, error)
                || !op_number_or(ctx, op, "maxRoom", maxRoom, 1, error))
                return false;

            if (!get_mob_index(mobVnum) && !is_planned(ctx, "mob", mobVnum)) {
                error = "mob " + DLString(mobVnum) + " not found";
                return false;
            }

            journalCmd = "add mob";
            journalArgs = DLString(mobVnum) + " " + DLString(maxArea) + " " + DLString(maxRoom);
        } else {
            if (!op_number(ctx, op, "obj", objVnum, error) || !op_string(op, "to", to, error))
                return false;

            if (!get_obj_index(objVnum) && !is_planned(ctx, "obj", objVnum)) {
                error = "obj " + DLString(objVnum) + " not found";
                return false;
            }

            if (to == "inside") {
                // limit 0 = no limit, as in the reset loop.
                if (!op_number(ctx, op, "container", container, error)
                    || !op_number_or(ctx, op, "limit", limit, 1, error, 0)
                    || !op_number_or(ctx, op, "count", count, 1, error))
                    return false;

                OBJ_INDEX_DATA *pContainer = get_obj_index(container);
                if (!pContainer && !is_planned(ctx, "obj", container)) {
                    error = "container " + DLString(container) + " not found";
                    return false;
                }
                if (pContainer && pContainer->item_type != ITEM_CONTAINER) {
                    error = "obj " + DLString(container) + " is not a container";
                    return false;
                }
            }
            else if (to != "room" && !wearlocationManager->findExisting(to)) {
                error = "unknown wearloc '" + to + "', see olchelp wearloc";
                return false;
            }

            journalCmd = "add obj";
            journalArgs = DLString(objVnum) + " " + to;
        }
    }
    else if (action == "set") {
        if (!op_string(op, "field", field, error) || !op_string(op, "value", value, error))
            return false;

        if (field != "flag" && field != "rand" && field != "tier" && field != "wearloc") {
            error = "field must be flag, rand, tier or wearloc";
            return false;
        }

        if (value.empty()) {
            error = "value is required for action set";
            return false;
        }

        if (field == "tier" && !value.isNumber()) {
            error = "tier must be a number";
            return false;
        }

        if (field == "wearloc" && !wearlocationManager->findExisting(value)) {
            error = "unknown wearloc '" + value + "', see olchelp wearloc";
            return false;
        }

        journalCmd = DLString(number) + " " + field;
        journalArgs = value;
    }
    else if (action == "delete") {
        journalCmd = DLString(number) + " delete";
    }
    else if (action == "randomize") {
        journalCmd = "randomize exits";
    }
    else {
        error = "reset action must be get, add, set, delete or randomize";
        return false;
    }

    if (is_planned(ctx, "room", vnum))
        return true;

    RoomIndexData *pRoom = get_room_index(vnum);
    if (!pRoom || !pRoom->room) {
        error = "Room vnum " + DLString(vnum) + " not found";
        return false;
    }

    if (!OLCState::can_edit(ctx.ch, vnum)) {
        error = "You can't build in room " + DLString(vnum);
        return false;
    }

    if (number > (int)pRoom->resets.size()) {
        error = "Room " + DLString(vnum) + " has " + DLString((int)pRoom->resets.size()) + " reset(s)";
        return false;
    }

    if (ctx.dryRun)
        return true;

    // The reset handlers answer the room the actor stands in.
    ctx.ch->in_room = pRoom->room;

    std::string::size_type mark = capture_mark(ctx);
    bool ok;

    if (action == "add" && mobVnum > 0)
        ok = reset_add_mob(ctx.ch, pRoom, after, mobVnum, maxArea, maxRoom);
    else if (action == "add" && to == "room")
        ok = reset_add_obj_room(ctx.ch, pRoom, after, objVnum);
    else if (action == "add" && to == "inside")
        ok = reset_add_obj_inside(ctx.ch, pRoom, after, objVnum, container, limit, count);
    else if (action == "add")
        ok = reset_add_obj_wear(ctx.ch, pRoom, after, objVnum, to);
    else if (action == "delete")
        ok = reset_delete(ctx.ch, pRoom, number);
    else if (action == "randomize")
        ok = reset_randomize_exits(ctx.ch, pRoom);
    else if (field == "flag")
        ok = reset_toggle_flags(ctx.ch, pRoom, number, value);
    else if (field == "rand")
        ok = reset_set_rand(ctx.ch, pRoom, number, value);
    else if (field == "tier")
        ok = reset_set_tier(ctx.ch, pRoom, number, value.toInt());
    else
        ok = reset_set_wearloc(ctx.ch, pRoom, number, value);

    if (ok)
        op_journal(ctx, "reset", vnum, journalCmd, journalArgs);

    return direct_result(ctx, ok, mark, res, error);
}

/*--------------------------------------------------------------------------
 * Area header
 *-------------------------------------------------------------------------*/

/** The whole <areadata> onto the existing header through the aedit state,
 *  the fields XMLAreaHeader::compat reads at boot. The file name stays. */
static bool area_put(OlcOpContext &ctx, AreaIndexData *pArea, const Json::Value &op,
                     Json::Value &res, DLString &error)
{
    XMLDocument::Pointer doc;
    XMLNode::Pointer node = parse_xml(op, doc, error);
    if (!node)
        return false;

    XMLAreaHeader header;
    header.fromXML(node);

    OLCStateArea::Pointer state(NEW, pArea);
    state->name = header.name;
    if (!header.credits.empty())
        state->name[LANG_EN] = header.credits;
    state->altname = header.altname;
    state->authors.setValue(header.authors.getValue());
    state->translator.setValue(header.translator.getValue());
    state->speedwalk = header.speedwalk;
    state->resetMessage = header.resetMessage;
    state->preposition = header.preposition;
    state->security = header.security.getValue();
    state->min_vnum = header.vnumLow.getValue();
    state->max_vnum = header.vnumHigh.getValue();
    state->low_range = header.levelLow.getValue();
    state->high_range = header.levelHigh.getValue();
    state->area_flag.setValue(header.flags.getValue());
    // Applied below and only when it differs: commit would rebuild the
    // behavior from its string and drop its runtime state on every put.
    state->behavior.setValue(DLString::emptyString);

    int lower = state->min_vnum.getValue(), upper = state->max_vnum.getValue();
    if (!state->checkOverlap(lower, upper)) {
        error = "Vnums " + DLString(lower) + "-" + DLString(upper) + " overlap another area";
        return false;
    }

    DLString refusal = state->commitRefusal();
    if (!refusal.empty()) {
        error = refusal;
        return false;
    }

    if (ctx.dryRun)
        return true;

    XMLAreaHeader current;
    current.init(pArea);
    XMLNode::Pointer behavior = header.behavior.getNode();
    bool behaviorChanged = !olc_same_xml(current.behavior.getNode(), behavior);

    state->commit();

    if (behaviorChanged) {
        if (pArea->behavior)
            pArea->behavior->unsetArea();
        pArea->behavior.clear();

        if (behavior) {
            pArea->behavior.fromXML(behavior);
            if (pArea->behavior)
                pArea->behavior->setArea(pArea);
        }
    }

    op_journal(ctx, "area", pArea->vnum, "put", "replace");
    res["changed"] = true;
    return true;
}

static bool area_op(OlcOpContext &ctx, const DLString &action, int vnum, const Json::Value &op,
                    Json::Value &res, DLString &error)
{
    if (action != "get" && action != "cmd" && action != "set" && action != "put") {
        error = "area action must be get, cmd, set or put";
        return false;
    }

    OlcOpLine line;
    if (action != "get" && action != "put" && !op_line(ctx, op, action, line, error))
        return false;

    AreaIndexData *pArea = AreaUtils::getByVnum(vnum);
    if (!pArea) {
        error = "Area vnum " + DLString(vnum) + " not found";
        return false;
    }

    if (!OLCState::can_edit(ctx.ch, pArea)) {
        error = "You can't build in area " + DLString(vnum);
        return false;
    }

    if (action == "get") {
        XMLAreaHeader header;
        header.init(pArea);

        Json::Value extra(Json::objectValue);
        extra["vnum"] = vnum;
        extra["file"] = pArea->area_file->file_name;
        extra["changed"] = pArea->changed;
        Json::Value quests(Json::arrayValue);
        for (auto &q: pArea->quests)
            quests.append(q->vnum.getValue());
        extra["quests"] = quests;
        return get_result(ctx, op, to_node(header, "areadata"), extra, res, error);
    }

    if (action == "put")
        return editor_refusal("area", vnum, error) && area_put(ctx, pArea, op, res, error);

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        error = "Limbo room instance not found";
        return false;
    }
    ctx.ch->in_room = limbo;

    OLCStateArea::Pointer state(NEW, pArea);
    CommandBase::Pointer cmd = state->findCommand(ctx.ch, line.cmdName);
    bool repop = cmd && cmd->getName() == "reset";

    if (!run_on_state(state, ctx, line, GlobalBitvector(), res, error, false,
                      [state]() { return state->commitRefusal(); }))
        return false;

    // A repop edits nothing, so the state never journals it.
    if (repop && !ctx.dryRun)
        op_journal(ctx, "area", vnum, "reset", "");

    return true;
}

/*--------------------------------------------------------------------------
 * Area quests
 *-------------------------------------------------------------------------*/

static bool quest_create(OlcOpContext &ctx, const Json::Value &op, Json::Value &res,
                         int &vnum, DLString &error)
{
    DLString arg, ref;
    AreaIndexData *pArea;

    if (!op_create_arg(ctx, op, arg, pArea, error) || !op_string(op, "ref", ref, error))
        return false;

    if (!ref.empty() && ctx.refs.count(ref)) {
        error = "ref '" + ref + "' is already used in this batch";
        return false;
    }

    if (!pArea) {
        pArea = OLCState::get_vnum_area(arg.toInt());
        if (!pArea) {
            error = "Quest vnum " + arg + " is in no area";
            return false;
        }
        if (!OLCState::can_edit(ctx.ch, pArea)) {
            error = "You can't build in area " + DLString(pArea->vnum);
            return false;
        }
    }

    std::set<int> &planned = ctx.planned["quest"];

    if (ctx.dryRun) {
        if (arg == "next") {
            vnum = OLCStateArea::nextQuestVnum(pArea, &planned);
            if (vnum < 0) {
                error = "No free quest vnum left in area " + DLString(pArea->vnum);
                return false;
            }
        } else {
            vnum = arg.toInt();
            if (areaQuests.find(vnum) != areaQuests.end() || planned.count(vnum)) {
                error = "Quest " + DLString(vnum) + " already exists";
                return false;
            }
        }

        planned.insert(vnum);
    } else {
        std::string::size_type mark = capture_mark(ctx);
        AreaQuest *q = OLCStateArea::questCreate(ctx.ch, pArea, arg == "next" ? DLString::emptyString : arg);

        if (!direct_result(ctx, q != 0, mark, res, error))
            return false;

        vnum = q->vnum.getValue();
        op_journal(ctx, "quest", vnum, "create", arg);
        ctx.quests.insert(vnum);
    }

    res["vnum"] = vnum;
    if (!ref.empty())
        ctx.refs[ref] = vnum;
    return true;
}

static bool step_end_lines(OlcOpContext &ctx, const Json::Value &op, const char *which,
                           const DLString &stepArg, std::vector<OlcOpLine> &lines, DLString &error)
{
    if (!op.isMember(which))
        return true;

    const Json::Value &part = op[which];
    if (!part.isObject()) {
        error = DLString(which) + " must be an object {type, vnum, trigger}";
        return false;
    }

    DLString type, trigger;
    int target = 0;

    if (!op_string(part, "type", type, error) || !op_string(part, "trigger", trigger, error))
        return false;

    if (part.isMember("vnum") && !op_number(ctx, part, "vnum", target, error))
        return false;

    DLString prefix = "step " + stepArg + " " + which + " ";
    OlcOpLine line;
    line.cmdName = line.requiredName = "step";

    // Type first: changing it drops the old trigger, which the editor then
    // checks the new trigger name against.
    if (!type.empty()) {
        line.arguments = stepArg + " " + which + " type " + type;
        lines.push_back(line);
    }
    if (target > 0) {
        line.arguments = stepArg + " " + which + " vnum " + DLString(target);
        lines.push_back(line);
    }
    if (!trigger.empty()) {
        line.arguments = stepArg + " " + which + " trig " + trigger;
        lines.push_back(line);
    }

    return true;
}

/** One step's edits as qedit 'step' lines. "step": "add" appends a step first. */
static bool step_lines(OlcOpContext &ctx, const Json::Value &op, const DLString &stepArg,
                       std::vector<OlcOpLine> &lines, DLString &error)
{
    OlcOpLine line;
    line.cmdName = line.requiredName = "step";

    if (op_bool(op, "delete")) {
        line.arguments = stepArg + " del";
        lines.push_back(line);
        return true;
    }

    if (!step_end_lines(ctx, op, "begin", stepArg, lines, error)
        || !step_end_lines(ctx, op, "end", stepArg, lines, error))
        return false;

    if (op.isMember("info")) {
        const Json::Value &info = op["info"];
        if (!info.isObject()) {
            error = "info must be an object {en, ru, ua}";
            return false;
        }

        static const char *langs[] = { "en", "ru", "ua", 0 };
        for (int i = 0; langs[i]; i++) {
            if (!info.isMember(langs[i]))
                continue;

            OlcOpLine paste = line;
            if (!op_string(info, langs[i], paste.text, error))
                return false;
            paste.arguments = stepArg + " " + langs[i] + " paste";
            paste.hasText = true;
            lines.push_back(paste);
        }
    }

    static const char *rewards[] = { "qp", "gold", "exp", "item", 0 };
    for (int i = 0; rewards[i]; i++) {
        if (!op.isMember(rewards[i]))
            continue;

        int amount;
        if (!op_number(ctx, op, rewards[i], amount, error, 0))
            return false;
        line.arguments = stepArg + " " + rewards[i] + " " + DLString(amount);
        lines.push_back(line);
    }

    return true;
}

static bool quest_step(OlcOpContext &ctx, AreaQuest *q, int vnum, const Json::Value &op,
                       Json::Value &res, DLString &error)
{
    DLString stepArg;
    bool add = op["step"].isString() && op["step"].asString() == "add";

    if (!add) {
        int step;
        if (!op_number(ctx, op, "step", step, error, 0))
            return false;
        if (q && step >= (int)q->steps.size()) {
            error = "Quest " + DLString(vnum) + " has " + DLString((int)q->steps.size()) + " step(s), counted from 0";
            return false;
        }
        stepArg = DLString(step);
    }

    // Lines are built against the final step number; for an added step that
    // is only known after 'step add' runs.
    std::vector<OlcOpLine> lines;
    if (!step_lines(ctx, op, add ? DLString("0") : stepArg, lines, error))
        return false;

    if (!q)
        return true;

    OLCStateAreaQuest::Pointer state(NEW, q);

    if (add) {
        OlcOpLine addLine;
        addLine.cmdName = addLine.requiredName = "step";
        addLine.arguments = "add";
        if (!run_on_state(state, ctx, addLine, GlobalBitvector(), res, error, false))
            return false;

        if (ctx.dryRun)
            return true;

        // From here on the quest has changed, even if a later line fails.
        ctx.quests.insert(vnum);
        stepArg = DLString((int)q->steps.size() - 1);
        lines.clear();
        if (!step_lines(ctx, op, stepArg, lines, error))
            return false;
    }

    res["step"] = stepArg.toInt();

    for (auto &line: lines) {
        OLCStateAreaQuest::Pointer s(NEW, q);
        bool ok = run_on_state(s, ctx, line, GlobalBitvector(), res, error, false);
        // Earlier lines of this op stay applied: check the pairing either way.
        if (!ctx.dryRun && s->lastChanged)
            ctx.quests.insert(vnum);
        if (!ok)
            return false;
    }

    return true;
}

/** The whole <quest> node onto quest vnum, created when it doesn't exist.
 *  An existing AreaQuest object stays: its Fenia wrapper hangs on it. */
static bool quest_put(OlcOpContext &ctx, int vnum, const Json::Value &op,
                      Json::Value &res, DLString &error)
{
    XMLDocument::Pointer doc;
    XMLNode::Pointer node = parse_xml(op, doc, error);
    if (!node)
        return false;

    AreaQuest::Pointer fresh(NEW);
    fresh->fromXML(node);

    AreaQuest *q = 0;
    auto qi = areaQuests.find(vnum);
    if (qi != areaQuests.end())
        q = qi->second;

    AreaIndexData *pArea = 0;
    if (q)
        pArea = q->pAreaIndex;
    else {
        if (!op_area(ctx, op, pArea, error))
            return false;
        if (!pArea)
            pArea = OLCState::get_vnum_area(vnum);
    }

    if (!pArea) {
        error = "Quest vnum " + DLString(vnum) + " is in no area";
        return false;
    }

    if (!OLCState::can_edit(ctx.ch, pArea)) {
        error = "You can't build in area " + DLString(pArea->vnum);
        return false;
    }

    if (!q && (vnum < pArea->min_vnum || vnum > pArea->max_vnum)) {
        error = "Quest vnum " + DLString(vnum) + " is outside the vnums of area " + DLString(pArea->vnum);
        return false;
    }

    if (ctx.dryRun) {
        if (!q)
            ctx.planned["quest"].insert(vnum);
        return true;
    }

    bool exists = q != 0;
    if (!q) {
        std::string::size_type mark = capture_mark(ctx);
        q = OLCStateArea::questCreate(ctx.ch, pArea, DLString(vnum));
        if (!q) {
            error = captured(ctx, mark);
            if (error.empty())
                error = "Refused";
            return false;
        }
    }

    q->title = fresh->title;
    q->description = fresh->description;
    q->minLevel = fresh->minLevel;
    q->maxLevel = fresh->maxLevel;
    q->limitPerLife = fresh->limitPerLife;
    q->oncePerDay = fresh->oncePerDay;
    q->steps = fresh->steps;
    q->flags = fresh->flags;
    q->prereq = fresh->prereq;
    q->align = fresh->align;
    q->hometowns = fresh->hometowns;
    q->classes = fresh->classes;
    q->lootVnums = fresh->lootVnums;

    pArea->changed = true;
    ctx.quests.insert(vnum);
    op_journal(ctx, "quest", vnum, "put", exists ? "replace" : "new");
    res["changed"] = true;
    return true;
}

static bool quest_op(OlcOpContext &ctx, const DLString &action, const Json::Value &op,
                     Json::Value &res, int &vnum, DLString &error)
{
    if (action == "create")
        return quest_create(ctx, op, res, vnum, error);

    if (!op_number(ctx, op, "vnum", vnum, error))
        return false;
    res["vnum"] = vnum;

    if ((action == "put" || action == "delete") && !editor_refusal("quest", vnum, error))
        return false;

    if (action == "put")
        return quest_put(ctx, vnum, op, res, error);

    if (action != "get" && action != "cmd" && action != "set" && action != "step" && action != "delete") {
        error = "quest action must be get, create, cmd, set, step, put or delete";
        return false;
    }

    OlcOpLine line;
    if ((action == "cmd" || action == "set") && !op_line(ctx, op, action, line, error))
        return false;

    AreaQuest *q = 0;
    auto qi = areaQuests.find(vnum);
    if (qi != areaQuests.end())
        q = qi->second;

    if (!q && !is_planned(ctx, "quest", vnum)) {
        error = "Quest vnum " + DLString(vnum) + " not found";
        return false;
    }

    if (q && !OLCState::can_edit(ctx.ch, q->pAreaIndex)) {
        error = "You can't build in area " + DLString(q->pAreaIndex->vnum);
        return false;
    }

    if (action == "step")
        return quest_step(ctx, q, vnum, op, res, error);

    if (!q)
        return true;

    if (action == "get") {
        Json::Value extra(Json::objectValue);
        extra["vnum"] = vnum;
        extra["area"] = q->pAreaIndex->vnum;
        return get_result(ctx, op, to_node(*q, "quest"), extra, res, error);
    }

    if (action == "delete") {
        op_warning(ctx, "the Fenia handlers of quest " + DLString(vnum)
                        + " stay: annul them first, 'cs search " + DLString(vnum) + "'");
        if (ctx.dryRun)
            return true;

        std::string::size_type mark = capture_mark(ctx);
        bool ok = OLCStateArea::questDelete(ctx.ch, q->pAreaIndex, DLString(vnum));
        if (ok)
            op_journal(ctx, "quest", vnum, "delete", "");
        return direct_result(ctx, ok, mark, res, error);
    }

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        error = "Limbo room instance not found";
        return false;
    }
    ctx.ch->in_room = limbo;

    OLCStateAreaQuest::Pointer state(NEW, q);
    if (!run_on_state(state, ctx, line, GlobalBitvector(), res, error, false))
        return false;

    ctx.quests.insert(vnum);
    return true;
}

/** Steps of a quest this batch touched that break the pairing rule. */
static void quest_pairing_warnings(OlcOpContext &ctx)
{
    for (int vnum: ctx.quests) {
        auto qi = areaQuests.find(vnum);
        if (qi == areaQuests.end())
            continue;

        AreaQuest *q = qi->second;

        for (int s = 0; s + 1 < (int)q->steps.size(); s++) {
            int pairing = qedit_step_pairing(q, s);
            auto &step = q->steps[s];
            auto &next = q->steps[s + 1];
            DLString where = "quest " + DLString(vnum) + " step " + DLString(s) + " -> " + DLString(s + 1);

            if (pairing & STEP_PAIR_TARGET)
                ctx.warnings->append(where + ": ends on " + step->endType + " " + step->endValue
                                     + ", the next step begins on " + next->beginType + " " + next->beginValue);

            if (pairing & STEP_PAIR_TRIGGER)
                ctx.warnings->append(where + ": ends with " + step->endTrigger
                                     + ", the next step begins with " + next->beginTrigger
                                     + " (an onX end pairs with a postX begin)");
        }
    }
}

/*--------------------------------------------------------------------------
 * Validators (olcvalidate.h)
 *-------------------------------------------------------------------------*/

/** 'validate': names a prototype, or an area for all of its prototypes, for the
 *  validator chain at the end of the batch. Reads only: no journal line. */
static bool validate_op(OlcOpContext &ctx, const DLString &kind, const Json::Value &op,
                        Json::Value &res, int &vnum, DLString &error)
{
    if (kind != "room" && kind != "mob" && kind != "obj" && kind != "area") {
        error = "validate is for room, mob, obj and area";
        return false;
    }

    if (!op_number(ctx, op, "vnum", vnum, error, kind == "area" ? 0 : 1))
        return false;

    res["vnum"] = vnum;

    bool exists = kind == "area" ? AreaUtils::getByVnum(vnum) != 0 : index_exists(kind, vnum);
    if (!exists) {
        error = kind + " " + DLString(vnum) + " not found";
        return false;
    }

    ctx.validate.insert(std::make_pair(kind, vnum));
    res["changed"] = false;
    return true;
}

static int severity_rank(const DLString &severity)
{
    if (severity == "error")
        return 0;
    if (severity == "warning")
        return 1;
    return 2;
}

/** Runs the chain on every prototype the batch changed and every one a
 *  'validate' op named, applies the profile and fills findingsSummary, and
 *  findings[]: what 'validate' named always, the rest of an edit batch only
 *  with verbose or strict (legacy prototypes carry a backlog of warnings that
 *  would ride along with every typo fix). Returns the number of errors. */
static int run_validators(OlcOpContext &ctx, const std::set<std::pair<DLString, int> > &changed,
                          bool strict, Json::Value &result)
{
    std::set<std::pair<DLString, int> > targets = ctx.validate;
    targets.insert(changed.begin(), changed.end());

    OlcFindings findings;
    std::set<int> areasDone;

    for (auto &t: targets) {
        const DLString &kind = t.first;
        int vnum = t.second;
        size_t from = findings.size();

        // A deleted prototype is gone or a tombstone: nothing to check.
        if (kind == "room" && vnum > 0)
            olc_validate_room(get_room_index(vnum), findings);
        else if (kind == "mob" && vnum > 0)
            olc_validate_mob(get_mob_index(vnum), findings);
        else if (kind == "obj" && vnum > 0)
            olc_validate_obj(get_obj_index(vnum), findings);
        else if (kind == "area" && vnum >= 0 && areasDone.insert(vnum).second) {
            AreaIndexData *pArea = AreaUtils::getByVnum(vnum);
            if (ctx.validate.count(t))
                olc_validate_area_all(pArea, findings);
            else
                olc_validate_area(pArea, findings);
        }

        if (ctx.validate.count(t))
            for (size_t k = from; k < findings.size(); k++)
                findings[k].audit = true;
    }

    // The profile decides what a finding means: the player profile and a
    // strict agent batch refuse on any warning, everyone else is told.
    bool hard = ctx.policy == OLC_POLICY_PLAYER || (ctx.policy == OLC_POLICY_AGENT && strict);
    int errors = 0;

    for (auto &f: findings) {
        if (hard && f.severity == "warning")
            f.severity = "error";
        if (f.severity == "error")
            errors++;
    }

    std::stable_sort(findings.begin(), findings.end(),
                     [](const OlcFinding &a, const OlcFinding &b) {
                         return severity_rank(a.severity) < severity_rank(b.severity);
                     });

    // Terse by default: info only with verbose, and a cap on the list. The
    // summary always counts everything.
    const size_t cap = ctx.verbose ? 2000 : 200;
    Json::Value &list = result["findings"];
    Json::Value &summary = result["findingsSummary"];
    list = Json::Value(Json::arrayValue);
    summary = Json::Value(Json::objectValue);
    int hidden = 0;

    for (auto &f: findings) {
        DLString key = f.severity + " " + f.check;
        summary[key] = summary.isMember(key) ? summary[key].asInt() + 1 : 1;

        if ((f.severity == "info" && !ctx.verbose) || list.size() >= cap
            || (!f.audit && !ctx.verbose && !strict))
        {
            hidden++;
            continue;
        }

        Json::Value j;
        j["severity"] = f.severity;
        j["check"] = f.check;
        j["kind"] = f.kind;
        j["vnum"] = f.vnum;
        if (!f.field.empty())
            j["field"] = f.field;
        if (!f.lang.empty())
            j["lang"] = f.lang;
        j["message"] = f.message;
        list.append(j);
    }

    if (hidden > 0)
        result["findingsHidden"] = hidden;

    return errors;
}

/*--------------------------------------------------------------------------
 * Area helps, by help id
 *-------------------------------------------------------------------------*/

/** The article under id when it is an area help, else 0 with the reason. */
static AreaHelp * area_help(int id, DLString &error)
{
    HelpArticle::Pointer article = helpManager->getArticle(id);
    if (article.isEmpty())
        return 0;

    AreaHelp *help = dynamic_cast<AreaHelp *>(article.getPointer());
    if (!help || !help->areafile || !help->areafile->area)
        error = "Help " + DLString(id) + " is not an area help";
    return help;
}

static bool help_op(OlcOpContext &ctx, const DLString &action, int id, const Json::Value &op,
                    Json::Value &res, DLString &error)
{
    if (action != "get" && action != "put" && action != "delete") {
        error = "help action must be get, put or delete";
        return false;
    }

    if ((action == "put" || action == "delete") && !editor_refusal("help", id, error))
        return false;

    AreaHelp *help = area_help(id, error);
    if (!error.empty())
        return false;

    AreaIndexData *pArea = help ? help->areafile->area : 0;
    AreaIndexData *opArea;
    if (!op_area(ctx, op, opArea, error))
        return false;

    if (pArea && opArea && pArea != opArea) {
        error = "Help " + DLString(id) + " belongs to area " + DLString(pArea->vnum);
        return false;
    }

    if (!pArea && action == "put" && is_planned(ctx, "help", id))
        return true;

    if (!pArea && action != "put") {
        error = "Help " + DLString(id) + " not found";
        return false;
    }

    if (!pArea && !opArea) {
        error = "A new help needs 'area': the area vnum whose file keeps it";
        return false;
    }

    if (!pArea)
        pArea = opArea;

    if (!OLCState::can_edit(ctx.ch, pArea)) {
        error = "You can't build in area " + DLString(pArea->vnum);
        return false;
    }

    if (action == "get") {
        XMLAreaHelp xh;
        xh.init(help);
        Json::Value extra(Json::objectValue);
        extra["id"] = id;
        extra["area"] = pArea->vnum;
        return get_result(ctx, op, to_node(xh, "help"), extra, res, error);
    }

    if (action == "delete") {
        if (ctx.dryRun)
            return true;

        HelpArticle::Pointer article = helpManager->getArticle(id);
        helpManager->unregistrate(article);
        for (auto h = pArea->helps.begin(); h != pArea->helps.end(); h++)
            if (h->getPointer() == help) {
                pArea->helps.erase(h);
                break;
            }

        pArea->changed = true;
        help_save_ids();
        op_journal(ctx, "help", id, "delete", "");
        res["changed"] = true;
        return true;
    }

    XMLDocument::Pointer doc;
    XMLNode::Pointer node = parse_xml(op, doc, error);
    if (!node)
        return false;

    XMLAreaHelp xh;
    xh.fromXML(node);
    // The op's id wins over whatever the XML says.
    xh.id = id;

    if (ctx.dryRun) {
        if (!help)
            ctx.planned["help"].insert(id);
        return true;
    }

    if (help) {
        // In place: re-registering the id would throw "Duplicate help ID".
        OLCStateHelp::Pointer state(NEW, help);
        state->keyword = xh.keyword;
        state->title = xh.title;
        state->extra = xh.extra;
        state->text = xh.text;
        state->level = xh.level;
        state->labels.setValue(xh.labels);
        state->seeAlso.setValue(HelpArticle::seeAlsoToString(xh.seeAlso));
        state->seeAlsoEdited = true;
        state->commit();
        // Self-help detection reads the keywords.
        help->setAreaIndex(pArea);
    } else {
        AreaHelp::Pointer fresh = xh.compat();
        fresh->setAreaIndex(pArea);
        helpManager->registrate(fresh);
        pArea->helps.push_back(XMLPersistent<HelpArticle>(fresh.getPointer()));
        help_save_ids();
    }

    pArea->changed = true;
    op_journal(ctx, "help", id, "put", help ? "replace" : "new");
    res["changed"] = true;
    return true;
}

/*--------------------------------------------------------------------------
 * Dispatch
 *-------------------------------------------------------------------------*/

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

    res["kind"] = kind;

    bool prototype = kind == "room" || kind == "mob" || kind == "obj";

    if (action == "validate")
        return validate_op(ctx, kind, op, res, vnum, error);

    if (ctx.policy == OLC_POLICY_PLAYER && action != "get") {
        if (!prototype) {
            error = "The player profile only reads " + kind + " (get)";
            return false;
        }
        if (action == "put") {
            error = "The player profile can't put whole prototypes";
            return false;
        }
        if (action == "delete" && !op.isMember("exit") && !op.isMember("eexit") && !op.isMember("ed")) {
            error = "The player profile deletes parts only: exit, eexit or ed";
            return false;
        }
    }

    if (kind == "quest")
        return quest_op(ctx, action, op, res, vnum, error);

    if (kind == "help") {
        if (!op_number(ctx, op, "vnum", vnum, error))
            return false;
        res["vnum"] = vnum;
        return help_op(ctx, action, vnum, op, res, error);
    }

    if (!prototype && kind != "area" && kind != "reset") {
        error = "kind must be room, mob, obj, reset, area, quest or help";
        return false;
    }

    if (prototype && action == "create") {
        if (!op_create(ctx, kind, op, res, vnum, error))
            return false;
        return true;
    }

    if (prototype && action != "get" && action != "cmd" && action != "set"
        && action != "delete" && action != "put")
    {
        error = kind + " action must be get, create, cmd, set, delete, put or validate";
        return false;
    }

    if (!op_number(ctx, op, "vnum", vnum, error, kind == "area" ? 0 : 1))
        return false;

    res["vnum"] = vnum;

    if (kind == "area")
        return area_op(ctx, action, vnum, op, res, error);

    if (kind == "reset")
        return reset_op(ctx, action, vnum, op, res, error);

    return prototype_op(ctx, kind, action, vnum, op, res, error);
}

Json::Value OlcOps::apply(PCharacter *actor, const Json::Value &ops,
                          OlcPolicy policy, bool dryRun, bool verbose, bool save, bool strict)
{
    Json::Value result;
    result["ok"] = true;
    result["dryRun"] = dryRun;
    result["profile"] = policyName(policy);
    result["strict"] = strict;
    result["changed"] = Json::Value(Json::arrayValue);
    result["errors"] = Json::Value(Json::arrayValue);
    // Op-level notes (quest step pairing...); the validators answer in findings.
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

    // The player profile waits for phase 5 and its field whitelist. The player
    // gates below (powerFields, put/delete/area/reset/quest refusals) are a
    // blacklist and stay unreachable until P5 replaces them with the whitelist.
    if (policy == OLC_POLICY_PLAYER) {
        Json::Value err;
        err["op"] = -1;
        err["error"] = "the player profile is not available until phase 5";
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
    ctx.opIndex = 0;
    ctx.warnings = &result["warnings"];

    for (auto &op: ops)
        if (op.isObject() && op["kind"] == "room" && op["action"] == "put" && op["vnum"].isInt())
            ctx.batchRooms.insert(op["vnum"].asInt());

    std::set<std::pair<DLString, int> > changed;
    Json::Value::ArrayIndex i;

    for (i = 0; i < ops.size(); i++) {
        const Json::Value &op = ops[i];
        Json::Value res;
        DLString kind, error;
        int vnum = 0;

        res["op"] = (int)i;
        ctx.opIndex = (int)i;

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

        // A reset edit changes its room; a quest step, its quest.
        DLString changedKind = kind == "reset" ? DLString("room") : kind;

        if (res.isMember("changed") && res["changed"].asBool()
            && changed.insert(std::make_pair(changedKind, vnum)).second)
        {
            Json::Value c;
            c["kind"] = changedKind;
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

        if (!op_bool(op, "continueOnError")) {
            i++;
            break;
        }
    }

    result["skipped"] = (int)(ops.size() - i);

    // Exits to rooms the batch created after the room that leads to them.
    if (!dryRun)
        for (int vnum: ctx.batchRooms) {
            RoomIndexData *pRoom = get_room_index(vnum);
            if (pRoom)
                OLCStateRoom::resolveInstance(pRoom);
        }

    if (!dryRun)
        quest_pairing_warnings(ctx);

    // A dry run changed nothing: only what 'validate' ops named is checked.
    int validationErrors = run_validators(ctx, dryRun ? std::set<std::pair<DLString, int> >() : changed,
                                          strict, result);

    if (validationErrors > 0) {
        Json::Value err;
        err["op"] = -1;
        err["error"] = DLString(validationErrors) + " validator error(s), see findings"
                       + (save && !dryRun ? ": save skipped; no rollback, the edits stay applied and the autosave still writes them" : "");
        result["errors"].append(err);
        result["ok"] = false;
    }

    // Say so explicitly when a requested save did not run.
    if (save && !dryRun && validationErrors > 0)
        result["saved"] = Json::Value(Json::arrayValue);

    if (save && !dryRun && validationErrors == 0 && !result["changed"].empty()) {
        std::list<AreaIndexData *> saved;
        int failed = save_changed_areas(actor, saved);

        result["saved"] = Json::Value(Json::arrayValue);
        for (auto &pArea: saved)
            result["saved"].append(pArea->area_file->file_name);

        if (failed > 0) {
            Json::Value err;
            err["op"] = -1;
            err["error"] = "asave changed: " + DLString(failed) + " area(s) failed to save";
            result["errors"].append(err);
            result["ok"] = false;
        }
    }

    return result;
}
