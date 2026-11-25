/*
 * AI phase 1 (docs/plans/AI_FEATURES.md 4.1): text for the empty fields of a
 * room, mob or object, written by the AI worker (dreamland_tools ai-worker)
 * through /api/olc once the builder leaves the editor.
 *
 * The editors only queue spool requests (airequest.h), and only while AI is
 * on (ai_enabled):
 *   'gen [hint]'  olc_gen with the editor's copy of the prototype, so the
 *                 model starts at once; the answer waits for the editor to
 *                 let go (/api/olc 'unlessOpen'; a new mob or object exists
 *                 only after 'done')
 *   'done'        olc_land for that 'gen': land the held answer now; or, with
 *                 no 'gen' typed, olc_gen for the empty core text as committed
 *   'cancel'      olc_land with drop: the copy is thrown away, so is the 'gen';
 *                 the same when the descriptor goes (link death, quit)
 * Every request carries seq, increasing in game order: the worker handles
 * the OLC requests of one spool scan in that order. A copy too big for the
 * spool falls back to the old way: the request goes out at 'done'.
 * The worker writes only the fields still empty when it lands ('ifEmpty',
 * checked by the game in the same call as the write), as journaled OLC ops.
 * Nothing here waits on it.
 */
#include <sys/time.h>

#include "olc.h"
#include "olcstate.h"
#include "olcops.h"
#include "airequest.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "room.h"
#include "merc.h"
#include "def.h"

// String attribute: "off" stops the queue at 'done'. A string, because an
// integer attribute does not survive a remort.
static const char *AIGEN_ATTR = "aigen";

// One olc_gen request: the prototype and its empty fields.
struct OlcAiTarget {
    DLString entity;
    int vnum;
    int area;
    // "field.lang" of the empty core text; "ed", "material" in extras
    Json::Value empty, extras;
    // The editor's copy as /api/olc 'get' would answer it, the fields the
    // prompt reads; null for the automatic request at 'done'.
    Json::Value copy;
    // No prototype in the index yet: a 'create' not yet committed.
    bool create;

    OlcAiTarget() : vnum(0), area(0), empty(Json::arrayValue), extras(Json::arrayValue), create(false) { }
};

static const lang_t langs[] = { LANG_EN, LANG_RU, LANG_UA };
static const char *langNames[] = { "en", "ru", "ua" };

/** Add "field.lang" to empty for every language the field has no text in. */
static void empty_langs(const char *field, const XMLMultiString &value, Json::Value &empty)
{
    for (int i = 0; i < 3; i++) {
        DLString text = value.get(langs[i]);
        text.stripWhiteSpace();
        if (text.empty())
            empty.append(DLString(field) + "." + langNames[i]);
    }
}

static bool material_unset(const DLString &material)
{
    return material.empty() || material == "none" || material == "unknown";
}

/** The top-level keys of a 'get' JSON the worker's prompt reads. */
static Json::Value pick(const Json::Value &data, const char **keys)
{
    Json::Value out(Json::objectValue);

    for (int i = 0; keys[i]; i++)
        if (data.isMember(keys[i]))
            out[keys[i]] = data[keys[i]];

    return out;
}

/** Each node of a 'get' JSON list ({"node": {...}} or {"node": [...]}). */
static void each_node(Json::Value &copy, const char *key, void (*keep)(Json::Value &))
{
    if (!copy[key].isObject() || !copy[key].isMember("node"))
        return;

    Json::Value &n = copy[key]["node"];
    if (n.isArray()) {
        for (auto &node: n)
            keep(node);
    }
    else if (n.isObject())
        keep(n);
}

static DLString clip(const Json::Value &v, size_t cap)
{
    DLString text = v.isString() ? DLString(v.asString()) : DLString::emptyString;
    return text.size() > cap ? DLString(text.substr(0, cap)) : text;
}

/** An extra description as the prompt reads it: keywords, and the RU or EN
 *  text cut where the prompt cuts it (ai_olc_gen.py describe_target). The
 *  full texts of a library would not fit the spool's 64 KB. */
static void keep_ed(Json::Value &node)
{
    Json::Value out(Json::objectValue);
    const Json::Value &desc = node["description"];

    out["keyword"] = node["keyword"];
    if (desc.isObject()) {
        out["description"]["ru"] = clip(desc["ru"], 300);
        out["description"]["en"] = clip(desc["en"], 300);
    }
    else
        out["description"] = clip(desc, 300);
    node = out;
}

/** An exit as the prompt reads it: direction, target room, door or not. */
static void keep_exit(Json::Value &node)
{
    static const char *keys[] = { "name", "target", "flags", "key", 0 };
    Json::Value out(Json::objectValue);

    for (int i = 0; keys[i]; i++)
        if (node.isMember(keys[i]))
            out[keys[i]] = node[keys[i]];
    node = out;
}

/** The copy as the worker's prompt reads it, nothing more. */
static Json::Value copy_json(const Json::Value &data, const char **keys)
{
    Json::Value copy = pick(data, keys);

    each_node(copy, "extraDescriptions", keep_ed);
    each_node(copy, "exits", keep_exit);
    return copy;
}

/**
 * Fill t with the empty fields of a room, mob or object: the index entry or
 * an editor's copy. The core fields are the text a player reads first; extra
 * descriptions and material go to extras: an old room without extra
 * descriptions is not a reason to call the model at 'done'. With withCopy
 * the facts the prompt reads ride along, taken from this same copy.
 */
static void target_room(RoomIndexData *pRoom, bool withCopy, OlcAiTarget &t)
{
    static const char *keys[] = { "name", "description", "sector", "flags", "exits", "extraDescriptions", 0 };

    t.entity = "room";
    t.vnum = pRoom->vnum;
    t.area = pRoom->areaIndex ? pRoom->areaIndex->vnum : 0;
    empty_langs("name", pRoom->name, t.empty);
    empty_langs("desc", pRoom->description, t.empty);
    if (pRoom->extraDescriptions.empty())
        t.extras.append("ed");
    if (withCopy)
        t.copy = copy_json(olc_room_json(pRoom), keys);
}

static void target_mob(const MOB_INDEX_DATA *pMob, bool withCopy, OlcAiTarget &t)
{
    static const char *keys[] = { "keyword", "short_descr", "long_descr", "description", "race", "sex",
                                  "size", "level", "act", "alignment", "extraDescriptions", 0 };

    t.entity = "mob";
    t.vnum = pMob->vnum;
    t.area = pMob->area ? pMob->area->vnum : 0;
    empty_langs("keyword", pMob->keyword, t.empty);
    empty_langs("short", pMob->short_descr, t.empty);
    empty_langs("long", pMob->long_descr, t.empty);
    empty_langs("desc", pMob->description, t.empty);
    if (withCopy)
        t.copy = copy_json(olc_mob_json(pMob), keys);
}

static void target_obj(const OBJ_INDEX_DATA *pObj, bool withCopy, OlcAiTarget &t)
{
    static const char *keys[] = { "keyword", "short_descr", "description", "type", "wear_flags", "gender",
                                  "level", "extra_flags", "material", "extraDescriptions", 0 };

    t.entity = "obj";
    t.vnum = pObj->vnum;
    t.area = pObj->area ? pObj->area->vnum : 0;
    empty_langs("keyword", pObj->keyword, t.empty);
    empty_langs("short", pObj->short_descr, t.empty);
    // The object's 'long' is the description field of the prototype.
    empty_langs("long", pObj->description, t.empty);
    if (pObj->extraDescriptions.empty())
        t.extras.append("ed");
    if (material_unset(pObj->material))
        t.extras.append("material");
    if (withCopy)
        t.copy = copy_json(olc_obj_json(pObj), keys);
}

/** The committed prototype; false when it does not exist. withCopy: its
 *  facts too (for the plan line; the request itself may leave them out). */
static bool target_index(const DLString &entity, int vnum, OlcAiTarget &t, bool withCopy = false)
{
    if (entity == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        if (pRoom)
            target_room(pRoom, withCopy, t);
        return pRoom != 0;
    }

    if (entity == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (pMob)
            target_mob(pMob, withCopy, t);
        return pMob != 0;
    }

    if (entity == "obj") {
        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        if (pObj)
            target_obj(pObj, withCopy, t);
        return pObj != 0;
    }

    return false;
}

/** A 'get' JSON value as text: an enum may come as {"text": ...}. */
static DLString json_text(const Json::Value &v)
{
    if (v.isString())
        return v.asString();
    if (v.isObject() && v["text"].isString())
        return v["text"].asString();
    return DLString::emptyString;
}

/** "name (en, ru), desc (ua), an extra description": what 'gen' asks for,
 *  one field after another, for a screen reader as much as for the eye.
 *  Mob sex and object gender are what the worker adds on its own (still a
 *  new one's value; for an object only with the Russian short). */
static DLString plan_fields(const OlcAiTarget &t)
{
    DLString list, field, langList;

    for (auto &e: t.empty) {
        DLString item = e.asString();
        DLString::size_type dot = item.find('.');
        DLString f = item.substr(0, dot), l = item.substr(dot + 1);

        if (f != field) {
            if (!field.empty())
                list += (list.empty() ? "" : ", ") + field + " (" + langList + ")";
            field = f;
            langList.clear();
        }
        langList += (langList.empty() ? "" : ", ") + l;
    }
    if (!field.empty())
        list += (list.empty() ? "" : ", ") + field + " (" + langList + ")";

    for (auto &e: t.extras) {
        DLString what = e.asString() == "ed" ? "an extra description" : e.asString();
        list += (list.empty() ? "" : ", ") + what;
    }

    if (t.empty.empty())
        return list;

    if (t.entity == "mob") {
        DLString sex = json_text(t.copy["sex"]);
        if (sex.empty() || sex == "neutral")
            list += ", sex (still neutral)";
    }
    else if (t.entity == "obj") {
        DLString gender = json_text(t.copy["gender"]);
        bool shortRu = false;
        for (auto &e: t.empty)
            shortRu = shortRu || e.asString() == "short.ru";
        if (shortRu && (gender.empty() || gender == "m"))
            list += ", gender (still m)";
    }

    return list;
}

/** ai_request with seq: microseconds, strictly increasing in game order, so
 *  the worker can order the requests one spool scan picks up together (a
 *  'gen' and the 'cancel' right after it). */
static AiRequestResult olc_ai_send(const DLString &kind, Json::Value &payload, DLString &error)
{
    static long long last = 0;
    struct timeval tv;

    gettimeofday(&tv, 0);
    long long seq = (long long)tv.tv_sec * 1000000 + tv.tv_usec;
    if (seq <= last)
        seq = last + 1;
    last = seq;

    payload["seq"] = (Json::Int64)seq;
    return ai_request(kind, payload, error);
}

static AiRequestResult olc_ai_queue(const DLString &builder, const OlcAiTarget &t, bool manual,
                                    const DLString &hint, bool withCopy, DLString &error)
{
    Json::Value payload;
    payload["entity"] = t.entity;
    payload["vnum"] = t.vnum;
    payload["area"] = t.area;
    payload["builder"] = builder;
    payload["hint"] = hint;
    payload["manual"] = manual;

    Json::Value empty = t.empty;
    for (auto &e: t.extras)
        empty.append(e);
    payload["empty"] = empty;

    if (withCopy && !t.copy.isNull()) {
        payload["copy"] = t.copy;
        payload["create"] = t.create;
    }

    return olc_ai_send("olc_gen", payload, error);
}

/** A request for the committed prototype, at 'done'. The automatic one
 *  (manual false) asks for the empty core text and is silent with AI off or
 *  'gen auto off'. manual: a 'gen' whose copy was too big to send, with its
 *  hint, extra descriptions and material included. */
static bool olc_ai_committed(PCharacter *ch, const DLString &entity, int vnum, bool manual, const DLString &hint,
                             OlcAiTarget &t)
{
    if (!manual && get_string_attribute(ch, AIGEN_ATTR) == "off")
        return false;

    if (!ai_enabled("olc_gen")) {
        if (manual)
            ch->send_to("{CAI:{x the AI is off now ({Wai status{x), nothing was asked.\r\n");
        return false;
    }

    // The core text decides; extra descriptions and material only ride along.
    if (!target_index(entity, vnum, t, manual) || (t.empty.empty() && (!manual || t.extras.empty()))) {
        if (manual)
            ptc(ch, "{CAI:{x nothing is empty on %s %d any more.\r\n", entity.c_str(), vnum);
        return false;
    }

    DLString error;
    if (olc_ai_queue(ch->getName(), t, manual, hint, false, error) != AI_REQUEST_OK) {
        if (manual)
            ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        return false;
    }

    if (manual)
        ptc(ch, "{CAI:{x asked for %s %d: %s.\r\n", entity.c_str(), vnum, plan_fields(t).c_str());
    else
        ptc(ch, "{CAI:{x asked for the empty text of %s %d ({Wgen auto off{x stops this).\r\n",
            entity.c_str(), vnum);
    return true;
}

/** The "field.lang" names of t.empty as the validators name them: the AI's
 *  "keyword" of a mob or object is the validators' "name". */
static void ai_fields(const OlcAiTarget &t, OlcAiFields &fields)
{
    for (auto &e: t.empty) {
        DLString f = e.asString();
        if (t.entity != "room" && f.find("keyword.") == 0)
            f = "name" + f.substr(7);
        fields.insert(f);
    }
}

void olc_ai_done(OLCState *state, PCharacter *ch, const DLString &entity, int vnum, OlcAiFields *pending)
{
    bool asked = state->aiAsked, deferred = state->aiDeferred;
    int askedVnum = state->aiVnum;
    DLString hint = state->aiHint;
    bool landed = false;

    state->aiAsked = false;
    state->aiDeferred = false;
    state->aiVnum = 0;
    state->aiHint.clear();

    if (asked && deferred) {
        OlcAiTarget t;
        landed = olc_ai_committed(ch, entity, askedVnum, true, hint, t);
    }
    else if (asked) {
        // The answer 'gen' started is held while this editor is open: land it
        // now. No line: 'gen' said what comes, the worker says when it lands.
        // Without this request a mob or object job reads as abandoned (the
        // editor is gone, no 'done' heard) and is dropped after 2 minutes.
        Json::Value payload;
        payload["entity"] = entity;
        payload["vnum"] = askedVnum;
        payload["builder"] = ch->getName();
        DLString error;
        if (olc_ai_send("olc_land", payload, error) != AI_REQUEST_OK)
            ptc(ch, "{CAI:{x could not tell the AI that you are done (%s): the 'gen' text may not land. "
                    "Type {Wgen{x again in the editor to retry.\r\n", error.c_str());
        else
            landed = true;
    }

    // redit follows the builder: 'gen' may have been typed in another room.
    if (asked && askedVnum == vnum) {
        // The answer lands only into fields still empty: those of the commit.
        OlcAiTarget t;
        if (landed && pending && target_index(entity, vnum, t))
            ai_fields(t, *pending);
        return;
    }

    OlcAiTarget t;
    if (olc_ai_committed(ch, entity, vnum, false, "", t) && pending)
        ai_fields(t, *pending);
}

/** The editor's copy is thrown away: so is its 'gen'. 1 when the worker was
 *  told to drop it (or a deferred one was never sent), -1 when the drop
 *  could not be queued, 0 when there was nothing to drop. Reads no
 *  character: it runs from the descriptor's close too. */
static int olc_ai_drop(OLCState *state, DLString &error)
{
    bool asked = state->aiAsked, deferred = state->aiDeferred;

    state->aiAsked = false;
    state->aiDeferred = false;
    state->aiHint.clear();

    // redit edits in place: nothing is thrown away, the answer still lands.
    if (!asked || state->getJournalKind() == "room")
        return 0;

    if (deferred)
        return 1;

    Json::Value payload;
    payload["entity"] = state->getJournalKind();
    payload["vnum"] = state->aiVnum;
    payload["builder"] = state->aiBuilder;
    payload["drop"] = true;
    state->aiVnum = 0;

    return olc_ai_send("olc_land", payload, error) == AI_REQUEST_OK ? 1 : -1;
}

void OLCState::aiCancel(PCharacter *ch)
{
    DLString error;
    int rc = olc_ai_drop(this, error);

    if (rc > 0)
        ch->send_to("{CAI:{x the 'gen' request goes with the editor, nothing will be written.\r\n");
    else if (rc < 0)
        ptc(ch, "{CAI:{x could not drop the 'gen' request (%s): its text may still land.\r\n", error.c_str());
}

void OLCState::close(Descriptor *d)
{
    DLString error;

    // Link death or quit inside the editor: nobody is left to tell.
    if (olc_ai_drop(this, error) < 0)
        LogStream::sendError() << "olc gen: could not drop on close: " << error << endl;

    InterpretHandler::close(d);
}

/** Where the 'gen' subcommand reads the prototype from: exactly one is set. */
struct OlcAiSource {
    RoomIndexData *room;
    const MOB_INDEX_DATA *mob;
    const OBJ_INDEX_DATA *obj;
    DLString entity;
    int vnum;
};

static bool olc_ai_gen_command(OLCState *state, PCharacter *ch, const OlcAiSource &src, const char *argument)
{
    DLString args = argument;
    args.stripWhiteSpace();
    DLString rest = args;
    DLString word = rest.getOneArgument();
    const DLString &entity = src.entity;
    int vnum = src.vnum;

    if (word == "auto") {
        DLString value = rest.getOneArgument();

        if (value == "off") {
            ch->getAttributes().getAttr<XMLStringAttribute>(AIGEN_ATTR)->setValue("off");
            ch->send_to("{CAI:{x no text is asked for at 'done' any more. 'gen' still asks by hand.\r\n");
        }
        else if (value == "on") {
            ch->getAttributes().eraseAttribute(AIGEN_ATTR);
            ch->send_to("{CAI:{x the empty text fields are asked for at 'done'.\r\n");
        }
        else
            ptc(ch, "{CAI:{x at 'done': %s. Usage: {Wgen auto on{x|{Woff{x\r\n",
                get_string_attribute(ch, AIGEN_ATTR) == "off" ? "off" : "on");
        return false;
    }

    if (word == "revert") {
        DLString which = rest.getOneArgument();
        int target = vnum;

        // A length bound first: toInt throws on overflow.
        if (!which.empty()) {
            if (which.size() > 7 || which.find_first_not_of("0123456789") != DLString::npos) {
                ch->send_to("Usage: {Wgen revert{x [vnum]\r\n");
                return false;
            }
            target = which.toInt();
        }

        if (!OLCState::can_edit(ch, target)) {
            ptc(ch, "{CAI:{x you cannot build in %s %d.\r\n", entity.c_str(), target);
            return false;
        }

        Json::Value payload;
        payload["entity"] = entity;
        payload["vnum"] = target;
        payload["builder"] = ch->getName();
        DLString error;
        if (olc_ai_send("olc_revert", payload, error) != AI_REQUEST_OK)
            ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        else
            ptc(ch, "{CAI:{x asked to revert the AI text on %s %d that nobody changed since.\r\n",
                entity.c_str(), target);
        return false;
    }

    if (word == "help" || word == "?") {
        ch->send_to("{Wgen{x [hint] - the AI writes the empty text fields of this entry from what the editor\r\n"
                    "    holds now; the hint (3 to 300 characters) steers it. It starts at once and\r\n"
                    "    lands after {Wdone{x, only into fields still empty then. A line tells you when.\r\n"
                    "{Wgen{x again before {Wdone{x - starts over with the editor as it is then\r\n"
                    "{Wgen auto on{x|{Woff{x - ask for the empty core text by itself at 'done' (on by default)\r\n"
                    "{Wgen revert{x [vnum] - remove the AI text nobody changed since\r\n");
        return false;
    }

    if (!OLCState::can_edit(ch, vnum)) {
        ptc(ch, "{CAI:{x you cannot build in %s %d.\r\n", entity.c_str(), vnum);
        return false;
    }

    // One or two characters are a typo more often than a hint.
    if (!args.empty() && args.size() < 3) {
        ch->send_to("{CAI:{x a hint needs at least 3 characters. A typo? {Wgen{x alone means no hint.\r\n");
        return false;
    }

    if (!ai_enabled("olc_gen")) {
        ch->send_to("{CAI:{x the AI is off now ({Wai status{x), nothing was asked.\r\n");
        return false;
    }

    OlcAiTarget t;
    if (src.room)
        target_room(src.room, true, t);
    else if (src.mob)
        target_mob(src.mob, true, t);
    else
        target_obj(src.obj, true, t);
    // 'create' gives an editor copy only: the prototype exists after 'done'.
    OlcAiTarget committed;
    t.create = !target_index(entity, vnum, committed);

    if (t.empty.empty() && t.extras.empty()) {
        ptc(ch, "{CAI:{x nothing is empty on %s %d.\r\n", entity.c_str(), vnum);
        return false;
    }

    // Untrusted text: it goes to the worker as data and into the prompt only.
    bool cut = args.size() > 300;
    if (cut)
        args = args.substr(0, 300);

    DLString error;
    AiRequestResult rc = olc_ai_queue(ch->getName(), t, true, args, true, error);
    bool deferred = rc == AI_REQUEST_INVALID;
    if (rc != AI_REQUEST_OK && !deferred) {
        ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        return false;
    }

    // A queued 'gen' replaces the worker's earlier job by itself; a deferred
    // one does not, so an earlier 'gen' from a smaller copy is dropped.
    if (deferred) {
        DLString dropError;
        olc_ai_drop(state, dropError);
    }

    state->aiAsked = true;
    state->aiDeferred = deferred;
    state->aiVnum = vnum;
    state->aiHint = deferred ? args : DLString::emptyString;
    state->aiBuilder = ch->getName();

    if (deferred)
        ptc(ch, "{CAI:{x %s %d is too large to send from the editor: the request goes out at {Wdone{x, "
                "written from what you commit.\r\n", entity.c_str(), vnum);
    else
        ptc(ch, "{CAI:{x %s %d, written from what this editor holds now.\r\n", entity.c_str(), vnum);
    ptc(ch, "{CAI:{x it writes: %s.\r\n", plan_fields(t).c_str());
    if (args.empty())
        ch->send_to("{CAI:{x no hint: written from the area and its neighbours.\r\n");
    else {
        // Echoed without colour codes and short: the full hint went out.
        DLString echo = args.colourStrip();
        if (echo.size() > 60)
            echo = echo.substr(0, 57) + "...";
        ptc(ch, "{CAI:{x hint: %s\r\n", echo.c_str());
    }
    if (cut)
        ch->send_to("{CAI:{x the hint was cut to its first 300 characters.\r\n");
    if (deferred)
        ch->send_to("{CAI:{x it lands a while after {Wdone{x, only into fields still empty then, "
                    "and a line tells you when.\r\n");
    else
        ch->send_to("{CAI:{x started. It lands after {Wdone{x, only into fields still empty then, "
                    "and a line tells you when.\r\n");
    return false;
}

bool olc_ai_gen_command(OLCState *state, PCharacter *ch, RoomIndexData *pRoom, const char *argument)
{
    OlcAiSource src = { pRoom, 0, 0, "room", pRoom->vnum };
    return olc_ai_gen_command(state, ch, src, argument);
}

bool olc_ai_gen_command(OLCState *state, PCharacter *ch, const MOB_INDEX_DATA *mob, const char *argument)
{
    OlcAiSource src = { 0, mob, 0, "mob", mob->vnum };
    return olc_ai_gen_command(state, ch, src, argument);
}

bool olc_ai_gen_command(OLCState *state, PCharacter *ch, const OBJ_INDEX_DATA *obj, const char *argument)
{
    OlcAiSource src = { 0, 0, obj, "obj", obj->vnum };
    return olc_ai_gen_command(state, ch, src, argument);
}
