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
 *   'cancel'      olc_land with drop: the copy is thrown away, so is the 'gen'
 * The worker writes only the fields still empty when it lands ('ifEmpty',
 * checked by the game in the same call as the write), as journaled OLC ops.
 * Nothing here waits on it.
 */
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
        t.copy = pick(olc_room_json(pRoom), keys);
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
        t.copy = pick(olc_mob_json(pMob), keys);
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
        t.copy = pick(olc_obj_json(pObj), keys);
}

/** The committed prototype; false when it does not exist. */
static bool target_index(const DLString &entity, int vnum, OlcAiTarget &t)
{
    if (entity == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        if (pRoom)
            target_room(pRoom, false, t);
        return pRoom != 0;
    }

    if (entity == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (pMob)
            target_mob(pMob, false, t);
        return pMob != 0;
    }

    if (entity == "obj") {
        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        if (pObj)
            target_obj(pObj, false, t);
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

static AiRequestResult olc_ai_queue(PCharacter *ch, const OlcAiTarget &t, bool manual,
                                    const DLString &hint, DLString &error)
{
    Json::Value payload;
    payload["entity"] = t.entity;
    payload["vnum"] = t.vnum;
    payload["area"] = t.area;
    payload["builder"] = ch->getName();
    payload["hint"] = hint;
    payload["manual"] = manual;

    Json::Value empty = t.empty;
    for (auto &e: t.extras)
        empty.append(e);
    payload["empty"] = empty;

    if (!t.copy.isNull()) {
        payload["copy"] = t.copy;
        payload["create"] = t.create;
    }

    return ai_request("olc_gen", payload, error);
}

/** The automatic request at 'done': the empty core text of the committed
 *  prototype. Silent with AI off or 'gen auto off'. */
static void olc_ai_auto(PCharacter *ch, const DLString &entity, int vnum)
{
    OlcAiTarget t;

    if (get_string_attribute(ch, AIGEN_ATTR) == "off")
        return;

    if (!ai_enabled("olc_gen"))
        return;

    // The core text decides; extra descriptions and material only ride along.
    if (!target_index(entity, vnum, t) || t.empty.empty())
        return;

    DLString error;
    if (olc_ai_queue(ch, t, false, "", error) != AI_REQUEST_OK)
        return;

    ptc(ch, "{CAI:{x asked for the empty text of %s %d ({Wgen auto off{x stops this).\r\n",
        entity.c_str(), vnum);
}

void olc_ai_done(OLCState *state, PCharacter *ch, const DLString &entity, int vnum)
{
    bool asked = state->aiAsked;
    int askedVnum = state->aiVnum;

    state->aiAsked = false;
    state->aiVnum = 0;

    if (asked) {
        // The answer 'gen' started is held while this editor is open: land it
        // now. No line: 'gen' said what comes, the worker says when it lands.
        // A lost request costs time only, the worker retries a held answer.
        Json::Value payload;
        payload["entity"] = entity;
        payload["vnum"] = askedVnum;
        payload["builder"] = ch->getName();
        DLString error;
        ai_request("olc_land", payload, error);

        // redit follows the builder: 'gen' may have been typed in another room.
        if (askedVnum == vnum)
            return;
    }

    olc_ai_auto(ch, entity, vnum);
}

void OLCState::aiCancel(PCharacter *ch)
{
    if (!aiAsked)
        return;

    aiAsked = false;
    if (getJournalKind() == "room")
        return;

    Json::Value payload;
    payload["entity"] = getJournalKind();
    payload["vnum"] = aiVnum;
    payload["builder"] = ch->getName();
    payload["drop"] = true;
    aiVnum = 0;

    DLString error;
    if (ai_request("olc_land", payload, error) == AI_REQUEST_OK)
        ch->send_to("{CAI:{x the 'gen' request goes with the editor, nothing will be written.\r\n");
    else
        ptc(ch, "{CAI:{x could not drop the 'gen' request (%s): its text may still land.\r\n", error.c_str());
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
        if (ai_request("olc_revert", payload, error) != AI_REQUEST_OK)
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
    if (olc_ai_queue(ch, t, true, args, error) != AI_REQUEST_OK) {
        ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        return false;
    }

    state->aiAsked = true;
    state->aiVnum = vnum;

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
